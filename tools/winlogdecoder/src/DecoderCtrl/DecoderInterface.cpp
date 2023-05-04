#include "DecoderInterface.h"


DecoderInterface::DecoderInterface(QObject *parent)
    : QObject{parent}
{

}


void DecoderInterface::CancelDecoding(bool c)
{
    cancel = c;
}


void DecoderInterface::doDecoding(QString fileName, QString destFolder)
{
    Q_UNUSED(fileName)
    Q_UNUSED(destFolder)
}


void DecoderInterface::extProcessStarted()
{
    extProcessMessage += QString("%1\r\n").arg(extProcessCommand);
    if (!cancel)
    {
        emit stateReport(DECODER_STATE_STARTED, fileName);
    }
    else
    {
        extProcess->kill();
    }
}


void DecoderInterface::extProcessErrorOccurred(QProcess::ProcessError error_no)
{
    const QStringList processErrorString = {QString("FailedToStart"),
                                            QString("Crashed"),
                                            QString("Timedout"),
                                            QString("WriteError"),
                                            QString("ReadError"),
                                            QString("UnknownError")};

    if (error_no > QProcess::UnknownError)
        error_no = QProcess::UnknownError;

    QString errorString = QString("\"%1\" error: error = %2\r\n").arg(extProcessCommand, processErrorString[error_no]);
    if (error_no != QProcess::FailedToStart)
        extProcess->kill();
    extProcessMessage += errorString;
    extProcessResult = false;

    extProcess->deleteLater();
}


void DecoderInterface::extProcessReadyReadStdOut()
{
    extProcessMessage += extProcess->readAllStandardOutput();
}


void DecoderInterface::extProcessReadyReadStdErr()
{
    extProcessMessage += extProcess->readAllStandardError();;
}


void DecoderInterface::extProcessFinished(int exit_code, QProcess::ExitStatus status)
{
    if (status == QProcess::ExitStatus::CrashExit || exit_code != 0)
        extProcessResult = false;

    extProcess->deleteLater();
}
