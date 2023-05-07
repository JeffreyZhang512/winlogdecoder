#include <QFileInfo>
#include "EvtxDecoder.h"

EvtxDecoder::EvtxDecoder(QObject *parent)
{

}


EvtxDecoder::~EvtxDecoder()
{

}


void EvtxDecoder::ParseNoOfRecords()
{
    QRegularExpressionMatch match = reRecords.match(extProcessMessage);
    if (match.hasMatch())
    {
        noOfEvents = match.captured(1).toULongLong();
    }
}


bool EvtxDecoder::ParseXml(QString xmlFileName)
{
    if (cancel)
        return true;

    QFile xmlFile(xmlFileName);
    if (!xmlFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        emit log(QString("Decoder: Open  %1 failed").arg(xmlFileName).toStdString(), LOG_ERROR);
        return false;
    }

    reader.setDevice(&xmlFile);

    QFile txtFile(xmlFileName + QString(".txt"));
    if (!txtFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    QTextStream txtOut(&txtFile);

    unsigned long long index = 0;
    QString name;

    while (!cancel && reader.readNextStartElement())
    {
        name = reader.name().toString();

        if(name == "Event")
        {
            ParseEvent(txtOut);
            index ++;
            if (noOfEvents != 0)
            {
                unsigned int newPercentage = static_cast<unsigned int>((index * 100) / noOfEvents);
                if (newPercentage > 100)
                    newPercentage = 100;
                if (newPercentage != percentage)
                {
                    percentage = newPercentage;
                    emit progressReport(fileName, percentage);
                }
            }
        }
    }

    // The last timeStop value is the stop time stamp
    QRegularExpressionMatch matchStop = reDataTime.match(timeStamp);
    if (matchStop.hasMatch())
    {
        stop = QDateTime::fromString(matchStop.captured(1), Qt::ISODateWithMs);
    }

    xmlFile.close();
    txtFile.close();
    if (reader.hasError())
    {
        emit log(QString("Decoder: failed to parse  %1, error = %2 ").arg(xmlFileName, reader.errorString()).toStdString(), LOG_ERROR);
        return false;
    }
    if (xmlFile.error() != QFile::NoError)
    {
        emit log(QString("Decoder: error happens in reading %1").arg(xmlFileName).toStdString(), LOG_ERROR);
        return false;
    }
    if (txtFile.error() != QFile::NoError)
    {
        emit log(QString("Decoder: error happens in writing %1").arg(xmlFileName + ".txt").toStdString(), LOG_ERROR);
        return false;
    }

    return true;
}


bool EvtxDecoder::ParseEvent(QTextStream& txtOut)
{
    QString name;
    QString level;
    QString provider;
    QString eventId;
    QString taskCategory;
    while (reader.readNextStartElement())
    {
        name = reader.name().toString();

        if(name == "System")
        {
            ParseSystemElement(level, provider, eventId, taskCategory);
        }
        else
        {
            reader.skipCurrentElement();
        }
    }

    if (!timeStamp.isEmpty() && !provider.isEmpty())
    {
        // write to txt file
        txtOut << level << "    " << timeStamp << "    " << provider << "    " << eventId << "    " << taskCategory << "\n";
    }
    return true;
}


void EvtxDecoder::doDecoding(QString evtxFileName, QString destFolder)
{
    // long long threadId = reinterpret_cast<long long>(QThread::currentThreadId());
    this->fileName = evtxFileName;
    QString evtxdump = "evtx_dump-v0.8.1_revised.exe";

    extProcess = new QProcess(this);
    // slotes for process
    connect(extProcess, SIGNAL(started()), this, SLOT(extProcessStarted()));
    connect(extProcess, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(extProcessErrorOccurred(QProcess::ProcessError)));
    connect(extProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(extProcessReadyReadStdOut()));
    connect(extProcess, SIGNAL(readyReadStandardError()), this, SLOT(extProcessReadyReadStdErr()));
    connect(extProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(extProcessFinished(int,QProcess::ExitStatus)));
    extProcessResult = true;
    extProcessMessage.clear();

    QFileInfo fileInfo = QFileInfo(evtxFileName);
    QString xmlFileName = QString("%1/%2.xml").arg(destFolder, fileInfo.fileName());
    extProcessCommand = evtxdump + QString(" -f ") + xmlFileName.replace("/", "\\") + QString(" --no-confirm-overwrite --dont-show-record-number ") + evtxFileName.replace("/", "\\");
    QStringList arguments;
    arguments << "-f" << xmlFileName << "--no-confirm-overwrite" << "--dont-show-record-number" << evtxFileName;

    extProcess->start(evtxdump, arguments);
    extProcess->waitForFinished(-1);


    if (extProcessResult)
    {
        // evtx_dump returns success
        emit log(QString("Decoder: converted to %1 successfully").arg(xmlFileName).toStdString(), LOG_OK);
        // Get the total number of events
        ParseNoOfRecords();
        // Then parser the xml file and generate the txt file
        if (ParseXml(xmlFileName))
        {
            if (!cancel)
            {
                emit log(QString("Decoder: generated %1.txt successfully").arg(xmlFileName).toStdString(), LOG_OK);
                emit stateReport(DECODER_STATE_SUCCESS, fileName);
                emit timeStampReport(fileName, start, stop);
            }
            else
            {
                emit log(QString("Decoder: generating %1.txt canceled").arg(xmlFileName).toStdString(), LOG_WARNING);
                emit stateReport(DECODER_STATE_CANCELED, fileName);
            }
        }
        else
        {
            emit log(QString("Decoder: generating %1.txt failed").arg(xmlFileName).toStdString(), LOG_ERROR);
            emit stateReport(DECODER_STATE_ERROR, fileName);
        }
    }
    else
    {
        // tracerpt returns failure
        if (!cancel)
        {
            emit log(QString("Decoder: generating %1 failed").arg(xmlFileName).toStdString(), LOG_ERROR);
            emit stateReport(DECODER_STATE_ERROR, fileName);
        }
        else
        {
            emit log(QString("Decoder: generating %1 canceled").arg(xmlFileName).toStdString(), LOG_WARNING);
            emit stateReport(DECODER_STATE_CANCELED, fileName);
        }
    }

    // Make sure the DecoderCtrl is in the right state
    readyToBeClosed.acquire(1);
    deleteLater();
}
