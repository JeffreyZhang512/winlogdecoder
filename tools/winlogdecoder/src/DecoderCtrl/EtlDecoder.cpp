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


EtlDecoder::EtlDecoder(QObject *parent)
{
}


EtlDecoder::~EtlDecoder()
{
}


void EtlDecoder::ParseSummary(QString summaryFileName)
{
    QFile summaryFile(summaryFileName);
    if (!summaryFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        emit log(QString("Decoder: Open  %1 failed").arg(summaryFileName).toStdString(), LOG_ERROR);
        return;
    }

    while (!summaryFile.atEnd())
    {
        QString line = summaryFile.readLine();
        // The data is as:
        // Total Events  Processed 74099
        QRegularExpressionMatch match = reEvents.match(line);
        if (match.hasMatch())
        {
            noOfEvents = match.captured(1).toULongLong();
            break;
        }
    }

    summaryFile.close();
    if (summaryFile.error() != QFile::NoError)
    {
        emit log(QString("Decoder: error happens in reading %1").arg(summaryFileName).toStdString(), LOG_ERROR);
        return;
    }
}


bool EtlDecoder::ParseXml(QString xmlFileName)
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
    QRegularExpressionMatch match = reDataTime.match(timeStamp);
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
    QString level;
    QString provider;
    QString eventId;
    QString taskCategory;
    QString message;
    bool foundMessage = false;
    while (reader.readNextStartElement())
    {
        name = reader.name().toString();

        if(name == "System")
        {
            ParseSystemElement(level, provider, eventId, taskCategory);
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
        txtOut << level << "    " << timeStamp << "    " << provider << "    " << eventId << "    " << taskCategory << "    " << message << "\n";
    }
    return true;
}


bool EtlDecoder::ParseSystemElement(QString& level, QString& provider, QString& eventId, QString& taskCategory)
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
        else if (name == "EventID")
        {
            eventId = reader.readElementText();
        }
        else if (name == "Level")
        {
            QString levelString = reader.readElementText();
            int levelNo = levelString.toInt();
            // 0 - Undefined
            // 1 - Critical
            // 2 - Error
            // 3 - Warning
            // 4 - Information
            // 5 - Verbose
            switch (levelNo)
            {
            case 1:
                level = QString("[Critical]   ");
                break;
            case 2:
                level = QString("[Error]      ");
                break;
            case 3:
                level = QString("[Warning]    ");
                break;
            case 5:
                level = QString("[Verbose]    ");
                break;
            default:
                level = QString("[Information]");
                break;
            }
        }
        else if (name == "Task")
        {
            taskCategory = QString("(%1)").arg(reader.readElementText());
        }
        else if (name == "TimeCreated")
        {
            if (reader.attributes().hasAttribute("SystemTime"))
            {
                timeStamp = reader.attributes().value("SystemTime").toString();
            }
            if (!start.isValid())
            {
                QRegularExpressionMatch match = reDataTime.match(timeStamp);
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
    QRegularExpressionMatch match = reBinaryEventData.match(message);
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
    this->fileName = etlFileName;
    QString tracerpt = "tracerpt";

    extProcess = new QProcess(this);
    // slotes for process
    connect(extProcess, SIGNAL(started()), this, SLOT(extProcessStarted()));
    connect(extProcess, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(extProcessErrorOccurred(QProcess::ProcessError)));
    connect(extProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(extProcessReadyReadStdOut()));
    connect(extProcess, SIGNAL(readyReadStandardError()), this, SLOT(extProcessReadyReadStdErr()));
    connect(extProcess, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(extProcessFinished(int,QProcess::ExitStatus)));
    extProcessResult = true;
    extProcessMessage.clear();

    QFileInfo fileInfo = QFileInfo(etlFileName);
    QString xmlFileName = QString("%1/%2.xml").arg(destFolder, fileInfo.fileName());
    QString summaryFileName = xmlFileName + QString(".summary.txt");
    extProcessCommand = tracerpt + QString(" ") + etlFileName.replace("/", "\\") + QString(" -lr -y -o ") + xmlFileName.replace("/", "\\") + QString(" -summary ") + summaryFileName.replace("/", "\\");
    QStringList arguments;
    arguments << etlFileName << "-lr" << "-y" << "-o" << xmlFileName << "-summary" << summaryFileName;

    extProcess->start(tracerpt, arguments);
    extProcess->waitForFinished(-1);

    if (extProcessResult)
    {
        // tracerpt returns success
        emit log(QString("Decoder: converted to %1 successfully").arg(xmlFileName).toStdString(), LOG_OK);
        // Parse the summary file to get the total number of events
        ParseSummary(summaryFileName);
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


