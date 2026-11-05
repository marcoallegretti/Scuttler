// SPDX-FileCopyrightText: 2026 Marco Allegretti
// SPDX-License-Identifier: EUPL-1.2

#pragma once

#include <QByteArray>
#include <QString>
#include <QUrl>

namespace Agama {

inline constexpr auto tokenCredential = "agama-token";
inline constexpr auto platformLangPath =
    "/sys/firmware/efi/efivars/PlatformLang-8be4df61-93ca-11d2-aa0d-00e098032b8c";

QByteArray readToken(const QString &path);
QString readFirmwareLanguage(const QString &path);
QUrl installerUrl(const QByteArray &token, const QString &language);

}
