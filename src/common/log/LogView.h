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
 
 
#ifndef LOGVIEW_H
#define LOGVIEW_H

#include <QWidget>
#include <QDateTime>
#include <QTableWidgetItem>


#include "LogInterface.h"


namespace Ui {
class LogView;
}


class LogView : public QWidget
{
    Q_OBJECT

public:
    explicit LogView(QWidget *parent = nullptr);
    virtual ~LogView();
    void Log(const QString message, LogType type);
    void Clear();

    QString GetAllLogText();

protected:
    void keyPressEvent(QKeyEvent *event);

private slots:
    void contextMenu(QPoint point);
    void contextMenu_ClearLog();
    void contextMenu_CopyLogToClipboard();

private:
    Ui::LogView *ui;
};

#endif // LOGVIEW_H
