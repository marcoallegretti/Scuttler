// SPDX-FileCopyrightText: 2026 Marco Allegretti
// SPDX-License-Identifier: EUPL-1.2

#include <QProcess>
#include <QTest>

using namespace Qt::StringLiterals;

class tst_Cli : public QObject
{
    Q_OBJECT

private:
    struct Result {
        int exitCode = -1;
        QByteArray out;
        QByteArray err;
    };

    static Result run(const QStringList &arguments)
    {
        QProcess process;
        QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
        environment.insert(u"QT_QPA_PLATFORM"_s, u"offscreen"_s);
        environment.insert(u"QT_FORCE_STDERR_LOGGING"_s, u"1"_s);
        process.setProcessEnvironment(environment);
        process.start(QStringLiteral(SCUTTLER_BINARY), arguments);
        if (!process.waitForFinished(30000)) {
            process.kill();
            process.waitForFinished();
        }
        if (process.exitStatus() != QProcess::NormalExit)
            return {-1, {}, process.readAllStandardError()};
        return {process.exitCode(), process.readAllStandardOutput(), process.readAllStandardError()};
    }

private slots:
    void version()
    {
        const Result result = run({u"--version"_s});
        QVERIFY2(result.exitCode == 0, result.err.constData());
        QCOMPARE(result.out.trimmed(), "scuttler " SCUTTLER_VERSION);
    }

    void rejects_data()
    {
        QTest::addColumn<QStringList>("arguments");
        QTest::addColumn<QByteArray>("message");
        QTest::newRow("window modes") << QStringList{u"--fullscreen"_s, u"--windowed"_s} << "either --fullscreen"_ba;
        QTest::newRow("two urls") << QStringList{u"http://a.test/"_s, u"http://b.test/"_s} << "at most one URL"_ba;
        QTest::newRow("invalid url") << QStringList{QString()} << "Invalid URL"_ba;
    }

    void rejects()
    {
        QFETCH(QStringList, arguments);
        QFETCH(QByteArray, message);
        const Result result = run(arguments);
        QVERIFY2(result.exitCode == 1 && result.err.contains(message), result.err.constData());
    }
};

QTEST_GUILESS_MAIN(tst_Cli)
#include "tst_cli.moc"
