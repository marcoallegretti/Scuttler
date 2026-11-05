// SPDX-FileCopyrightText: 2026 Marco Allegretti
// SPDX-License-Identifier: EUPL-1.2

#include "agama.h"

#include <QFile>

using namespace Qt::StringLiterals;

namespace Agama {

QByteArray readToken(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning("Cannot read the Agama token from %s: %s", qPrintable(path), qPrintable(file.errorString()));
        return {};
    }
    return file.readAll().trimmed();
}

QString readFirmwareLanguage(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return {};
    // efivarfs prefixes the value with 4 bytes of variable attributes.
    const QByteArray value = file.readAll().mid(4);
    QString language;
    for (const char c : value) {
        if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '-' || c == '_')
            language.append(QLatin1Char(c));
    }
    return language;
}

QUrl installerUrl(const QByteArray &token, const QString &language)
{
    QUrl url(u"http://localhost/"_s);
    QByteArrayList query;
    if (!token.isEmpty()) {
        url.setPath(u"/login"_s);
        query.append("token=" + token.toPercentEncoding());
    }
    if (!language.isEmpty())
        query.append("lang=" + language.toLatin1());
    if (!query.isEmpty())
        url.setQuery(QString::fromLatin1(query.join('&')), QUrl::StrictMode);
    return url;
}

}
