#include "MainWindow.h"
#include "ThemeManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();

    connect(&m_task, &DownloadTask::progressUpdated, this, &MainWindow::onProgressUpdated);
    connect(&m_task, &DownloadTask::logReceived, this, &MainWindow::onLogReceived);
    connect(&m_task, &DownloadTask::finished, this, &MainWindow::onDownloadFinished);
}

void MainWindow::setupUi()
{
    setWindowTitle("Axel Download Manager");
    resize(760, 560);

    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    // --- Header / Theme Selector Bar ---
    QHBoxLayout *topBar = new QHBoxLayout();
    QLabel *themeLabel = new QLabel("Color Scheme:", this);
    m_themeCombo = new QComboBox(this);
    m_themeCombo->addItems(ThemeManager::instance().availableThemes());
    m_themeCombo->setCurrentText(ThemeManager::instance().currentThemeName());

    QPushButton *importThemeBtn = new QPushButton("Import JSON...", this);
    QPushButton *resetThemeBtn = new QPushButton("Reset to Default", this);

    topBar->addWidget(themeLabel);
    topBar->addWidget(m_themeCombo);
    topBar->addWidget(importThemeBtn);
    topBar->addWidget(resetThemeBtn);
    topBar->addStretch();
    mainLayout->addLayout(topBar);

    connect(m_themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onThemeSelected);
    connect(importThemeBtn, &QPushButton::clicked, this, &MainWindow::onImportTheme);
    connect(resetThemeBtn, &QPushButton::clicked, this, &MainWindow::onResetTheme);

    // --- Configuration Group ---
    QGroupBox *configGroup = new QGroupBox("Download Details", this);
    QGridLayout *gridLayout = new QGridLayout(configGroup);

    m_urlEdit = new QLineEdit(this);
    m_urlEdit->setPlaceholderText("https://example.com/file.tar.gz");

    m_destEdit = new QLineEdit(this);
    QString defDownloadDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    m_destEdit->setText(defDownloadDir);
    QPushButton *browseBtn = new QPushButton("Browse", this);
    connect(browseBtn, &QPushButton::clicked, this, &MainWindow::onBrowseFolder);

    m_fileEdit = new QLineEdit(this);
    m_fileEdit->setPlaceholderText("Optional override filename");

    m_connSpin = new QSpinBox(this);
    m_connSpin->setRange(1, 64);
    m_connSpin->setValue(8);

    gridLayout->addWidget(new QLabel("URL:"), 0, 0);
    gridLayout->addWidget(m_urlEdit, 0, 1, 1, 2);
    gridLayout->addWidget(new QLabel("Save Folder:"), 1, 0);
    gridLayout->addWidget(m_destEdit, 1, 1);
    gridLayout->addWidget(browseBtn, 1, 2);
    gridLayout->addWidget(new QLabel("Save Name:"), 2, 0);
    gridLayout->addWidget(m_fileEdit, 2, 1, 1, 2);
    gridLayout->addWidget(new QLabel("Connections (-n):"), 3, 0);
    gridLayout->addWidget(m_connSpin, 3, 1);

    mainLayout->addWidget(configGroup);

    // --- Action Controls ---
    QHBoxLayout *actionLayout = new QHBoxLayout();
    m_startBtn = new QPushButton("Start Accelerator", this);
    m_cancelBtn = new QPushButton("Cancel", this);
    m_cancelBtn->setEnabled(false);

    connect(m_startBtn, &QPushButton::clicked, this, &MainWindow::onStartDownload);
    connect(m_cancelBtn, &QPushButton::clicked, this, &MainWindow::onCancelDownload);

    actionLayout->addWidget(m_startBtn);
    actionLayout->addWidget(m_cancelBtn);
    mainLayout->addLayout(actionLayout);

    // --- Progress Information ---
    m_progressBar = new QProgressBar(this);
    m_progressBar->setValue(0);
    mainLayout->addWidget(m_progressBar);

    QHBoxLayout *statLayout = new QHBoxLayout();
    m_statusLabel = new QLabel("Ready", this);
    m_speedLabel = new QLabel("Speed: --", this);
    m_etaLabel = new QLabel("ETA: --", this);
    statLayout->addWidget(m_statusLabel, 2);
    statLayout->addWidget(m_speedLabel, 1);
    statLayout->addWidget(m_etaLabel, 1);
    mainLayout->addLayout(statLayout);

    // --- Terminal Log Viewer ---
    m_logViewer = new QTextEdit(this);
    m_logViewer->setReadOnly(true);
    mainLayout->addWidget(m_logViewer);
}

void MainWindow::onBrowseFolder()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Download Directory", m_destEdit->text());
    if (!dir.isEmpty())
    {
        m_destEdit->setText(dir);
    }
}

void MainWindow::onStartDownload()
{
    if (m_urlEdit->text().trimmed().isEmpty())
    {
        QMessageBox::warning(this, "Missing URL", "Please enter a valid download URL.");
        return;
    }

    m_logViewer->clear();
    m_progressBar->setValue(0);
    m_statusLabel->setText("Downloading...");
    m_startBtn->setEnabled(false);
    m_cancelBtn->setEnabled(true);

    m_task.start(m_urlEdit->text(), m_destEdit->text(), m_fileEdit->text(), m_connSpin->value());
}

void MainWindow::onCancelDownload()
{
    m_task.cancel();
    m_startBtn->setEnabled(true);
    m_cancelBtn->setEnabled(false);
    m_statusLabel->setText("Cancelled");
}

void MainWindow::onThemeSelected(int index)
{
    QString theme = m_themeCombo->itemText(index);
    ThemeManager::instance().applyTheme(theme);
}

void MainWindow::onImportTheme()
{
    QString file = QFileDialog::getOpenFileName(this, "Import Theme File", QString(), "JSON Files (*.json)");
    if (file.isEmpty())
        return;

    QString themeName;
    if (ThemeManager::instance().importThemeFromJson(file, themeName))
    {
        if (m_themeCombo->findText(themeName) == -1)
        {
            m_themeCombo->addItem(themeName);
        }
        m_themeCombo->setCurrentText(themeName);
        QMessageBox::information(this, "Theme Applied", QString("Theme '%1' successfully loaded.").arg(themeName));
    }
    else
    {
        QMessageBox::critical(this, "Import Error", "Failed to parse the theme JSON file. Verify required color attributes.");
    }
}

void MainWindow::onResetTheme()
{
    ThemeManager::instance().resetToDefault();
    m_themeCombo->setCurrentText("Default");
}

void MainWindow::onProgressUpdated(int percent, const QString &speed, const QString &eta)
{
    m_progressBar->setValue(percent);
    m_speedLabel->setText(QString("Speed: %1").arg(speed));
    m_etaLabel->setText(QString("ETA: %1").arg(eta));
}

void MainWindow::onLogReceived(const QString &line)
{
    m_logViewer->append(line);
}

void MainWindow::onDownloadFinished(bool success, const QString &message)
{
    m_startBtn->setEnabled(true);
    m_cancelBtn->setEnabled(false);
    m_statusLabel->setText(success ? "Completed" : "Error");
    if (!success)
    {
        QMessageBox::critical(this, "Download Failed", message);
    }
}
