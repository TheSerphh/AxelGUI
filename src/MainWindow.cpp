#include "MainWindow.h"
#include "ThemeManager.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>
#include <QHeaderView>
#include <QDateTime>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QUuid>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
    loadPausedDownloads();

    connect(&m_task, &DownloadTask::progressUpdated, this, &MainWindow::onProgressUpdated);
    connect(&m_task, &DownloadTask::logReceived, this, &MainWindow::onLogReceived);
    connect(&m_task, &DownloadTask::finished, this, &MainWindow::onDownloadFinished);
    connect(&m_task, &DownloadTask::paused, this, &MainWindow::onDownloadPaused);
}

void MainWindow::setupUi()
{
    setWindowTitle("Axel Download Manager");
    resize(820, 620);

    QWidget *central = new QWidget(this);
    central->setObjectName("centralWidget");
    setCentralWidget(central);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(14, 14, 14, 14);
    mainLayout->setSpacing(10);

    QHBoxLayout *topBar = new QHBoxLayout();
    QLabel *themeLabel = new QLabel("Theme:", this);
    themeLabel->setStyleSheet("font-weight: bold;");

    m_themeCombo = new QComboBox(this);
    m_themeCombo->setCursor(Qt::PointingHandCursor);
    m_themeCombo->addItems(ThemeManager::instance().availableThemes());
    m_themeCombo->setCurrentText(ThemeManager::instance().currentThemeName());

    QPushButton *importThemeBtn = new QPushButton("Import JSON...", this);
    QPushButton *resetThemeBtn = new QPushButton("Reset Default", this);
    importThemeBtn->setCursor(Qt::PointingHandCursor);
    resetThemeBtn->setCursor(Qt::PointingHandCursor);

    topBar->addWidget(themeLabel);
    topBar->addWidget(m_themeCombo, 1);
    topBar->addWidget(importThemeBtn);
    topBar->addWidget(resetThemeBtn);
    topBar->addStretch(2);
    mainLayout->addLayout(topBar);

    connect(m_themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onThemeSelected);
    connect(importThemeBtn, &QPushButton::clicked, this, &MainWindow::onImportTheme);
    connect(resetThemeBtn, &QPushButton::clicked, this, &MainWindow::onResetTheme);

    m_tabWidget = new QTabWidget(this);
    m_tabWidget->setCursor(Qt::PointingHandCursor);

    QWidget *downloaderTab = new QWidget();
    QVBoxLayout *downloaderLayout = new QVBoxLayout(downloaderTab);
    downloaderLayout->setContentsMargins(10, 10, 10, 10);
    downloaderLayout->setSpacing(10);

    QGroupBox *configGroup = new QGroupBox("Download Details", this);
    QGridLayout *gridLayout = new QGridLayout(configGroup);
    gridLayout->setVerticalSpacing(10);
    gridLayout->setHorizontalSpacing(10);

    m_urlEdit = new QLineEdit(this);
    m_urlEdit->setPlaceholderText("https://example.com/file.iso");

    m_destEdit = new QLineEdit(this);
    m_destEdit->setText(QStandardPaths::writableLocation(QStandardPaths::DownloadLocation));
    QPushButton *browseBtn = new QPushButton("Browse", this);
    browseBtn->setCursor(Qt::PointingHandCursor);
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
    downloaderLayout->addWidget(configGroup);

    QHBoxLayout *actionLayout = new QHBoxLayout();
    m_startBtn = new QPushButton("Start Accelerator", this);
    m_startBtn->setObjectName("primaryBtn");
    m_startBtn->setCursor(Qt::PointingHandCursor);

    m_pauseBtn = new QPushButton("Pause", this);
    m_pauseBtn->setCursor(Qt::PointingHandCursor);
    m_pauseBtn->setEnabled(false);

    m_cancelBtn = new QPushButton("Cancel", this);
    m_cancelBtn->setObjectName("dangerBtn");
    m_cancelBtn->setCursor(Qt::PointingHandCursor);
    m_cancelBtn->setEnabled(false);

    connect(m_startBtn, &QPushButton::clicked, this, &MainWindow::onStartDownload);
    connect(m_pauseBtn, &QPushButton::clicked, this, &MainWindow::onPauseDownload);
    connect(m_cancelBtn, &QPushButton::clicked, this, &MainWindow::onCancelDownload);

    actionLayout->addWidget(m_startBtn, 2);
    actionLayout->addWidget(m_pauseBtn, 1);
    actionLayout->addWidget(m_cancelBtn, 1);
    downloaderLayout->addLayout(actionLayout);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setValue(0);
    downloaderLayout->addWidget(m_progressBar);

    QHBoxLayout *statLayout = new QHBoxLayout();
    m_statusLabel = new QLabel("Ready", this);
    m_statusLabel->setStyleSheet("font-weight: bold;");
    m_speedLabel = new QLabel("Speed: --", this);
    m_etaLabel = new QLabel("ETA: --", this);

    statLayout->addWidget(m_statusLabel, 2);
    statLayout->addWidget(m_speedLabel, 1);
    statLayout->addWidget(m_etaLabel, 1);
    downloaderLayout->addLayout(statLayout);

    m_logViewer = new QTextEdit(this);
    m_logViewer->setReadOnly(true);
    downloaderLayout->addWidget(m_logViewer);

    m_tabWidget->addTab(downloaderTab, "Active Download");

    QWidget *pausedTab = new QWidget();
    QVBoxLayout *pausedLayout = new QVBoxLayout(pausedTab);
    pausedLayout->setContentsMargins(10, 10, 10, 10);
    pausedLayout->setSpacing(10);

    m_pausedTable = new QTableWidget(0, 4, this);
    m_pausedTable->setHorizontalHeaderLabels(QStringList() << "File Name" << "Progress" << "Paused At" << "Destination");
    m_pausedTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_pausedTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_pausedTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_pausedTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    m_pausedTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_pausedTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_pausedTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    pausedLayout->addWidget(m_pausedTable);

    QHBoxLayout *pausedActions = new QHBoxLayout();
    m_resumeBtn = new QPushButton("Resume Selected", this);
    m_resumeBtn->setObjectName("primaryBtn");
    m_resumeBtn->setCursor(Qt::PointingHandCursor);

    m_deleteBtn = new QPushButton("Delete Selected", this);
    m_deleteBtn->setObjectName("dangerBtn");
    m_deleteBtn->setCursor(Qt::PointingHandCursor);

    m_clearAllBtn = new QPushButton("Clear All", this);
    m_clearAllBtn->setCursor(Qt::PointingHandCursor);

    connect(m_resumeBtn, &QPushButton::clicked, this, &MainWindow::onResumeSelectedPaused);
    connect(m_deleteBtn, &QPushButton::clicked, this, &MainWindow::onDeleteSelectedPaused);
    connect(m_clearAllBtn, &QPushButton::clicked, this, &MainWindow::onClearAllPaused);

    pausedActions->addWidget(m_resumeBtn);
    pausedActions->addWidget(m_deleteBtn);
    pausedActions->addWidget(m_clearAllBtn);
    pausedActions->addStretch();
    pausedLayout->addLayout(pausedActions);

    m_tabWidget->addTab(pausedTab, "Paused Downloads (0)");

    mainLayout->addWidget(m_tabWidget);
}

void MainWindow::setDownloadParameters(const QString &url, const QString &filename,
                                       const QString &cookie, const QString &userAgent,
                                       const QString &referer, bool autoStart)
{
    if (!url.trimmed().isEmpty())
        m_urlEdit->setText(url.trimmed());
    if (!filename.trimmed().isEmpty())
        m_fileEdit->setText(filename.trimmed());
    m_cookie = cookie;
    m_userAgent = userAgent;
    m_referer = referer;

    if (autoStart && !m_urlEdit->text().trimmed().isEmpty())
    {
        m_tabWidget->setCurrentIndex(0);
        onStartDownload();
    }
}

void MainWindow::onBrowseFolder()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Download Directory", m_destEdit->text());
    if (!dir.isEmpty())
        m_destEdit->setText(dir);
}

void MainWindow::onStartDownload()
{
    if (m_urlEdit->text().trimmed().isEmpty())
    {
        QMessageBox::warning(this, "Missing URL", "Please enter a valid download URL.");
        return;
    }

    m_logViewer->clear();
    m_statusLabel->setText("Downloading...");
    m_startBtn->setEnabled(false);
    m_pauseBtn->setEnabled(true);
    m_cancelBtn->setEnabled(true);

    m_task.start(m_urlEdit->text(), m_destEdit->text(), m_fileEdit->text(), m_connSpin->value(),
                 m_cookie, m_userAgent, m_referer);
}

void MainWindow::onPauseDownload()
{
    m_task.pause();
}

void MainWindow::onCancelDownload()
{
    m_task.cancel();
    m_startBtn->setEnabled(true);
    m_pauseBtn->setEnabled(false);
    m_cancelBtn->setEnabled(false);
    m_statusLabel->setText("Cancelled");
}

void MainWindow::onDownloadPaused()
{
    m_startBtn->setEnabled(true);
    m_pauseBtn->setEnabled(false);
    m_cancelBtn->setEnabled(false);
    m_statusLabel->setText("Paused");
    m_speedLabel->setText("Speed: --");

    // Track in paused list
    QString filename = m_fileEdit->text().trimmed();
    if (filename.isEmpty())
    {
        filename = QUrl(m_urlEdit->text().trimmed()).fileName();
        if (filename.isEmpty())
            filename = "download.bin";
    }

    PausedItem item;
    item.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    item.url = m_urlEdit->text().trimmed();
    item.destDir = m_destEdit->text().trimmed();
    item.filename = filename;
    item.connections = m_connSpin->value();
    item.cookie = m_cookie;
    item.userAgent = m_userAgent;
    item.referer = m_referer;
    item.percent = m_lastKnownProgress;
    item.pausedAt = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm");

    m_pausedList.prepend(item);
    savePausedDownloads();
    refreshPausedTable();
}

void MainWindow::onResumeSelectedPaused()
{
    int row = m_pausedTable->currentRow();
    if (row < 0 || row >= m_pausedList.size())
    {
        QMessageBox::information(this, "Select Item", "Please select a paused download to resume.");
        return;
    }

    if (m_task.isRunning())
    {
        QMessageBox::warning(this, "Download Active", "A download is currently running. Please pause or cancel it first.");
        return;
    }

    PausedItem item = m_pausedList.takeAt(row);
    savePausedDownloads();
    refreshPausedTable();

    m_urlEdit->setText(item.url);
    m_destEdit->setText(item.destDir);
    m_fileEdit->setText(item.filename);
    m_connSpin->setValue(item.connections);
    m_cookie = item.cookie;
    m_userAgent = item.userAgent;
    m_referer = item.referer;
    m_progressBar->setValue(item.percent);

    m_tabWidget->setCurrentIndex(0);
    onStartDownload();
}

void MainWindow::onDeleteSelectedPaused()
{
    int row = m_pausedTable->currentRow();
    if (row < 0 || row >= m_pausedList.size())
    {
        QMessageBox::information(this, "Select Item", "Please select an item to delete.");
        return;
    }

    PausedItem item = m_pausedList.at(row);

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "Confirm Deletion",
        QString("Delete paused download '%1'?\n\nAlso delete the partial file and .st resume state on disk?")
            .arg(item.filename),
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

    if (reply == QMessageBox::Cancel)
        return;

    if (reply == QMessageBox::Yes)
    {
        QString targetPath = QDir(item.destDir).filePath(item.filename);
        QFile::remove(targetPath);
        QFile::remove(targetPath + ".st");
    }

    m_pausedList.removeAt(row);
    savePausedDownloads();
    refreshPausedTable();
}

void MainWindow::onClearAllPaused()
{
    if (m_pausedList.isEmpty())
        return;

    if (QMessageBox::question(this, "Clear All", "Remove all paused downloads from tracking?") == QMessageBox::Yes)
    {
        m_pausedList.clear();
        savePausedDownloads();
        refreshPausedTable();
    }
}

void MainWindow::refreshPausedTable()
{
    m_pausedTable->setRowCount(m_pausedList.size());
    for (int i = 0; i < m_pausedList.size(); ++i)
    {
        const PausedItem &item = m_pausedList.at(i);
        m_pausedTable->setItem(i, 0, new QTableWidgetItem(item.filename));
        m_pausedTable->setItem(i, 1, new QTableWidgetItem(QString("%1%").arg(item.percent)));
        m_pausedTable->setItem(i, 2, new QTableWidgetItem(item.pausedAt));
        m_pausedTable->setItem(i, 3, new QTableWidgetItem(item.destDir));
    }
    m_tabWidget->setTabText(1, QString("Paused Downloads (%1)").arg(m_pausedList.size()));
}

void MainWindow::savePausedDownloads()
{
    QSettings settings("AxelGui", "Tasks");
    QJsonArray array;
    for (const auto &item : m_pausedList)
    {
        QJsonObject obj;
        obj["id"] = item.id;
        obj["url"] = item.url;
        obj["destDir"] = item.destDir;
        obj["filename"] = item.filename;
        obj["connections"] = item.connections;
        obj["cookie"] = item.cookie;
        obj["userAgent"] = item.userAgent;
        obj["referer"] = item.referer;
        obj["percent"] = item.percent;
        obj["pausedAt"] = item.pausedAt;
        array.append(obj);
    }
    settings.setValue("pausedList", QJsonDocument(array).toJson(QJsonDocument::Compact));
}

void MainWindow::loadPausedDownloads()
{
    QSettings settings("AxelGui", "Tasks");
    QString json = settings.value("pausedList").toString();
    if (json.isEmpty())
        return;

    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8());
    if (!doc.isArray())
        return;

    m_pausedList.clear();
    for (const auto &val : doc.array())
    {
        QJsonObject obj = val.toObject();
        PausedItem item;
        item.id = obj["id"].toString();
        item.url = obj["url"].toString();
        item.destDir = obj["destDir"].toString();
        item.filename = obj["filename"].toString();
        item.connections = obj["connections"].toInt(8);
        item.cookie = obj["cookie"].toString();
        item.userAgent = obj["userAgent"].toString();
        item.referer = obj["referer"].toString();
        item.percent = obj["percent"].toInt(0);
        item.pausedAt = obj["pausedAt"].toString();
        m_pausedList.append(item);
    }
    refreshPausedTable();
}

void MainWindow::onThemeSelected(int index)
{
    ThemeManager::instance().applyTheme(m_themeCombo->itemText(index));
}

void MainWindow::onImportTheme()
{
    QString file = QFileDialog::getOpenFileName(this, "Import Theme", QString(), "JSON Files (*.json)");
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
    }
}

void MainWindow::onResetTheme()
{
    ThemeManager::instance().resetToDefault();
    m_themeCombo->setCurrentText("Default");
}

void MainWindow::onProgressUpdated(int percent, const QString &speed, const QString &eta)
{
    m_lastKnownProgress = percent;
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
    m_pauseBtn->setEnabled(false);
    m_cancelBtn->setEnabled(false);
    m_statusLabel->setText(success ? "Completed" : "Error");
    if (!success)
    {
        QMessageBox::critical(this, "Download Failed", message);
    }
}
