// SPDX-FileCopyrightText: 2026 Marco Allegretti
// SPDX-License-Identifier: EUPL-1.2

#include "agama.h"

#include <QRegularExpression>
#include <QTemporaryDir>
#include <QTest>
#include <QUrlQuery>

using namespace Qt::StringLiterals;

class tst_Agama : public QObject
{
    Q_OBJECT

private:
    QTemporaryDir m_dir;

    QString writeFile(const QString &name, const QByteArray &content)
    {
        const QString path = m_dir.filePath(name);
        QFile file(path);
        if (!file.open(QIODevice::WriteOnly) || file.write(content) != content.size())
            qFatal("cannot write %s", qPrintable(path));
        return path;
    }

private slots:
    void readToken_data()
    {
        QTest::addColumn<QByteArray>("content");
        QTest::addColumn<QByteArray>("expected");
        QTest::newRow("plain") << "abc.def.ghi"_ba << "abc.def.ghi"_ba;
        QTest::newRow("trailing newline") << "abc.def.ghi\n"_ba << "abc.def.ghi"_ba;
        QTest::newRow("empty") << ""_ba << ""_ba;
        QTest::newRow("whitespace") << " \n"_ba << ""_ba;
    }

    void readToken()
    {
        QFETCH(QByteArray, content);
        QFETCH(QByteArray, expected);
        QCOMPARE(Agama::readToken(writeFile(u"token"_s, content)), expected);
    }

    void readTokenMissing()
    {
        QTest::ignoreMessage(QtWarningMsg, QRegularExpression(u"^Cannot read the Agama token from .*missing"_s));
        QCOMPARE(Agama::readToken(m_dir.filePath(u"missing"_s)), QByteArray());
    }

    void readFirmwareLanguage_data()
    {
        QTest::addColumn<QByteArray>("content");
        QTest::addColumn<QString>("expected");
        QTest::newRow("region") << "\x07\x00\x00\x00" "en-US\0"_ba << u"en-US"_s;
        QTest::newRow("language") << "\x07\x00\x00\x00" "de\0"_ba << u"de"_s;
        QTest::newRow("underscore") << "\x07\x00\x00\x00" "pt_BR\0"_ba << u"pt_BR"_s;
        QTest::newRow("letter attributes") << "abcd" "fr-FR\0"_ba << u"fr-FR"_s;
        QTest::newRow("digits") << "\x07\x00\x00\x00" "es-419\0"_ba << u"es-"_s;
        QTest::newRow("query injection") << "\x07\x00\x00\x00" "it&token=x#\0"_ba << u"ittokenx"_s;
        QTest::newRow("attributes only") << "\x07\x00\x00\x00"_ba << QString();
        QTest::newRow("truncated") << "\x07\x00"_ba << QString();
    }

    void readFirmwareLanguage()
    {
        QFETCH(QByteArray, content);
        QFETCH(QString, expected);
        QCOMPARE(Agama::readFirmwareLanguage(writeFile(u"PlatformLang"_s, content)), expected);
    }

    void readFirmwareLanguageMissing()
    {
        QCOMPARE(Agama::readFirmwareLanguage(m_dir.filePath(u"missing"_s)), QString());
    }

    void installerUrl_data()
    {
        QTest::addColumn<QByteArray>("token");
        QTest::addColumn<QString>("language");
        QTest::addColumn<QString>("expected");
        QTest::newRow("token and language")
            << "a.b.c"_ba << u"en-US"_s << u"http://localhost/login?token=a.b.c&lang=en-US"_s;
        QTest::newRow("token") << "a.b.c"_ba << QString() << u"http://localhost/login?token=a.b.c"_s;
        QTest::newRow("language") << QByteArray() << u"de"_s << u"http://localhost/?lang=de"_s;
        QTest::newRow("neither") << QByteArray() << QString() << u"http://localhost/"_s;
    }

    void installerUrl()
    {
        QFETCH(QByteArray, token);
        QFETCH(QString, language);
        QFETCH(QString, expected);
        QCOMPARE(Agama::installerUrl(token, language).toString(QUrl::FullyEncoded), expected);
    }

    void installerUrlKeepsTokenIntact()
    {
        const QByteArray token = "a&lang=x+y#z%41\xff"_ba;
        const QUrl url = Agama::installerUrl(token, u"de"_s);
        const QUrlQuery query(url);
        QCOMPARE(QByteArray::fromPercentEncoding(query.queryItemValue(u"token"_s, QUrl::FullyEncoded).toLatin1()), token);
        QCOMPARE(query.allQueryItemValues(u"lang"_s), QStringList{u"de"_s});
        QVERIFY(url.fragment().isEmpty());
    }
};

QTEST_GUILESS_MAIN(tst_Agama)
#include "tst_agama.moc"
