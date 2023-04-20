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
 
 
#ifndef ETLDECODERGUI_H
#define ETLDECODERGUI_H

#include <QWidget>
#include <QFileInfo>
#include "EtlDecoderCtrl.h"
#include "LogInterface.h"
#include "Settings.h"


namespace Ui {
class EtlDecoderGui;
}


typedef enum
{
    RUNNING_STATE_STOPPED,
    RUNNING_STATE_STARTED
}RunningState;


class EtlDecoderGui : public QWidget, public LogInterface
{
    Q_OBJECT

public:
    explicit EtlDecoderGui(QWidget *parent = nullptr);
    ~EtlDecoderGui();
    void SetData(SettingData *data);

private:
    EtlDecoderCtrl *ctrl = nullptr;
    QFileInfoList fileList;
    Ui::EtlDecoderGui *ui;
    RunningState state = RUNNING_STATE_STOPPED;
    int noOfThreads = 8;

    virtual void ClearLog();
    virtual void Log(std::string log, LogType type = LOG_INFO);

private slots:
    void toolButtonOpenFolder_clicked();
    void pushButtonRun_clicked();
    void handleStateReport(DecoderState state, QString etlFileName);
    void handleTimeStampReport(QString etlFileName, QDateTime start, QDateTime stop);
    void handleCompleted();
};

#endif // ETLDECODERGUI_H
