#include "ThemeManager.h"
#include <QApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>
#include <QDebug>

ThemeManager& ThemeManager::instance() {
    static ThemeManager instance;
    return instance;
}

ThemeManager::ThemeManager() {
    registerBuiltinThemes();
}

void ThemeManager::registerBuiltinThemes() {
    m_themes["Default"] = {
        "Default", "#1e1e24", "#2b2b36", "#3e3e4f",
        "#f0f0f5", "#a0a0b2", "#4f8ff7", "#6aa2fc",
        "#3773db", "#4f8ff7", "#4ec9b0", "#f44747"
    };

    m_themes["Nord"] = {
        "Nord", "#2e3440", "#3b4252", "#4c566a",
        "#eceff4", "#d8dee9", "#88c0d0", "#8fbcbb",
        "#81a1c1", "#a3be8c", "#a3be8c", "#bf616a"
    };

    m_themes["Gruvbox Dark"] = {
        "Gruvbox Dark", "#282828", "#3c3836", "#504945",
        "#ebdbb2", "#d5c4a1", "#fe8019", "#fabd2f",
        "#d65d0e", "#b8bb26", "#b8bb26", "#fb4934"
    };

    m_themes["Solarized Dark"] = {
        "Solarized Dark", "#002b36", "#073642", "#586e75",
        "#839496", "#93a1a1", "#268bd2", "#2aa198",
        "#1e6fa8", "#859900", "#859900", "#dc322f"
    };

    m_themes["Catppuccin Mocha"] = {
        "Catppuccin Mocha", "#1e1e2e", "#181825", "#313244",
        "#cdd6f4", "#a6adc8", "#89b4fa", "#b4befe",
        "#74c7ec", "#a6e3a1", "#a6e3a1", "#f38ba8"
    };

    m_themes["Tokyo Night"] = {
        "Tokyo Night", "#1a1b26", "#24283b", "#414868",
        "#c0caf5", "#a9b1d6", "#7aa2f7", "#7dcfff",
        "#5a8df2", "#9ece6a", "#9ece6a", "#f7768e"
    };
}

void ThemeManager::initialize() {
    QSettings settings("AxelGui", "Appearance");
    QString saved = settings.value("currentTheme", "Default").toString();
    if (!applyTheme(saved)) {
        resetToDefault();
    }
}

QStringList ThemeManager::availableThemes() const {
    return m_themes.keys();
}

QString ThemeManager::currentThemeName() const {
    return m_currentTheme;
}

bool ThemeManager::applyTheme(const QString &themeName) {
    if (!m_themes.contains(themeName)) {
        return false;
    }
    m_currentTheme = themeName;
    ThemeColors c = m_themes[themeName];
    qApp->setStyleSheet(generateStylesheet(c));

    QSettings settings("AxelGui", "Appearance");
    settings.setValue("currentTheme", themeName);
    return true;
}

void ThemeManager::resetToDefault() {
    QSettings settings("AxelGui", "Appearance");
    settings.remove("currentTheme");
    applyTheme("Default");
}

bool ThemeManager::parseColors(const QJsonObject &obj, ThemeColors &c) const {
    if (!obj.contains("name") || !obj.contains("background") || !obj.contains("accent")) {
        return false;
    }
    c.name = obj.value("name").toString();
    c.background = obj.value("background").toString("#1e1e24");
    c.surface = obj.value("surface").toString("#2b2b36");
    c.border = obj.value("border").toString("#3e3e4f");
    c.textPrimary = obj.value("textPrimary").toString("#ffffff");
    c.textSecondary = obj.value("textSecondary").toString("#a0a0a0");
    c.accent = obj.value("accent").toString("#4f8ff7");
    c.accentHover = obj.value("accentHover").toString("#6aa2fc");
    c.accentPressed = obj.value("accentPressed").toString("#3773db");
    c.progressChunk = obj.value("progressChunk").toString(c.accent);
    c.success = obj.value("success").toString("#4ec9b0");
    c.error = obj.value("error").toString("#f44747");
    return true;
}

bool ThemeManager::importThemeFromJson(const QString &filePath, QString &outThemeName) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) return false;

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject()) return false;

    ThemeColors colors;
    if (!parseColors(doc.object(), colors)) return false;

    m_themes[colors.name] = colors;
    outThemeName = colors.name;
    return applyTheme(colors.name);
}

QString ThemeManager::generateStylesheet(const ThemeColors &c) const {
    return QString(R"(
        QWidget {
            background-color: %1;
            color: %4;
            font-family: 'Segoe UI', 'Cantarell', sans-serif;
            font-size: 13px;
        }
        QMainWindow, QDialog {
            background-color: %1;
        }
        QGroupBox {
            border: 1px solid %3;
            border-radius: 6px;
            margin-top: 10px;
            padding-top: 10px;
            font-weight: bold;
            color: %7;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 0 5px;
        }
        QLineEdit, QSpinBox, QComboBox {
            background-color: %2;
            border: 1px solid %3;
            border-radius: 4px;
            padding: 6px 10px;
            color: %4;
            selection-background-color: %7;
        }
        QLineEdit:focus, QSpinBox:focus, QComboBox:focus {
            border: 1px solid %7;
        }
        QPushButton {
            background-color: %7;
            color: %1;
            font-weight: bold;
            border: none;
            border-radius: 4px;
            padding: 8px 16px;
        }
        QPushButton:hover {
            background-color: %8;
        }
        QPushButton:pressed {
            background-color: %9;
        }
        QPushButton:disabled {
            background-color: %3;
            color: %5;
        }
        QProgressBar {
            background-color: %2;
            border: 1px solid %3;
            border-radius: 4px;
            text-align: center;
            color: %4;
            height: 22px;
        }
        QProgressBar::chunk {
            background-color: %10;
            border-radius: 3px;
        }
        QTextEdit {
            background-color: %2;
            border: 1px solid %3;
            border-radius: 4px;
            color: %4;
            font-family: monospace;
            font-size: 11px;
        }
        QLabel {
            color: %4;
        }
    )")
    .arg(c.background)       // %1
    .arg(c.surface)          // %2
    .arg(c.border)           // %3
    .arg(c.textPrimary)      // %4
    .arg(c.textSecondary)    // %5
    .arg("")                 // %6
    .arg(c.accent)           // %7
    .arg(c.accentHover)      // %8
    .arg(c.accentPressed)    // %9
    .arg(c.progressChunk);   // %10
}
