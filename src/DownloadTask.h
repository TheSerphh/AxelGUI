#pragma once

#include <QObject>
#include <QProcess>
#include <QRegularExpression>

class DownloadTask : public QObject
{
    Q_OBJECT
public:
    explicit DownloadTask(QObject *parent = nullptr);
    void start(const QString &url, const QString &destDir, const QString &filename, int connections,
               const QString &cookie = QString(), const QString &userAgent = QString(), const QString &referer = QString());
    void cancel();
    bool isRunning() const;

signals:
    void progressUpdated(int percentage, const QString &speed, const QString &eta);
    void logReceived(const QString &line);
    void finished(bool success, const QString &message);

private slots:
    void handleReadyRead();
    void handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void handleErrorOccurred(QProcess::ProcessError error);

private:
    QProcess m_process;
    QRegularExpression m_regexProgress;
    QRegularExpression m_regexSpeed;
    QRegularExpression m_regexEta;
};
