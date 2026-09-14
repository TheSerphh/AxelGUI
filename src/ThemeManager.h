#pragma once

#include <QString>
#include <QColor>
#include <QMap>
#include <QJsonObject>

struct ThemeColors {
    QString name;
    QString background;
    QString surface;
    QString border;
    QString textPrimary;
    QString textSecondary;
    QString accent;
    QString accentHover;
    QString accentPressed;
    QString progressChunk;
    QString success;
    QString error;
};

class ThemeManager {
public:
    static ThemeManager& instance();

    void initialize();
    QStringList availableThemes() const;
    bool applyTheme(const QString &themeName);
    bool importThemeFromJson(const QString &filePath, QString &outThemeName);
    void resetToDefault();
    QString currentThemeName() const;

private:
    ThemeManager();
    void registerBuiltinThemes();
    QString generateStylesheet(const ThemeColors &colors) const;
    bool parseColors(const QJsonObject &json, ThemeColors &colors) const;

    QMap<QString, ThemeColors> m_themes;
    QString m_currentTheme;
};
