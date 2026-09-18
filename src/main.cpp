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
    QCommandLineOption cookieOption("cookie", "Session Cookie header", "cookie");
    QCommandLineOption userAgentOption("user-agent", "Browser User-Agent header", "user-agent");
    QCommandLineOption refererOption("referer", "Referer header", "referer");
    
    parser.addOption(resetThemeOption);
    parser.addOption(urlOption);
    parser.addOption(fileOption);
    parser.addOption(autoStartOption);
    parser.addOption(cookieOption);
    parser.addOption(userAgentOption);
    parser.addOption(refererOption);
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
            parser.value(cookieOption),
            parser.value(userAgentOption),
            parser.value(refererOption),
            parser.isSet(autoStartOption));
    }

    window.show();
    return app.exec();
}
