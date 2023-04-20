/*
 * Copyright (C) 2023 Zhang, Jeffrey <jeffreyzh512@outlook.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
 
#include "EtlDecoder.h"
#include <QFileInfo>
#include <QThread>
#include <QRegularExpression>


EtlDecoder::EtlDecoder(QObject *parent)
    : QObject{parent}
{
}


EtlDecoder::~EtlDecoder()
{
}


void EtlDecoder::CancelDecoding()
{
    cancel = true;
}


bool EtlDecoder::ParseXml(QString xmlFileName)
{
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


    QString name;
    if (reader.readNextStartElement())
    {
        name = reader.name().toString();

        if(name == "Events")
        {
            while (!cancel && reader.readNextStartElement())
            {
                name = reader.name().toString();

                if(name == "Event")
                {
                    ParseEvent(txtOut);
                }
            }
        }
        else
        {
            reader.raiseError("wrong format");
        }
    }
    else
    {
        reader.raiseError("wrong format");
    }

    // The last timeStop value is the stop time stamp
    QRegularExpression re("(\\d\\d\\d\\d-\\d\\d-\\d\\dT\\d\\d:\\d\\d:\\d\\d\\.\\d\\d\\d)");
    QRegularExpressionMatch match = re.match(timeStamp);
    if (match.hasMatch())
    {
        stop = QDateTime::fromString(match.captured(1), Qt::ISODateWithMs);
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


bool EtlDecoder::ParseEvent(QTextStream& txtOut)
{
    QString name;
    QString provider;
    QString message;
    bool foundMessage = false;
    while (reader.readNextStartElement())
    {
        name = reader.name().toString();

        if(name == "System")
        {
            ParseSystemElement(provider);
        }
        else if (name == "BinaryEventData")
        {
            if (!foundMessage)
            {
                ParseBinaryEventDataElement(message);
                foundMessage = true;
            }
            else
            {
                reader.skipCurrentElement();
            }
        }
        else if (name == "DebugData")
        {
            if (!foundMessage)
            {
                ParseDebugDataElement(message);
                foundMessage = true;
            }
            else
            {
                reader.skipCurrentElement();
            }
        }
        else if (name == "RenderingInfo")
        {
            if (!foundMessage)
            {
                ParseRenderingInfoElement(message);
                foundMessage = true;
            }
            else
            {
                reader.skipCurrentElement();
            }
        }
        else
        {
            reader.skipCurrentElement();
        }
    }

    if (!timeStamp.isEmpty() && !provider.isEmpty() && !message.isEmpty())
    {
        // write to txt file
        txtOut << timeStamp << "    " << provider << "    " << message << "\n";
    }
    return true;
}


bool EtlDecoder::ParseSystemElement(QString& provider)
{
    // The data is like:
    // <System>
    //     <Provider Guid="{f10521c0-ffa9-4698-a49e-f00dadc2046e}" />
    //     <EventID>19</EventID>
    //     <Version>0</Version>
    //     <Level>5</Level>
    //     <Task>0</Task>
    //     <Opcode>0</Opcode>
    //     <Keywords>0x0</Keywords>
    //     <TimeCreated SystemTime="2022-08-22T17:26:59.586989500+07:59" />
    //     <Correlation ActivityID="{00000000-0000-0000-0000-000000000000}" />
    //     <Execution ProcessID="3032" ThreadID="2732" ProcessorID="11" KernelTime="0" UserTime="0" />
    //     <Channel />
    //     <Computer />
    // </System>

    QString name;
    QString processId;
    QString threadId;
    while (reader.readNextStartElement())
    {
        name = reader.name().toString();

        if (name == "Provider")
        {
            if (reader.attributes().hasAttribute("Name"))
            {
                provider = reader.attributes().value("Name").toString();
            }
            else if (reader.attributes().hasAttribute("Guid"))
            {
                provider = reader.attributes().value("Guid").toString();
            }
            reader.skipCurrentElement();
        }
        else if (name == "TimeCreated")
        {
            if (reader.attributes().hasAttribute("SystemTime"))
            {
                timeStamp = reader.attributes().value("SystemTime").toString();
            }
            if (!start.isValid())
            {
                QRegularExpression re("(\\d\\d\\d\\d-\\d\\d-\\d\\dT\\d\\d:\\d\\d:\\d\\d\\.\\d\\d\\d)");
                QRegularExpressionMatch match = re.match(timeStamp);
                if (match.hasMatch())
                {
                    start = QDateTime::fromString(match.captured(1), Qt::ISODateWithMs);
                }
            }

            reader.skipCurrentElement();
        }
        else if (name == "Execution")
        {
            if (reader.attributes().hasAttribute("ProcessID"))
            {
                processId = reader.attributes().value("ProcessID").toString();
            }

            if (reader.attributes().hasAttribute("ThreadID"))
            {
                threadId = reader.attributes().value("ThreadID").toString();
            }
            reader.skipCurrentElement();
        }
        else
        {
            reader.skipCurrentElement();
        }
    }

    provider += QString("[%1][%2]").arg(processId, threadId);

    return true;
}


bool EtlDecoder::ParseBinaryEventDataElement(QString& message)
{
    // The data is like:
    // <BinaryEventData>202020202020202020202020202044656C65746549485652494C536572766963653A204F70656E53434D616E61676572206661696C6564202835290A00</BinaryEventData>

    // In case of no error, the current token after having called readElementText is EndElement.
    // So, don't call skipCurrentElement afterwards.
    QString hex = reader.readElementText();
    QByteArray array(QByteArray::fromHex(hex.toUtf8()));
    message = array;
    // Remove the invisible characters in the end of the string
    // Invisible character: outside the scope from space(0x20) to ~(0x7e)
    QRegularExpression re("([^\\040-\\176]*)$");
    QRegularExpressionMatch match = re.match(message);
    if (match.hasMatch())
    {
        message = message.left(message.size() - match.captured(1).size());
    }
    return true;
}


bool EtlDecoder::ParseDebugDataElement(QString& message)
{
    // The data is like:
    // <DebugData>
    //     <SequenceNumber>0</SequenceNumber>
    //     <FlagsName></FlagsName>
    //     <LevelName></LevelName>
    //     <Component></Component>
    //     <SubComponent></SubComponent>
    //     <FileLine> 10</FileLine>
    //     <Function></Function>
    //     <Message>Unknown( 10): GUID=400c1ae9-0bd1-369b-1fa7-27d9adbc8749 (No Format Information found).</Message>
    // </DebugData>

    QString name;
    bool found = false;
    while (reader.readNextStartElement())
    {
        name = reader.name().toString();

        if (name == "Message")
        {
            message = reader.readElementText();
            found = true;
        }
        else
        {
            reader.skipCurrentElement();
        }
    }
    return found;
}


bool EtlDecoder::ParseRenderingInfoElement(QString& message)
{
    // The data i slike:
    // <RenderingInfo Culture="zh-CN">
    //     <Level>&#20449;&#24687; </Level>
    //     <Keywords>
    //         <Keyword>acpi:Diagnostic</Keyword>
    //         <Keyword>ms:Telemetry</Keyword>
    //     </Keywords>
    //     <Task>AmlMethodTrace</Task>
    //     <Message>ACPI method \_SB.PC00.TDM1._STA evaluation has started . </Message>
    //     <Channel>Microsoft-Windows-Kernel-Acpi/Diagnostic</Channel>
    //     <Provider>Microsoft-Windows-Kernel-Acpi </Provider>
    // </RenderingInfo>

    QString name;
    bool found = false;
    while (reader.readNextStartElement())
    {
        name = reader.name().toString();

        if (name == "Message")
        {
            message = reader.readElementText();
            found = true;
        }
        else
        {
            reader.skipCurrentElement();
        }
    }
    return found;
}


void EtlDecoder::doDecoding(QString etlFileName, QString destFolder)
{
    // long long threadId = reinterpret_cast<long long>(QThread::currentThreadId());
    this->etlFileName = etlFileName;
    QString tracerpt = "tracerpt";

    tracerptProcess = new QProcess(this);
    // slotes for process
    connect(tracerptProcess, SIGNAL(started()), this, SLOT(tracerptStarted()));
    connect(tracerptProcess, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(tracerptErrorOccurred(QProcess::ProcessError)));
    connect(tracerptProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(tracerptReadyReadStdOut()));
    connect(tracerptProcess, SIGNAL(readyReadStandardError()), this, SLOT(tracerptReadyReadStdErr()));
    connect(tracerptProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(tracerptFinished(int,QProcess::ExitStatus)));
    tracerptResult = true;
    tracerptMessage.clear();

    QFileInfo fileInfo = QFileInfo(etlFileName);
    QString xmlFileName = QString("%1/%2.xml").arg(destFolder, fileInfo.fileName());
    tracerptCommand = tracerpt + QString(" ") + etlFileName.replace("/", "\\") + QString(" -lr -y -o ") + xmlFileName.replace("/", "\\");
    QStringList arguments;
    arguments << etlFileName << "-lr" << "-y" << "-o" << xmlFileName;

    tracerptProcess->start(tracerpt, arguments);
    tracerptProcess->waitForFinished(-1);

    if (tracerptResult)
    {
        // tracerpt returns success
        emit log(QString("Decoder: converted to %1 successfully").arg(xmlFileName).toStdString(), LOG_OK);
        // Then parser the xml file and generate the txt file
        if (ParseXml(xmlFileName))
        {
            emit log(QString("Decoder: generated %1.txt successfully").arg(xmlFileName).toStdString(), LOG_OK);
            emit stateReport(DECODER_STATE_SUCCESS, etlFileName.replace("\\", "/"));
            emit timeStampReport(etlFileName, start, stop);
        }
        else
        {
            emit log(QString("Decoder: converted to %1.txt failed").arg(xmlFileName).toStdString(), LOG_OK);
            emit stateReport(DECODER_STATE_ERROR, etlFileName.replace("\\", "/"));
        }
    }
    else
    {
        // tracerpt returns failure
        emit log(QString("Decoder: generated %1 failed").arg(xmlFileName).toStdString(), LOG_ERROR);
        emit stateReport(DECODER_STATE_ERROR, etlFileName.replace("\\", "/"));
    }

    deleteLater();
}


void EtlDecoder::tracerptStarted()
{
    tracerptMessage += QString("%1\r\n").arg(tracerptCommand);
    emit stateReport(DECODER_STATE_STARTED, etlFileName);
}


void EtlDecoder::tracerptErrorOccurred(QProcess::ProcessError error_no)
{
    const QStringList processErrorString = {QString("FailedToStart"),
                                            QString("Crashed"),
                                            QString("Timedout"),
                                            QString("WriteError"),
                                            QString("ReadError"),
                                            QString("UnknownError")};

    if (error_no > QProcess::UnknownError)
        error_no = QProcess::UnknownError;

    QString errorString = QString("\"%1\" error: error = %2\r\n").arg(tracerptCommand, processErrorString[error_no]);
    if (error_no != QProcess::FailedToStart)
        tracerptProcess->kill();
    tracerptMessage += errorString;
    tracerptResult = false;

    tracerptProcess->deleteLater();
}


void EtlDecoder::tracerptReadyReadStdOut()
{
    tracerptMessage += tracerptProcess->readAllStandardOutput();
}


void EtlDecoder::tracerptReadyReadStdErr()
{
    tracerptMessage += tracerptProcess->readAllStandardError();;
}


void EtlDecoder::tracerptFinished(int exit_code, QProcess::ExitStatus status)
{
    if (status == QProcess::ExitStatus::CrashExit || exit_code != 0)
        tracerptResult = false;

    tracerptProcess->deleteLater();
}
