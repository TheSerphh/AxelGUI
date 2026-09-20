#include "DownloadTask.h"
#include <QDir>
#include <QUrl>

DownloadTask::DownloadTask(QObject *parent)
    : QObject(parent), m_regexProgress(R"(\[\s*(\d+)%\])")
      // Matches rates like 2921.9KB/s, 2.9 MB/s, 800 B/s with or without brackets
      ,
      m_regexSpeed(R"(\b([\d\.]+\s*(?:[KMGT]?B|Bytes)/s)\b)", QRegularExpression::CaseInsensitiveOption), m_regexEta(R"(\[\s*(\d{1,2}:\d{2}(?::\d{2})?)\s*\])")
{
    connect(&m_process, &QProcess::readyReadStandardOutput, this, &DownloadTask::handleReadyRead);
    connect(&m_process, &QProcess::readyReadStandardError, this, &DownloadTask::handleReadyRead);
    connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &DownloadTask::handleProcessFinished);
    connect(&m_process, &QProcess::errorOccurred, this, &DownloadTask::handleErrorOccurred);
}

void DownloadTask::start(const QString &url, const QString &destDir, const QString &filename, int connections,
                         const QString &cookie, const QString &userAgent, const QString &referer)
{
    m_isPaused = false;

    // Reset runtime stats and start timer
    m_downloadTimer.restart();
    m_initialPercent = -1;
    m_currentSpeed = "--";
    m_currentEta = "--";
    m_currentPercent = 0;

    QStringList args;
    args << "-n" << QString::number(connections);

    if (!filename.trimmed().isEmpty())
    {
        args << "-o" << QDir(destDir).filePath(filename.trimmed());
    }
    else
    {
        m_process.setWorkingDirectory(destDir);
    }

    if (!userAgent.trimmed().isEmpty())
    {
        args << "-U" << userAgent.trimmed();
    }
    if (!cookie.trimmed().isEmpty())
    {
        args << "-H" << QString("Cookie: %1").arg(cookie.trimmed());
    }
    if (!referer.trimmed().isEmpty())
    {
        args << "-H" << QString("Referer: %1").arg(referer.trimmed());
    }

    args << url.trimmed();

    emit logReceived(QString("[INFO] Executing: axel %1").arg(args.join(" ")));
    m_process.start("axel", args);
}

void DownloadTask::pause()
{
    if (isRunning())
    {
        m_isPaused = true;
        m_process.terminate();
        if (!m_process.waitForFinished(3000))
        {
            m_process.kill();
        }
        emit logReceived("[INFO] Download paused. Resume state (.st) preserved.");
        emit paused();
    }
}

void DownloadTask::cancel()
{
    m_isPaused = false;
    if (isRunning())
    {
        m_process.terminate();
        if (!m_process.waitForFinished(1500))
        {
            m_process.kill();
        }
        emit logReceived("[INFO] Download cancelled.");
    }
}

bool DownloadTask::isRunning() const
{
    return m_process.state() != QProcess::NotRunning;
}

bool DownloadTask::isPaused() const
{
    return m_isPaused;
}

void DownloadTask::handleReadyRead()
{
    QByteArray raw = m_process.readAllStandardOutput() + m_process.readAllStandardError();
    QString output = QString::fromLocal8Bit(raw);

    QStringList lines = output.split(QRegularExpression("[\r\n]+"), Qt::SkipEmptyParts);
    bool statsUpdated = false;

    for (const QString &line : lines)
    {
        emit logReceived(line);

        auto matchSpeed = m_regexSpeed.match(line);
        if (matchSpeed.hasMatch())
        {
            m_currentSpeed = matchSpeed.captured(1).trimmed();
            statsUpdated = true;
        }

        auto matchProg = m_regexProgress.match(line);
        if (matchProg.hasMatch())
        {
            int percent = matchProg.captured(1).toInt();
            m_currentPercent = percent;
            if (m_initialPercent < 0)
            {
                m_initialPercent = percent;
            }
            statsUpdated = true;
        }
        auto matchEta = m_regexEta.match(line);
        if (matchEta.hasMatch())
        {
            m_currentEta = matchEta.captured(1).trimmed();
            statsUpdated = true;
        }
    }
    if (m_currentPercent > 0 && m_currentPercent < 100 && m_downloadTimer.isValid())
    {
        int delta = m_currentPercent - (m_initialPercent >= 0 ? m_initialPercent : 0);
        qint64 elapsedMs = m_downloadTimer.elapsed();

        if (delta > 0 && elapsedMs >= 1000)
        {
            double msPerPercent = static_cast<double>(elapsedMs) / delta;
            double remainingMs = msPerPercent * (100 - m_currentPercent);
            int remainingSec = qMax(0, static_cast<int>(remainingMs / 1000.0));

            int hours = remainingSec / 3600;
            int minutes = (remainingSec % 3600) / 60;
            int seconds = remainingSec % 60;

            if (hours > 0)
            {
                m_currentEta = QString("%1:%2:%3")
                                   .arg(hours, 2, 10, QChar('0'))
                                   .arg(minutes, 2, 10, QChar('0'))
                                   .arg(seconds, 2, 10, QChar('0'));
            }
            else
            {
                m_currentEta = QString("%1:%2")
                                   .arg(minutes, 2, 10, QChar('0'))
                                   .arg(seconds, 2, 10, QChar('0'));
            }
            statsUpdated = true;
        }
    }

    if (statsUpdated)
    {
        emit progressUpdated(m_currentPercent, m_currentSpeed, m_currentEta);
    }
}

void DownloadTask::handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (m_isPaused)
        return;

    if (exitStatus == QProcess::NormalExit && exitCode == 0)
    {
        emit progressUpdated(100, "0 B/s", "Done");
        emit finished(true, "Download finished successfully.");
    }
    else
    {
        emit finished(false, QString("Axel exited with code %1").arg(exitCode));
    }
}

void DownloadTask::handleErrorOccurred(QProcess::ProcessError error)
{
    if (!m_isPaused && error == QProcess::FailedToStart)
    {
        emit finished(false, "Failed to start axel. Make sure 'axel' is installed.");
    }
}
