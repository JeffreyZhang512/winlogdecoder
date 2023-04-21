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


typedef enum
{
    DECODER_STATE_STARTED,
    DECODER_STATE_SUCCESS,
    DECODER_STATE_ERROR,
    DECODER_STATE_CANCELED,
    DECODER_STATE_MAX
}DecoderState;


typedef struct
{
    QString timeStamp;
    QString provider;
    QString message;
}EventEntry;


class EtlDecoder : public QObject
{
    Q_OBJECT
public:
    explicit EtlDecoder(QObject *parent = nullptr);
    ~EtlDecoder();
    static void CancelDecoding(bool c);
    QProcess * tracerptProcess = nullptr;

private:
    bool tracerptResult = true;
    QString tracerptCommand;
    QString tracerptMessage;
    QString etlFileName;
    QXmlStreamReader reader;
    QString timeStamp;
    QDateTime start;
    QDateTime stop;
    inline static volatile bool cancel = false;
    // It's for extract the date time.
    QRegularExpression reDataTime = QRegularExpression("(\\d\\d\\d\\d-\\d\\d-\\d\\dT\\d\\d:\\d\\d:\\d\\d\\.\\d\\d\\d)");
    // It's for removing the invisible characters in the end of the string.
    // Invisible character: outside the scope from space(0x20) to ~(0x7e)
    QRegularExpression reBinaryEventData = QRegularExpression("([^\\040-\\176]*)$");

    bool ParseXml(QString xmlFileName);

    bool ParseEvent(QTextStream& txtOut);
    bool ParseSystemElement(QString& provider);
    bool ParseBinaryEventDataElement(QString& message);
    bool ParseDebugDataElement(QString& message);
    bool ParseRenderingInfoElement(QString& message);

signals:
    void stateReport(DecoderState state, QString etlFileName);
    void timeStampReport(QString etlFileName, QDateTime start, QDateTime stop);
    void log(std::string message,  LogType type);

private slots:
    void doDecoding(QString etlFileName, QString destFolder);
    void tracerptStarted();
    void tracerptErrorOccurred(QProcess::ProcessError error_no);
    void tracerptReadyReadStdOut();
    void tracerptReadyReadStdErr();
    void tracerptFinished(int exit_code, QProcess::ExitStatus status);
};

#endif // ETLDECODER_H
