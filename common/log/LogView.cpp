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
 
 
#include "LogView.h"
#include "ui_LogView.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QKeyEvent>
#include <QClipboard>
#include <QString>
#include <QMenu>


LogView::LogView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LogView)
{
    ui->setupUi(this);

    QStringList tableHeader;
    tableHeader <<"Timestamp"<<"Type"<<"Message";
    ui->tableWidgetLogView->setHorizontalHeaderLabels(tableHeader);
    connect(ui->tableWidgetLogView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(contextMenu(QPoint)));
}


LogView::~LogView()
{
    delete ui;
}


void LogView::Log(const QString message, LogType type)
{ 
    QString msg;
    QString msgShort;
    QString time;
    int rowCount = ui->tableWidgetLogView->rowCount();
    QTime current = QTime::currentTime();
    time = current.toString("hh:mm:ss.zzz");
    QTableWidgetItem *data = new QTableWidgetItem;
    QBrush color;

    ui->tableWidgetLogView->insertRow(rowCount);
    ui->tableWidgetLogView->setItem(rowCount,0,new QTableWidgetItem(QString("%1").arg(time)));

    QFont fontSize = QFont("MS Shell Dlg 2");
    fontSize.setPointSize(8);
    ui->tableWidgetLogView->setFont(fontSize);

    QString messageShort = message;
    QRegularExpression re("[\\r\\n]");
    qsizetype index = messageShort.indexOf(re);
    if (index != -1)
    {
        messageShort = messageShort.left(index) + QString("...");
    }

    switch  (type)
    {
    case LOG_OK:
        ui->tableWidgetLogView->setItem(rowCount, 1, new QTableWidgetItem("OK"));
        data->setToolTip(message);
        data->setText(messageShort);
        data->setData(Qt::EditRole, message);
        color = QBrush(Qt::darkGreen);
        data->setForeground(color);
        ui->tableWidgetLogView->setItem(rowCount,2,data);
        break;

    case LOG_ERROR:
        msg = QString("%1%2%3").arg("!!! ", message, " !!!");
        msgShort = QString("%1%2%3").arg("!!! ", messageShort, " !!!");
        ui->tableWidgetLogView->setItem(rowCount, 1, new QTableWidgetItem("ERROR"));
        data->setToolTip(msg);
        data->setText(msgShort);
        data->setData(Qt::EditRole, msg);
        color = QBrush(Qt::red);
        data->setForeground(color);
        ui->tableWidgetLogView->setItem(rowCount, 2, data);
        break;

    case LOG_INFO:
    case LOG_STATUS:
        ui->tableWidgetLogView->setItem(rowCount, 1, new QTableWidgetItem("INFO"));
        data->setToolTip(message);
        data->setText(messageShort);
        data->setData(Qt::EditRole, message);
        // color = QBrush(qRgb(0, 162, 232));
        color = QBrush(Qt::blue);
        data->setForeground(color);
        ui->tableWidgetLogView->setItem(rowCount, 2, data);
        break;

    case LOG_WARNING:
        ui->tableWidgetLogView->setItem(rowCount,1,new QTableWidgetItem("WARNING"));
        data->setToolTip(message);
        data->setText(messageShort);
        data->setData(Qt::EditRole, message);
        color = QBrush(Qt::magenta);
        data->setForeground(color);
        ui->tableWidgetLogView->setItem(rowCount, 2, data);
        break;

    case LOG_DEBUG:
        ui->tableWidgetLogView->setItem(rowCount,1,new QTableWidgetItem("DEBUG"));
        data->setToolTip(message);
        data->setText(messageShort);
        data->setData(Qt::EditRole, message);
        color = QBrush(Qt::black);
        data->setForeground(color);
        ui->tableWidgetLogView->setItem(rowCount,2,data);
        break;

    default:
        delete data;
        break;
    }

    ui->tableWidgetLogView->scrollToBottom();
}


void LogView::contextMenu(QPoint point)
{
    Q_UNUSED(point)
    QMenu *menu = new QMenu();
    menu->addAction("Clear Log", this, SLOT(contextMenu_ClearLog()));
    menu->addAction("Copy Log To Clipboard", this, SLOT(contextMenu_CopyLogToClipboard()));
    menu->exec(QCursor::pos());
    delete menu;
}


void LogView::contextMenu_ClearLog()
{
    Clear();
}


void LogView::contextMenu_CopyLogToClipboard()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(GetAllLogText(), QClipboard::Clipboard);
}


void LogView::keyPressEvent(QKeyEvent * event)
{
    // Ctrl+C event
    if (event->matches(QKeySequence::Copy))
    {
        QClipboard *clipboard = QApplication::clipboard();
        if (clipboard != NULL)
        {
            QList<QTableWidgetItem *> selected(ui->tableWidgetLogView->selectedItems());
            QTableWidgetItem *item = nullptr, *PreviousItem = nullptr;
            QString str;

            foreach(item, selected)
            {
                if (PreviousItem != NULL &&
                        str.length() > 0 &&
                        PreviousItem->row() != item->row())
                {
                    str.append("\r\n");
                    str.append(item->text());
                }
                else
                {
                    if (str.length() > 0)
                    {
                        str.append("\t");
                    }
                    str.append(item->text());
                }
                PreviousItem = item;
            }
            clipboard->setText(str);
        }
    }
}


void LogView::Clear()
{
    ui->tableWidgetLogView->setRowCount(0);
}


QString LogView::GetAllLogText()
{
    QString str;
    for (int row = 0; row <ui->tableWidgetLogView->rowCount(); row ++)
    {
        for (int col = 0; col < ui->tableWidgetLogView->columnCount(); col ++)
        {
            QTableWidgetItem *item = ui->tableWidgetLogView->item(row, col);
            if (item != nullptr)
            {
                str.append(item->data(Qt::EditRole).toString());
                str.append("\t");
            }
        }
        str.append("\r\n");
    }
    return str;
}

