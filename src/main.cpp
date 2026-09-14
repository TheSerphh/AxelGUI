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
    parser.setApplicationDescription("Axel Download Manager GUI");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption resetThemeOption(
        QStringList() << "r" << "reset-theme",
        "Resets theme settings back to Default.");
    QCommandLineOption urlOption(
        QStringList() << "u" << "url",
        "Download URL",
        "url");
    QCommandLineOption fileOption(
        QStringList() << "f" << "filename",
        "Target file name",
        "filename");
    QCommandLineOption autoStartOption(
        QStringList() << "a" << "autostart",
        "Automatically start the download immediately");

    parser.addOption(resetThemeOption);
    parser.addOption(urlOption);
    parser.addOption(fileOption);
    parser.addOption(autoStartOption);

    parser.process(app);

    if (parser.isSet(resetThemeOption))
    {
        ThemeManager::instance().resetToDefault();
        std::cout << "[Axel-GUI] Theme reset to Default." << std::endl;
        return 0;
    }

    ThemeManager::instance().initialize();

    MainWindow window;

    if (parser.isSet(urlOption))
    {
        window.setDownloadParameters(
            parser.value(urlOption),
            parser.value(fileOption),
            parser.isSet(autoStartOption));
    }

    window.show();
    return app.exec();
}
