#include "ThemeManager.h"
#include <QApplication>
#include <QFile>
#include <QDir>
#include <QImage>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>

// Dynamically creates crisp, anti-aliased chevron icons matching the theme palette
static QString generateChevronPng(const QString &direction, const QString &hexColor, int size, qreal strokeWidth)
{
    QString safeColor = QString(hexColor).remove('#');
    QString filePath = QString("%1/axel_chevron_%2_%3_%4.png")
                           .arg(QDir::tempPath(), direction, safeColor, QString::number(size));

    if (QFile::exists(filePath))
    {
        return filePath;
    }

    QImage img(size, size, QImage::Format_ARGB32_Premultiplied);
    img.fill(Qt::transparent);

    QPainter painter(&img);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPen pen(QColor(hexColor), strokeWidth, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
    painter.setPen(pen);

    QPainterPath path;
    if (direction == "down")
    {
        path.moveTo(size * 0.22, size * 0.36);
        path.lineTo(size * 0.50, size * 0.64);
        path.lineTo(size * 0.78, size * 0.36);
    }
    else if (direction == "up")
    {
        path.moveTo(size * 0.22, size * 0.64);
        path.lineTo(size * 0.50, size * 0.36);
        path.lineTo(size * 0.78, size * 0.64);
    }

    painter.drawPath(path);
    painter.end();

    img.save(filePath, "PNG");
    return filePath;
}

ThemeManager &ThemeManager::instance()
{
    static ThemeManager instance;
    return instance;
}

ThemeManager::ThemeManager()
{
    registerBuiltinThemes();
}

void ThemeManager::registerBuiltinThemes()
{
    m_themes["Default"] = {
        "Default", "#18181b", "#27272a", "#202024", "#3f3f46",
        "#3f3f46", "#f4f4f5", "#a1a1aa", "#3b82f6", "#60a5fa",
        "#2563eb", "#ffffff", "#ef4444", "#381a1a", "#10b981"};

    m_themes["Gruvbox Dark"] = {
        "Gruvbox Dark", "#282828", "#32302f", "#2c2a29", "#3c3836",
        "#504945", "#ebdbb2", "#a89984", "#fe8019", "#fabd2f",
        "#d65d0e", "#282828", "#fb4934", "#3c2020", "#b8bb26"};

    m_themes["Nord"] = {
        "Nord", "#2e3440", "#3b4252", "#353b49", "#434c5e",
        "#4c566a", "#eceff4", "#d8dee9", "#88c0d0", "#8fbcbb",
        "#81a1c1", "#2e3440", "#bf616a", "#3d2b33", "#a3be8c"};

    m_themes["Catppuccin Mocha"] = {
        "Catppuccin Mocha", "#1e1e2e", "#252538", "#202030", "#313244",
        "#45475a", "#cdd6f4", "#a6adc8", "#89b4fa", "#b4befe",
        "#74c7ec", "#11111b", "#f38ba8", "#3d2331", "#a6e3a1"};

    m_themes["Tokyo Night"] = {
        "Tokyo Night", "#1a1b26", "#24283b", "#1f2335", "#2f354f",
        "#414868", "#c0caf5", "#9aa5ce", "#7aa2f7", "#7dcfff",
        "#5a8df2", "#15161e", "#f7768e", "#38202d", "#9ece6a"};

    m_themes["Solarized Dark"] = {
        "Solarized Dark", "#002b36", "#073642", "#052e39", "#0e4352",
        "#586e75", "#93a1a1", "#657b83", "#268bd2", "#2aa198",
        "#1e6fa8", "#ffffff", "#dc322f", "#321b20", "#859900"};
}

void ThemeManager::initialize()
{
    QSettings settings("AxelGui", "Appearance");
    QString saved = settings.value("currentTheme", "Default").toString();
    if (!applyTheme(saved))
    {
        resetToDefault();
    }
}

QStringList ThemeManager::availableThemes() const
{
    return m_themes.keys();
}

QString ThemeManager::currentThemeName() const
{
    return m_currentTheme;
}

bool ThemeManager::applyTheme(const QString &themeName)
{
    if (!m_themes.contains(themeName))
        return false;

    m_currentTheme = themeName;
    qApp->setStyleSheet(generateStylesheet(m_themes[themeName]));

    QSettings settings("AxelGui", "Appearance");
    settings.setValue("currentTheme", themeName);
    return true;
}

void ThemeManager::resetToDefault()
{
    QSettings settings("AxelGui", "Appearance");
    settings.remove("currentTheme");
    applyTheme("Default");
}

bool ThemeManager::parseColors(const QJsonObject &obj, ThemeColors &c) const
{
    if (!obj.contains("name") || !obj.contains("background") || !obj.contains("accent"))
    {
        return false;
    }
    c.name = obj.value("name").toString();
    c.background = obj.value("background").toString("#18181b");
    c.surface = obj.value("surface").toString("#27272a");
    c.surfaceCard = obj.value("surfaceCard").toString(c.surface);
    c.surfaceLight = obj.value("surfaceLight").toString("#3f3f46");
    c.border = obj.value("border").toString("#3f3f46");
    c.textPrimary = obj.value("textPrimary").toString("#ffffff");
    c.textMuted = obj.value("textMuted").toString("#a1a1aa");
    c.accent = obj.value("accent").toString("#3b82f6");
    c.accentHover = obj.value("accentHover").toString("#60a5fa");
    c.accentPressed = obj.value("accentPressed").toString("#2563eb");
    c.accentText = obj.value("accentText").toString("#ffffff");
    c.danger = obj.value("danger").toString("#ef4444");
    c.dangerBg = obj.value("dangerBg").toString("#381a1a");
    c.success = obj.value("success").toString("#10b981");
    return true;
}

bool ThemeManager::importThemeFromJson(const QString &filePath, QString &outThemeName)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &err);
    if (err.error != QJsonParseError::NoError || !doc.isObject())
        return false;

    ThemeColors colors;
    if (!parseColors(doc.object(), colors))
        return false;

    m_themes[colors.name] = colors;
    outThemeName = colors.name;
    return applyTheme(colors.name);
}

QString ThemeManager::generateStylesheet(const ThemeColors &c) const
{
    // Generate theme-matching chevron assets in temp directory
    QString comboArrowNormal = generateChevronPng("down", c.textPrimary, 16, 2.2);
    QString comboArrowHover = generateChevronPng("down", c.accent, 16, 2.2);
    QString spinUpNormal = generateChevronPng("up", c.textPrimary, 14, 2.0);
    QString spinUpHover = generateChevronPng("up", c.accent, 14, 2.0);
    QString spinDownNormal = generateChevronPng("down", c.textPrimary, 14, 2.0);
    QString spinDownHover = generateChevronPng("down", c.accent, 14, 2.0);

    QString qss = R"(
        QMainWindow, QDialog, QWidget#centralWidget {
            background-color: {{BG}};
            color: {{TEXT_PRIMARY}};
            font-family: 'Cantarell', 'Inter', 'Segoe UI', sans-serif;
            font-size: 13px;
        }

        QLabel {
            background: transparent;
            border: none;
            color: {{TEXT_PRIMARY}};
        }

        /* --- Cards / GroupBox --- */
        QGroupBox {
            background-color: {{SURFACE_CARD}};
            border: 1px solid {{BORDER}};
            border-radius: 8px;
            margin-top: 28px;
            padding: 18px 14px 14px 14px;
            font-weight: bold;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            left: 12px;
            top: 4px;
            background: transparent;
            color: {{ACCENT}};
            font-size: 11px;
            text-transform: uppercase;
            letter-spacing: 0.5px;
        }

        /* --- Text Inputs --- */
        QLineEdit {
            background-color: {{SURFACE}};
            border: 1px solid {{BORDER}};
            border-radius: 6px;
            padding: 7px 12px;
            color: {{TEXT_PRIMARY}};
            selection-background-color: {{ACCENT}};
            selection-color: {{ACCENT_TEXT}};
        }
        QLineEdit:focus {
            border: 1.5px solid {{ACCENT}};
        }

        /* --- Dropdown (QComboBox) --- */
        QComboBox {
            background-color: {{SURFACE}};
            border: 1px solid {{BORDER}};
            border-radius: 6px;
            padding: 6px 36px 6px 12px;
            color: {{TEXT_PRIMARY}};
            font-weight: 500;
        }
        QComboBox:hover {
            border-color: {{ACCENT}};
        }
        QComboBox::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 30px;
            border-left: 1px solid {{SURFACE}};
            border-top-right-radius: 10px;
            border-bottom-right-radius: 10px;
            background-color: {{SURFACE}};
        }
        QComboBox::drop-down:hover {
            background-color: {{SURFACE}};
        }
        QComboBox::down-arrow {
            image: url("{{COMBO_ARROW}}");
            width: 13px;
            height: 13px;
        }
        QComboBox::down-arrow:hover {
            image: url("{{COMBO_ARROW_HOVER}}");
        }
        QComboBox QAbstractItemView {
            background-color: {{SURFACE}};
            border: 1px solid {{BORDER}};
            border-radius: 6px;
            selection-background-color: {{ACCENT}};
            selection-color: {{ACCENT_TEXT}};
            color: {{TEXT_PRIMARY}};
            padding: 4px;
            outline: none;
        }
        QComboBox QAbstractItemView::item {
            min-height: 28px;
            padding: 4px 8px;
            border-radius: 4px;
        }
        /* --- Modern Tab Widget & Bar --- */
        QTabWidget::pane {
            border: 1px solid {{BORDER}};
            border-radius: 8px;
            background-color: {{SURFACE_CARD}};
            top: -1px;
        }
        QTabBar::tab {
            background-color: {{SURFACE}};
            color: {{TEXT_MUTED}};
            border: 1px solid {{BORDER}};
            border-bottom: none;
            border-top-left-radius: 6px;
            border-top-right-radius: 6px;
            padding: 8px 18px;
            margin-right: 4px;
            font-weight: 600;
        }
        QTabBar::tab:selected {
            background-color: {{SURFACE_CARD}};
            color: {{ACCENT}};
            border-bottom: 2px solid {{ACCENT}};
        }
        QTabBar::tab:hover:!selected {
            background-color: {{SURFACE_LIGHT}};
            color: {{TEXT_PRIMARY}};
        }

        QTableWidget {
            background-color: {{SURFACE}};
            border: 1px solid {{BORDER}};
            border-radius: 6px;
            gridline-color: {{BORDER}};
            color: {{TEXT_PRIMARY}};
            selection-background-color: {{SURFACE_LIGHT}};
            selection-color: {{ACCENT}};
        }
        QHeaderView::section {
            background-color: {{SURFACE_CARD}};
            color: {{TEXT_MUTED}};
            font-weight: bold;
            padding: 6px 10px;
            border: none;
            border-bottom: 1px solid {{BORDER}};
            border-right: 1px solid {{BORDER}};
        }
        QTableWidget::item {
            padding: 6px 10px;
        }

        /* --- Connections SpinBox --- */
        QSpinBox {
            background-color: {{SURFACE}};
            border: 1px solid {{BORDER}};
            border-radius: 6px;
            padding: 6px 40px 6px 12px;
            color: {{TEXT_PRIMARY}};
            font-weight: 500;
        }
        QSpinBox:focus {
            border: 1.5px solid {{ACCENT}};
        }
        QSpinBox::up-button {
            subcontrol-origin: border;
            subcontrol-position: top right;
            width: 0px;
            height: 0px;
            border-left: 1px solid {{BORDER}};
            border-bottom: 1px solid {{BORDER}};
            border-top-right-radius: 6px;
            background-color: {{SURFACE_LIGHT}};
        }
        QSpinBox::up-button:hover {
            background-color: {{BORDER}};
        }
        QSpinBox::up-button:pressed {
            background-color: {{ACCENT}};
        }
        QSpinBox::up-arrow {
            image: url("{{SPIN_UP}}");
            width: 0px;
            height: 0px;
        }
        QSpinBox::up-arrow:hover {
            image: url("{{SPIN_UP_HOVER}}");
        }
        QSpinBox::down-button {
            subcontrol-origin: border;
            subcontrol-position: bottom right;
            width: 0px;
            height: 0px;
            border-left: 1px solid {{BORDER}};
            border-bottom-right-radius: 6px;
            background-color: {{SURFACE_LIGHT}};
        }
        QSpinBox::down-button:hover {
            background-color: {{BORDER}};
        }
        QSpinBox::down-button:pressed {
            background-color: {{ACCENT}};
        }
        QSpinBox::down-arrow {
            image: url("{{SPIN_DOWN}}");
            width: 0px;
            height: 0px;
        }
        QSpinBox::down-arrow:hover {
            image: url("{{SPIN_DOWN_HOVER}}");
        }

        /* --- Secondary Buttons --- */
        QPushButton {
            background-color: {{SURFACE}};
            color: {{TEXT_PRIMARY}};
            border: 1px solid {{BORDER}};
            border-radius: 6px;
            padding: 7px 14px;
            font-weight: 600;
        }
        QPushButton:hover {
            background-color: {{SURFACE_LIGHT}};
            border-color: {{ACCENT}};
            color: {{ACCENT}};
        }
        QPushButton:pressed {
            background-color: {{BORDER}};
        }
        QPushButton:disabled {
            background-color: {{SURFACE}};
            border-color: {{SURFACE}};
            color: {{TEXT_MUTED}};
        }

        /* --- Primary Action Button --- */
        QPushButton#primaryBtn {
            background-color: {{ACCENT}};
            color: {{ACCENT_TEXT}};
            border: none;
            padding: 9px 20px;
            font-size: 13px;
        }
        QPushButton#primaryBtn:hover {
            background-color: {{ACCENT_HOVER}};
        }
        QPushButton#primaryBtn:pressed {
            background-color: {{ACCENT_PRESSED}};
        }
        QPushButton#primaryBtn:disabled {
            background-color: {{SURFACE_LIGHT}};
            color: {{TEXT_MUTED}};
        }

        /* --- Danger / Cancel Button --- */
        QPushButton#dangerBtn {
            background-color: {{DANGER_BG}};
            color: {{DANGER}};
            border: 1px solid {{DANGER}};
            padding: 9px 20px;
        }
        QPushButton#dangerBtn:hover {
            background-color: {{DANGER}};
            color: {{BG}};
        }
        QPushButton#dangerBtn:disabled {
            background-color: {{SURFACE}};
            border-color: {{BORDER}};
            color: {{TEXT_MUTED}};
        }

        /* --- Progress Bar --- */
        QProgressBar {
            background-color: {{SURFACE}};
            border: 1px solid {{BORDER}};
            border-radius: 6px;
            text-align: center;
            color: {{TEXT_PRIMARY}};
            font-weight: 600;
            height: 22px;
        }
        QProgressBar::chunk {
            background-color: {{ACCENT}};
            border-radius: 5px;
        }

        /* --- Terminal Log Viewer --- */
        QTextEdit {
            background-color: {{SURFACE_CARD}};
            border: 1px solid {{BORDER}};
            border-radius: 6px;
            color: {{TEXT_PRIMARY}};
            font-family: 'JetBrains Mono', 'Fira Code', monospace;
            font-size: 11px;
            padding: 8px;
        }

        /* --- Scrollbars --- */
        QScrollBar:vertical {
            background: transparent;
            width: 8px;
            margin: 0;
        }
        QScrollBar::handle:vertical {
            background: {{BORDER}};
            min-height: 24px;
            border-radius: 4px;
        }
        QScrollBar::handle:vertical:hover {
            background: {{TEXT_MUTED}};
        }
        QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {
            height: 0px;
        }
    )";

    // Replace color and asset tokens
    qss.replace("{{BG}}", c.background);
    qss.replace("{{SURFACE}}", c.surface);
    qss.replace("{{SURFACE_CARD}}", c.surfaceCard);
    qss.replace("{{SURFACE_LIGHT}}", c.surfaceLight);
    qss.replace("{{BORDER}}", c.border);
    qss.replace("{{TEXT_PRIMARY}}", c.textPrimary);
    qss.replace("{{TEXT_MUTED}}", c.textMuted);
    qss.replace("{{ACCENT}}", c.accent);
    qss.replace("{{ACCENT_HOVER}}", c.accentHover);
    qss.replace("{{ACCENT_PRESSED}}", c.accentPressed);
    qss.replace("{{ACCENT_TEXT}}", c.accentText);
    qss.replace("{{DANGER}}", c.danger);
    qss.replace("{{DANGER_BG}}", c.dangerBg);
    qss.replace("{{SUCCESS}}", c.success);

    // Replace vector icon paths
    qss.replace("{{COMBO_ARROW}}", comboArrowNormal);
    qss.replace("{{COMBO_ARROW_HOVER}}", comboArrowHover);
    qss.replace("{{SPIN_UP}}", spinUpNormal);
    qss.replace("{{SPIN_UP_HOVER}}", spinUpHover);
    qss.replace("{{SPIN_DOWN}}", spinDownNormal);
    qss.replace("{{SPIN_DOWN_HOVER}}", spinDownHover);

    return qss;
}
