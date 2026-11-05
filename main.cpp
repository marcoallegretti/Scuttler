// SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
// SPDX-FileCopyrightText: 2026 Marco Allegretti
// SPDX-License-Identifier: BSD-3-Clause AND EUPL-1.2

#include "agama.h"

#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDir>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QScreen>
#include <QUrl>
#include <QWindow>
#include <QtWebEngineQuick/qtwebenginequickglobal.h>

using namespace Qt::StringLiterals;

static QUrl agamaUrl()
{
    QByteArray token;
    if (const QString credentials = qEnvironmentVariable("CREDENTIALS_DIRECTORY"); !credentials.isEmpty())
        token = Agama::readToken(QDir(credentials).filePath(QLatin1StringView(Agama::tokenCredential)));
    return Agama::installerUrl(token, Agama::readFirmwareLanguage(QLatin1StringView(Agama::platformLangPath)));
}

int main(int argc, char *argv[])
{
    QtWebEngineQuick::initialize();
    QGuiApplication app(argc, argv);
    QGuiApplication::setApplicationVersion(QStringLiteral(SCUTTLER_VERSION));

    QCommandLineParser parser;
    parser.setApplicationDescription(u"Web view for the Agama installer."_s);
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(u"url"_s, u"Page to open instead of the Agama web UI."_s, u"[url]"_s);
    const QCommandLineOption fullscreen(u"fullscreen"_s, u"Cover the whole screen (default)."_s);
    const QCommandLineOption windowed(u"windowed"_s, u"Open a regular window."_s);
    parser.addOptions({fullscreen, windowed});
    parser.process(app);

    const auto fail = [&parser](const QString &message) {
        parser.showMessageAndExit(QCommandLineParser::MessageType::Error, message + u'\n', 1);
    };
    if (parser.isSet(fullscreen) && parser.isSet(windowed))
        fail(u"Use either --fullscreen or --windowed."_s);
    const QStringList arguments = parser.positionalArguments();
    if (arguments.size() > 1)
        fail(u"Give at most one URL."_s);

    const QUrl url = arguments.isEmpty() ? agamaUrl() : QUrl::fromUserInput(arguments.constFirst());
    if (!url.isValid())
        fail(u"Invalid URL: %1"_s.arg(arguments.value(0)));

    QVariantMap properties{{u"initialUrl"_s, url}};
    if (parser.isSet(windowed)) {
        const QSize size = QGuiApplication::primaryScreen()->availableSize() * 4 / 5;
        properties.insert({{u"width"_s, size.width()},
                           {u"height"_s, size.height()},
                           {u"visibility"_s, QWindow::Windowed}});
    } else {
        properties.insert(u"visibility"_s, QWindow::FullScreen);
    }

    QQmlApplicationEngine engine;
    engine.setInitialProperties(properties);
    engine.load(QUrl(u"qrc:/main.qml"_s));
    if (engine.rootObjects().isEmpty())
        return 1;
    return app.exec();
}
