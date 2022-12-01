#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QObject>
#include "tChartStyle.h"
#include "tColormapPickWidget.h"
#include "tContrastWidget.h"
#include "tDisplaySettings.h"
#include "tFileIO.h"
#include "tMiscSmallWidgets.h"
#include "tROIs.h"
#include "tROIStyle.h"
#include "tTraceChartWidget.h"
#include "tTraceViewWidget.h"
#include "tSettings.h"
#include "tStyleWidget.h"
#include "tVideoControllerWidget.h"
#include "tVideoData.h"
#include "tWorkflow.h"
#include <iostream>

typedef QObject* (*QConstructor)();

class TestBank {
public:
    template<class T> void addSuite()
    {
        constructormap.insert(T::staticMetaObject.className(), &constructor<T>);
        metamap.insert(T::staticMetaObject.className(), T::staticMetaObject);
    }
    QStringList getTestSuiteList() const
    {
        return constructormap.keys();
    }
    QStringList getTestCaseList(QString suite = "") const
    {
        QStringList  ret;
        if (!suite.isEmpty()) {
            auto it = metamap.find(suite);
            if (it == metamap.end()) {
                return QList<QString>();
            }
            auto meta{ it.value() };
            for (int i = meta.methodOffset(); i < meta.methodCount(); ++i) {
                auto thisname = QString::fromLatin1(meta.method(i).name());
                if (meta.method(i).methodType() == QMetaMethod::MethodType::Slot &&
                    thisname != "cleanup" && thisname != "init" &&
                    thisname != "cleanupTestCase" && thisname != "initTestCase"
                    && !thisname.endsWith("_data")) {
                    ret.push_back(suite + "." + thisname);
                }
            }
            return ret;
        }

        QStringList suites{ getTestSuiteList() };
        for (auto& s : suites) {
            ret.append(getTestCaseList(s));
        }
        return ret;
    }

    QObject* create(const QString& className) const
    {
        QConstructor c = constructormap.value(className);
        return (c == nullptr) ? nullptr : (*c)();
    }
private:
    QMap<QString, QConstructor> constructormap;
    QMap<QString, QMetaObject> metamap;
    template<class T> static QObject* constructor()
    {
        return new T();
    }
};

namespace {
    QMap<QString, QStringList> getTestSet(const QStringList& args, const TestBank& bank) {
        auto alltests{ bank.getTestCaseList() };
        QStringList selectedTests;
        if (args.isEmpty()) {
            selectedTests.append(alltests);
        }
        else {
            for (const auto& pat : args) {
                QRegExp reg(pat, Qt::CaseInsensitive, QRegExp::Wildcard);
                for (const auto& tc : alltests) {
                    if (reg.indexIn(tc) == 0) {
                        selectedTests.push_back(tc);
                    }
                }
            }
        }

        // Now the selectedTests go into a map for better organization later:
        QMap<QString, QStringList> testset;
        for (const auto& tc : selectedTests) {
            auto suitename = tc.left(tc.indexOf("."));
            auto casename = tc.mid(tc.indexOf(".") + 1);

            if (!testset.contains(suitename)) {
                testset.insert(suitename, {});
            }
            testset[suitename].push_back(casename);
        }

        return testset;
    }

    void printsuites(const QMap<QString, QStringList>& testset) {
        std::cout << "\n";
        for (const auto& s : testset.keys()) {
            std::cout << s.toStdString() << "\n";
        }
        std::cout << std::endl;
    }
    void printcases(const QMap<QString, QStringList>& testset) {
        std::cout << "\n";
        for (const auto& s : testset.keys()) {
            for (const auto& t : testset[s]) {
                std::cout << (s + "." + t).toStdString() << "\n";
            }
        }
        std::cout << std::endl;
    }
    size_t countsuites(const QMap<QString, QStringList>& testset) {
        return testset.count();
    }
    size_t countcases(const QMap<QString, QStringList>& testset) {
        size_t ret = 0;
        for (const auto& s : testset.keys()) {
            for (const auto& t : testset[s]) {
                ret++;
            }
        }
        return ret;
    }

    std::array<size_t, 3> printresults(QFile& source, bool disablepretty, QFile& outfile) {
        bool doFile = !outfile.fileName().isEmpty();
        bool doPretty = !doFile && !disablepretty;

        QTextStream outstream;
        if (doFile) {
            outstream.setDevice(&outfile);
        }

        std::array<size_t, 3> ret({ 0,0,0 });

        QTextStream in(&source);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            if (!line.startsWith("***")) {
                line = "  " + line;
            }

            ret[0] += line.startsWith("  PASS");
            ret[1] += line.startsWith("  FAIL");
            ret[2] += line.startsWith("  XFAIL");

            if (doPretty) {
                line = line.replace("PASS", "\033[32mPASS\033[0m");
                line = line.replace("FAIL!", "\033[31mFAIL!\033[0m");
                line = line.replace("INFO", "\033[36mINFO\033[0m");
                line = line.replace("XFAIL", "\033[33mXFAIL\033[0m");
            }
            if (doFile) {
                outstream << line << "\n";
            }
            else {
                std::cout << line.toStdString() << "\n";
            }
        }
        return ret;
    }
}

class filescopeguard {
public:
    filescopeguard(QFile* f) {
        file = f;
    }
    ~filescopeguard() {
        file->remove();
    }
private:
    QFile* file;
};

int main(int argc, char** argv)
{
    TestBank bank;
    

    bank.addSuite<tChartStyle>();
    bank.addSuite<tColormapPickWidget>();
    bank.addSuite<tContrastWidget>();
    bank.addSuite<tDisplaySettings>();
    bank.addSuite<tFileIO>();
    bank.addSuite<tMiscSmallWidgets>();
    bank.addSuite<tROIs>();
    bank.addSuite<tROIStyle>();
    bank.addSuite<tSettings>();
    bank.addSuite<tStyleWidget>();
    bank.addSuite<tTraceChartWidget>();
    bank.addSuite<tTraceViewWidget>();    
    bank.addSuite<tVideoControllerWidget>();
    bank.addSuite<tVideoData>();
    bank.addSuite<tWorkflow>();

    //QGuiApplication app(argc, argv);
    QApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription("ROIVert Test");
    //parser.addHelpOption();
    parser.addPositionalArgument("test", "Test name(s): specify in the format Suite.Name. Wildcards allowed, and a trailing wildcard is automatically added. Omit test to include all tests.");
    parser.addOption({ {"h", "?", "help"}, "Displays help on commandline options." });
    parser.addOption({ {"o", "output"}, "Write results to <file>. Note that this only applies to test results, not to list options.", "file" });
    parser.addOption({ {"L", "listsuites"}, "List test suites." });
    parser.addOption({ {"l", "listcases"}, "List test cases." });
    parser.addOption({ {"s", "silent"}, "Silent output (passthrough to Qt Test)" });
    parser.addOption({ {"v", "verbosity"}, "Verbosity (passthrough to Qt Test)", "1|2|s" });
    parser.addOption({ {"d", "disableformatting"}, "Disable pretty formatting (for terminals that don't support ANSI formatting)." });
    parser.addOption({ {"C", "countsuites"}, "Count the number of test suites." });
    parser.addOption({ {"c", "countcases"}, "Count the number of test cases." });

    // options:
    parser.process(app);
    if (parser.isSet("h")) {
        std::cout << parser.helpText().toStdString();
        return 0;
    }

    const QStringList args = parser.positionalArguments();
    QMap<QString, QStringList> testset{ getTestSet(args, bank) };

    if (parser.isSet("L")) {
        printsuites(testset);
        return 0;
    }

    if (parser.isSet("l")) {
        printcases(testset);
        return 0;
    }

    if (parser.isSet("C")) {
        std::cout << "\n" << "Number of Suites: " << countsuites(testset) << std::endl;
        return 0;
    }

    if (parser.isSet("c")) {
        std::cout << "\n" << "Number of Cases: " << countcases(testset) << std::endl;
        return 0;
    }

    QFile tempfile("ROIVertTestResultsTemporaryFile");
    filescopeguard fsg(&tempfile);

    QFile outfile;
    if (!parser.value("o").isEmpty()) {
        outfile.setFileName(parser.value("o"));
        if (!outfile.open(QIODevice::WriteOnly)) {
            std::cerr << "Cannot write to " << parser.value("o").toStdString() << ".\n";
            return 1;
        }
    }

    QElapsedTimer t;
    t.start();
    std::array<size_t, 3> totals_case({ 0,0,0 });
    size_t totalpassed = 0, totalfailed = 0;
    for (const auto& s : testset.keys()) {
        auto obj = bank.create(s);
        QStringList params = { "" };    // First parameter always blank
        params.append(testset[s]);      // All cases
        params.append({ "-o", "ROIVertTestResultsTemporaryFile" });
        if (parser.isSet("s")) {
            params.push_back("-silent");
        }
        else if (!parser.value("v").isEmpty()) {
            params.push_back("-v" + parser.value("v"));
        }

        QTest::qExec(obj, params);
        if (!tempfile.open(QIODevice::ReadOnly)) {
            std::cerr << "Error reading temporary file.\n";
            return 1;
        }
        std::array<size_t, 3> res = printresults(tempfile, parser.isSet("d"), outfile);
        for (size_t i = 0; i < 3; ++i) {
            totals_case[i] += res[i];
        }
        totalpassed += static_cast<size_t>(res[1] == 0);
        totalfailed += static_cast<size_t>(res[1] != 0);

        tempfile.close();
    }

    std::cout << "\nSummary\n-------\n"
        << "Cases Passed:\t\033[32m" << totals_case[0] << "\033[0m\n"
        << "Cases Failed:\t\033[31m" << totals_case[1] << "\033[0m\n"
        << "Cases Ignored:\t\033[33m" << totals_case[2] << "\033[0m\n"
        << "Suites Passed:\t\033[32m" << totalpassed << "\033[0m\n"
        << "Suites Failed:\t\033[31m" << totalfailed << "\033[0m\n"
        << "Duration:\t" << (float)t.elapsed() / 1000.f << "s\n"
        << std::endl;

    if (!outfile.fileName().isEmpty()) {
        outfile.close();
    }

    return 0;
};