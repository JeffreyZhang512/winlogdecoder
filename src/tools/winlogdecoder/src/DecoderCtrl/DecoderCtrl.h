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
 
 
#ifndef DecoderCtrl_H
#define DecoderCtrl_H

#include <QObject>
#include <QThread>
#include <QFileInfo>
#include <QDateTime>
#include "EtlDecoder.h"
#include "LogInterface.h"


#define MAX_NO_OF_THREADS 32

typedef enum
{
    THREAD_NOT_USED,
    THREAD_BUSY,
    THREAD_FREE
}ThreadState;

typedef struct
{
    QString etlFileName[MAX_NO_OF_THREADS];
    QThread pool[MAX_NO_OF_THREADS];
    DecoderInterface *decoder[MAX_NO_OF_THREADS];
    ThreadState state[MAX_NO_OF_THREADS];
}ThreadPool;


class DecoderCtrl : public QObject
{
    Q_OBJECT
public:
    explicit DecoderCtrl(LogInterface *logger, QObject *parent = nullptr);
    ~DecoderCtrl();
    void SetMaxNoOfThreads(int n);
    void Start(QFileInfoList *fileList, QString destFolder);
    void Stop();

private:
    int noOfThreadUsed = 8;
    ThreadPool threadPool;
    QFileInfoList *fileList = nullptr;
    int nextFileIndex = 0;
    QString destFolder;
    LogInterface *logger = nullptr;
    QDateTime startTimeStamp;

signals:
    void startDecoder(QString etlFileName, QString destFolder);
    void stateReport(DecoderState state, QString etlFileName);
    void progressReport(QString etlFileName, unsigned int percentage);
    void timeStampReport(QString etlFileName, QDateTime start, QDateTime stop);
    void completed();

private slots:
    void handleStateReport(DecoderState state, QString etlFileName);
    void handleProgressReport(QString etlFileName, unsigned int percentage);
    void handleTimeStampReport(QString etlFileName, QDateTime start, QDateTime stop);
    void handleLog(std::string message, LogType type);
};

#endif // DecoderCtrl_H
