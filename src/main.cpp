#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <iostream>
#include "MainWindow.h"
#include "ThemeManager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("AxelGui");
    app.setApplicationVersion("1.0.0");

    QCommandLineParser parser;
    parser.setApplicationDescription("C++ Clang GUI wrapper for Axel with robust theme engine");
    parser.addHelpOption();
    parser.addVersionOption();

    // Command-line safetynet option
    QCommandLineOption resetThemeOption(
        QStringList() << "r" << "reset-theme",
        "Resets any active or corrupted custom theme back to the system default.");
    parser.addOption(resetThemeOption);

    parser.process(app);

    if (parser.isSet(resetThemeOption))
    {
        ThemeManager::instance().resetToDefault();
        std::cout << "[Axel-GUI SafetyNet] Theme settings successfully reset to Default." << std::endl;
        return 0;
    }

    ThemeManager::instance().initialize();

    MainWindow window;
    window.show();

    return app.exec();
}
