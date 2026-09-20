#pragma once

#include <QMainWindow>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QProgressBar>
#include <QTextEdit>
#include <QLabel>
#include <QComboBox>
#include <QTabWidget>
#include <QTableWidget>
#include "DownloadTask.h"

struct PausedItem
{
    QString id;
    QString url;
    QString destDir;
    QString filename;
    int connections;
    QString cookie;
    QString userAgent;
    QString referer;
    int percent;
    QString pausedAt;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    void setDownloadParameters(const QString &url, const QString &filename = QString(),
                               const QString &cookie = QString(), const QString &userAgent = QString(),
                               const QString &referer = QString(), bool autoStart = false);

private slots:
    void onBrowseFolder();
    void onStartDownload();
    void onPauseDownload();
    void onCancelDownload();
    void onThemeSelected(int index);
    void onImportTheme();
    void onResetTheme();

    void onProgressUpdated(int percent, const QString &speed, const QString &eta);
    void onLogReceived(const QString &line);
    void onDownloadFinished(bool success, const QString &message);
    void onDownloadPaused();

    void onResumeSelectedPaused();
    void onDeleteSelectedPaused();
    void onClearAllPaused();

private:
    void setupUi();
    void loadPausedDownloads();
    void savePausedDownloads();
    void refreshPausedTable();

    QTabWidget *m_tabWidget;

    // --- Tab 1: Downloader ---
    QLineEdit *m_urlEdit;
    QLineEdit *m_destEdit;
    QLineEdit *m_fileEdit;
    QSpinBox *m_connSpin;
    QPushButton *m_startBtn;
    QPushButton *m_pauseBtn;
    QPushButton *m_cancelBtn;
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
    QLabel *m_speedLabel;
    QLabel *m_etaLabel;
    QTextEdit *m_logViewer;
    QComboBox *m_themeCombo;

    // --- Tab 2: Paused Downloads ---
    QTableWidget *m_pausedTable;
    QPushButton *m_resumeBtn;
    QPushButton *m_deleteBtn;
    QPushButton *m_clearAllBtn;

    DownloadTask m_task;
    int m_lastKnownProgress = 0;
    QString m_cookie;
    QString m_userAgent;
    QString m_referer;

    QList<PausedItem> m_pausedList;
};
