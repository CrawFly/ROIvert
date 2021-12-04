#include <QtTest/QtTest>
#include <QCoreApplication>
#include <QObject>
#include "tChartStyle.h"
#include "tColormapPickWidget.h"
#include "tContrastWidget.h"
#include "tDisplaySettings.h"
#include "tROIStyle.h"
#include "tVideoControllerWidget.h"
#include "tMiscSmallWidgets.h"
#include <iostream>

typedef QObject * (*QConstructor)();

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
    void printresults(QFile& source, bool disablepretty, QFile& outfile) {
        bool doFile = !outfile.fileName().isEmpty();
        bool doPretty = !doFile && !disablepretty;

        QTextStream outstream;
        if (doFile) {
            outstream.setDevice(&outfile);
        }
        
        QTextStream in(&source);
        while (!in.atEnd())
        {
            QString line = in.readLine();
            if (!line.startsWith("***")) {
                line = "  " + line;
            }
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
    bank.addSuite<tROIStyle>();
    bank.addSuite<tVideoControllerWidget>();
    bank.addSuite<tMiscSmallWidgets>();

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
    // todo: summarize results
    
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

    for (const auto& s : testset.keys()) {
        auto obj = bank.create(s);
        QStringList params = { "" };    // First parameter always blank
        params.append(testset[s]);      // All cases
        params.append({"-o", "ROIVertTestResultsTemporaryFile"});
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
        printresults(tempfile, parser.isSet("d"), outfile);
        tempfile.close();
    }
    if (!outfile.fileName().isEmpty()) {
        outfile.close();
    }    
    
    return 0;
};
