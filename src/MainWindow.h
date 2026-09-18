#pragma once

#include <QMainWindow>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QProgressBar>
#include <QTextEdit>
#include <QLabel>
#include <QComboBox>
#include "DownloadTask.h"

class MainWindow : public QMainWindow
{

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void setDownloadParameters(const QString &url, const QString &filename = QString(),
                               const QString &cookie = QString(), const QString &userAgent = QString(),
                               const QString &referer = QString(), bool autoStart = false);

    Q_OBJECT

private slots:
    void onBrowseFolder();
    void onStartDownload();
    void onCancelDownload();
    void onThemeSelected(int index);
    void onImportTheme();
    void onResetTheme();

    void onProgressUpdated(int percent, const QString &speed, const QString &eta);
    void onLogReceived(const QString &line);
    void onDownloadFinished(bool success, const QString &message);

private:
    void setupUi();

    QLineEdit *m_urlEdit;
    QLineEdit *m_destEdit;
    QLineEdit *m_fileEdit;
    QSpinBox *m_connSpin;
    QPushButton *m_startBtn;
    QPushButton *m_cancelBtn;
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
    QLabel *m_speedLabel;
    QLabel *m_etaLabel;
    QTextEdit *m_logViewer;
    QComboBox *m_themeCombo;
    QString m_cookie;
    QString m_userAgent;
    QString m_referer;
    DownloadTask m_task;
};
