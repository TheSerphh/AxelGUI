#include "DownloadTask.h"
#include <QDir>

DownloadTask::DownloadTask(QObject *parent)
    : QObject(parent)
    , m_regexProgress(R"(\[\s*(\d+)%\])")
    , m_regexSpeed(R"(\[\s*([\d\.]+\s*[KMGT]?B/s)\])")
    , m_regexEta(R"(\[\s*(\d{2}:\d{2}(?::\d{2})?)\s*\])")
{
    connect(&m_process, &QProcess::readyReadStandardOutput, this, &DownloadTask::handleReadyRead);
    connect(&m_process, &QProcess::readyReadStandardError, this, &DownloadTask::handleReadyRead);
    connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &DownloadTask::handleProcessFinished);
    connect(&m_process, &QProcess::errorOccurred, this, &DownloadTask::handleErrorOccurred);
}

void DownloadTask::start(const QString &url, const QString &destDir, const QString &filename, int connections) {
    QStringList args;
    args << "-n" << QString::number(connections);

    if (!filename.trimmed().isEmpty()) {
        args << "-o" << QDir(destDir).filePath(filename.trimmed());
    } else {
        m_process.setWorkingDirectory(destDir);
    }

    args << url.trimmed();

    emit logReceived(QString("[INFO] Executing: axel %1").arg(args.join(" ")));
    m_process.start("axel", args);
}

void DownloadTask::cancel() {
    if (isRunning()) {
        m_process.terminate();
        if (!m_process.waitForFinished(1000)) {
            m_process.kill();
        }
        emit logReceived("[INFO] Download cancelled by user.");
    }
}

bool DownloadTask::isRunning() const {
    return m_process.state() != QProcess::NotRunning;
}

void DownloadTask::handleReadyRead() {
    QByteArray raw = m_process.readAllStandardOutput() + m_process.readAllStandardError();
    QString output = QString::fromLocal8Bit(raw);

    // Axel updates in terminal with carriage returns
    QStringList lines = output.split(QRegularExpression("[\r\n]+"), Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        emit logReceived(line);

        auto matchProg = m_regexProgress.match(line);
        auto matchSpeed = m_regexSpeed.match(line);
        auto matchEta = m_regexEta.match(line);

        if (matchProg.hasMatch()) {
            int percent = matchProg.captured(1).toInt();
            QString speed = matchSpeed.hasMatch() ? matchSpeed.captured(1) : "--";
            QString eta = matchEta.hasMatch() ? matchEta.captured(1) : "--";
            emit progressUpdated(percent, speed, eta);
        }
    }
}

void DownloadTask::handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        emit progressUpdated(100, "0 B/s", "Done");
        emit finished(true, "Download finished successfully.");
    } else {
        emit finished(false, QString("Axel exited with code %1").arg(exitCode));
    }
}

void DownloadTask::handleErrorOccurred(QProcess::ProcessError error) {
    if (error == QProcess::FailedToStart) {
        emit finished(false, "Failed to start axel. Make sure 'axel' is installed and in PATH.");
    }
}
