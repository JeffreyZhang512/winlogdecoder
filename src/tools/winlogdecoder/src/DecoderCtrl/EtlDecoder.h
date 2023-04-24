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
 
 
#ifndef ETLDECODER_H
#define ETLDECODER_H


#include <QObject>
#include <QProcess>
#include <LogInterface.h>
#include <QXmlStreamReader>
#include <QDateTime>
#include <QRegularExpression>
#include <DecoderInterface.h>


typedef struct
{
    QString timeStamp;
    QString provider;
    QString message;
}EventEntry;


class EtlDecoder : public DecoderInterface
{
    Q_OBJECT
public:
    explicit EtlDecoder(QObject *parent = nullptr);
    ~EtlDecoder();
    QProcess * tracerptProcess = nullptr;

private:
    bool tracerptResult = true;
    QString tracerptCommand;
    QString tracerptMessage;
    QString etlFileName;
    unsigned long long noOfEvents = 0;
    unsigned int percentage = 0;
    QXmlStreamReader reader;
    QString timeStamp;
    QDateTime start;
    QDateTime stop;
    // It's for extract the date time.
    QRegularExpression reDataTime = QRegularExpression("(\\d\\d\\d\\d-\\d\\d-\\d\\dT\\d\\d:\\d\\d:\\d\\d\\.\\d\\d\\d)");
    // It's for removing the invisible characters in the end of the string.
    // Invisible character: outside the scope from space(0x20) to ~(0x7e)
    QRegularExpression reBinaryEventData = QRegularExpression("([^\\040-\\176]*)$");
    QRegularExpression reEvents = QRegularExpression("Total\\s+Events\\s+Processed\\s+(\\d+)");

    void ParseSummary(QString summaryFileName);
    bool ParseXml(QString xmlFileName);
    bool ParseEvent(QTextStream& txtOut);
    bool ParseSystemElement(QString& provider);
    bool ParseBinaryEventDataElement(QString& message);
    bool ParseDebugDataElement(QString& message);
    bool ParseRenderingInfoElement(QString& message);

private slots:
    void doDecoding(QString etlFileName, QString destFolder);
    void tracerptStarted();
    void tracerptErrorOccurred(QProcess::ProcessError error_no);
    void tracerptReadyReadStdOut();
    void tracerptReadyReadStdErr();
    void tracerptFinished(int exit_code, QProcess::ExitStatus status);
};

#endif // ETLDECODER_H
