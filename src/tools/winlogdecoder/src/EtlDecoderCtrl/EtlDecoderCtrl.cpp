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
 
 
#include "EtlDecoderCtrl.h"
#include "EtlDecoder.h"
#include "Windows.h"


EtlDecoderCtrl::EtlDecoderCtrl(LogInterface *logger, QObject *parent)
    : QObject{parent}
{
    this->logger = logger;
    for (int i = 0; i < MAX_NO_OF_THREADS; i ++)
    {
        threadPool.state[i] = THREAD_NOT_USED;
        threadPool.pool[i].start();
    }
}


EtlDecoderCtrl::~EtlDecoderCtrl()
{
    EtlDecoder::CancelDecoding(true);
    Sleep(1000);
    for (int i = 0; i < MAX_NO_OF_THREADS; i ++)
    {
        threadPool.pool[i].terminate();
        threadPool.pool[i].wait();
    }
}

void EtlDecoderCtrl::SetMaxNoOfThreads(int n)
{
    if (n > MAX_NO_OF_THREADS)
        n = MAX_NO_OF_THREADS;

    noOfThreadUsed = n;
    for (int i = 0; i < noOfThreadUsed; i ++)
    {
        if (threadPool.state[i] == THREAD_NOT_USED)
            threadPool.state[i] = THREAD_FREE;
    }
    for (int i = noOfThreadUsed; i < MAX_NO_OF_THREADS; i ++)
    {
        if (threadPool.state[i] == THREAD_FREE)
            threadPool.state[i] = THREAD_NOT_USED;
        // If the state is THREAD_BUSY, it will be changed back to THREAD_NOT_USED once the current task is completed.
    }
}


void EtlDecoderCtrl::Start(QFileInfoList *fileList, QString destFolder)
{
    this->fileList = fileList;
    this->destFolder = destFolder;
    this->nextFileIndex = 0;
    int noOfLogFiles = this->fileList->size();
    bool next = true;
    logger->Log("Decoding started");
    EtlDecoder::CancelDecoding(false);
    startTimeStamp = QDateTime::currentDateTime();
    for (int i = 0; i < noOfLogFiles; i ++)
    {
        if (false == next)
            break;

        next = false;
        for (int j = 0; j < MAX_NO_OF_THREADS; j ++)
        {
            if (threadPool.state[j] == THREAD_FREE)
            {
                EtlDecoder *decoder = new EtlDecoder;
                threadPool.decoder[j] = decoder;
                decoder->moveToThread(&threadPool.pool[j]);
                connect(this, SIGNAL(startDecoder(QString,QString)), decoder, SLOT(doDecoding(QString,QString)));
                connect(decoder, SIGNAL(stateReport(DecoderState,QString)), this, SLOT(handleStateReport(DecoderState,QString)));
                connect(decoder, SIGNAL(timeStampReport(QString,QDateTime,QDateTime)), this, SLOT(handleTimeStampReport(QString,QDateTime,QDateTime)));
                connect(decoder, SIGNAL(log(std::string,LogType)), this, SLOT(handleLog(std::string,LogType)));
                QString name = fileList->at(i).filePath();
                threadPool.etlFileName[j] = name;
                logger->Log(QString("Controller: send decoding request for %1").arg(name).toStdString(), LOG_INFO);
                emit startDecoder(name, destFolder);
                disconnect(this, SIGNAL(startDecoder(QString,QString)), nullptr, nullptr);
                threadPool.state[j] = THREAD_BUSY;
                next = true;
                this->nextFileIndex ++;
                break;
            }
        }
    }
}


void EtlDecoderCtrl::Stop()
{
    EtlDecoder::CancelDecoding(true);
    // Kill the tracerpt process for each EtlDecoder object
    for (int i = 0; i < MAX_NO_OF_THREADS; i ++)
    {
        if (threadPool.state[i] == THREAD_BUSY)
        {
            threadPool.decoder[i]->tracerptProcess->kill();
        }
    }
}

void EtlDecoderCtrl::handleStateReport(DecoderState state, QString etlFileName)
{
    // Notify the state to the GUI
    emit stateReport(state, etlFileName);

    for (int i = 0; i < MAX_NO_OF_THREADS; i ++)
    {
        if (threadPool.etlFileName[i] == etlFileName)
        {
            if (state != DECODER_STATE_STARTED)
            {
                if (noOfThreadUsed <= i)
                    threadPool.state[i] = THREAD_NOT_USED;
                else
                {
                    threadPool.state[i] = THREAD_FREE;

                    if (nextFileIndex >= fileList->size())
                    {
                        bool busy = false;

                        for (int j = 0; j < MAX_NO_OF_THREADS; j ++)
                        {
                            if (threadPool.state[j] == THREAD_BUSY)
                            {
                                busy = true;
                                break;
                            }
                        }
                        if (false == busy)
                        {
                            logger->Log(QString("Decoding completed(%1)").arg(QString::number(static_cast<double>(startTimeStamp.msecsTo(QDateTime::currentDateTime())) / 1000.0) + QString(" secs")).toStdString());
                            emit completed();
                        }
                        return;
                    }

                    EtlDecoder *decoder = new EtlDecoder;
                    threadPool.decoder[i] = decoder;
                    decoder->moveToThread(&threadPool.pool[i]);
                    connect(this, SIGNAL(startDecoder(QString,QString)), decoder, SLOT(doDecoding(QString,QString)));
                    connect(decoder, SIGNAL(stateReport(DecoderState,QString)), this, SLOT(handleStateReport(DecoderState,QString)));
                    connect(decoder, SIGNAL(timeStampReport(QString,QDateTime,QDateTime)), this, SLOT(handleTimeStampReport(QString,QDateTime,QDateTime)));
                    connect(decoder, SIGNAL(log(std::string,LogType)), this, SLOT(handleLog(std::string,LogType)));
                    QString name = fileList->at(nextFileIndex).filePath();
                    threadPool.etlFileName[i] = name;
                    logger->Log(QString("Controller: send decoding request %1").arg(name).toStdString(), LOG_INFO);
                    emit startDecoder(name, destFolder);
                    disconnect(this, SIGNAL(startDecoder(QString,QString)), nullptr, nullptr);
                    threadPool.state[i] = THREAD_BUSY;
                    nextFileIndex ++;
                }
            }
        }
    }
}


void EtlDecoderCtrl::handleTimeStampReport(QString etlFileName, QDateTime start, QDateTime stop)
{
    emit timeStampReport(etlFileName, start, stop);
}


void EtlDecoderCtrl::handleLog(std::string message, LogType type)
{
    logger->Log(message, type);
}
