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
 
 
#include "DecoderGui.h"
#include "DecoderCtrl.h"
#include "ui_DecoderGui.h"
#include <thread>
#include <QDesktopServices>
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QDir>


DecoderGui::DecoderGui(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DecoderGui)
{
    ui->setupUi(this);
    ctrl = new DecoderCtrl(this);
    connect(ctrl, SIGNAL(stateReport(DecoderState,QString)), this, SLOT(handleStateReport(DecoderState,QString)));
    connect(ctrl, SIGNAL(progressReport(QString,uint)), this, SLOT(handleProgressReport(QString,uint)));
    connect(ctrl, SIGNAL(timeStampReport(QString,QDateTime,QDateTime)), this, SLOT(handleTimeStampReport(QString,QDateTime,QDateTime)));
    connect(ctrl, SIGNAL(completed()), this, SLOT(handleCompleted()));
    connect(ui->toolButtonOpenFolder, SIGNAL(clicked(bool)), this, SLOT(toolButtonOpenFolder_clicked()));
    connect(ui->pushButtonRun, SIGNAL(clicked(bool)), this, SLOT(pushButtonRun_clicked()));
    connect(ui->tableWidget, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(tableWidget_cellDoubleClicked(int,int)));
    ui->splitter->setStretchFactor(0, 2);
    ui->splitter->setStretchFactor(1, 1);
    ui->logger->Log(QString("%1 Logical Processer(s)").arg(std::thread::hardware_concurrency()), LOG_INFO);
}


DecoderGui::~DecoderGui()
{
    delete ctrl;
    delete ui;
}


void DecoderGui::SetData(SettingData *data)
{
    noOfThreads = data->maxThreads;
    ctrl->SetMaxNoOfThreads(noOfThreads);
}


void DecoderGui::toolButtonOpenFolder_clicked()
{
    QFileDialog dlg(this);
    dlg.setDirectory(ui->lineEditSelectedFolder->text());
    dlg.setFileMode(QFileDialog::Directory);
    dlg.setOption(QFileDialog::ShowDirsOnly, true);
    dlg.setWindowTitle("Select the folder");
    dlg.setLabelText(QFileDialog::Accept, "Select");
    dlg.setLabelText(QFileDialog::Reject, "Cancel");
    if (dlg.exec())
    {
        ui->lineEditSelectedFolder->setText(dlg.directory().path());
    }
}


void DecoderGui::pushButtonRun_clicked()
{
    if (state == RUNNING_STATE_STOPPED)
    {
        // Check if the selected folder is a valid folder
        QString selectedFolder = ui->lineEditSelectedFolder->text();

        QDir d = QDir();
        if (false == d.exists(selectedFolder))
        {
            QMessageBox::critical(this, "Error", "The selected folder is not valid!");
            return;
        }

        // Clean the table
        // The method clear will delete the QTableWidgetItem objects automatically assigned to the table
        ui->tableWidget->clear();

        // Get the file lists
        d.setPath(selectedFolder);
        QStringList filters;
        // filters << "*.etl" << "*.evtx";
        // filters << "*.evtx";
        filters << "*.etl";
        fileList = d.entryInfoList(filters, QDir::Files, QDir::Name | QDir::IgnoreCase);
        int noOfLogFiles = fileList.size();

        if (noOfLogFiles == 0)
        {
            QMessageBox::critical(this, "Error", "There is no etl or evtx file in the selected folder!");
            return;
        }

        ui->tableWidget->setColumnCount(6);
        QStringList header;
        header << "Input"  << "State" << "Output" << "Start Time" << "Stop Time" << "Duration";
        ui->tableWidget->setHorizontalHeaderLabels(header);
        ui->tableWidget->setStyleSheet ("QHeaderView::section { background-color:lightblue }");
        ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
        ui->tableWidget->setRowCount(noOfLogFiles);

        for (int row = 0; row < noOfLogFiles; row ++)
        {
            for (int col = 0; col < 6; col ++)
            {
                QTableWidgetItem *item = new QTableWidgetItem;
                ui->tableWidget->setItem(row, col, item);
            }
        }

        ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        ui->tableWidget->horizontalHeader()->setMinimumSectionSize(120);
        for (int i = 0; i < noOfLogFiles; i ++)
        {
            QString name = fileList[i].fileName();
            qint64 size = fileList[i].size();
            double sizeK = static_cast<double>(size) / 1024;
            double sizeM = static_cast<double>(size) / (1024 * 1024);
            QString sizeString;

            if (sizeM >= 1)
                sizeString = QString::number(sizeM, 'f', 1) + "M";
            else if (sizeK >= 1)
                sizeString = QString::number(sizeK, 'f', 1) + "K";
            else
                sizeString = QString("%1").arg(size);

            QTableWidgetItem *item = ui->tableWidget->item(i, 0);
            item->setText(QString("%1(%2)").arg(name, sizeString));
            item->setToolTip(item->text());
        }

        QString destFolder;
        selectedFolder.replace("\\", "/");
        if (selectedFolder.right(1) == "/")
            destFolder = selectedFolder + QString("output");
        else
            destFolder = selectedFolder + QString("/output");

        if (false == QDir(destFolder).exists())
            d.mkdir("output");

        ctrl->SetMaxNoOfThreads(noOfThreads);
        ctrl->Start(&fileList, destFolder);
        ui->pushButtonRun->setText("Stop");
        state = RUNNING_STATE_STARTED;
    }
    else
    {
        ui->pushButtonRun->setEnabled(false);
        ctrl->Stop();
    }
}


void DecoderGui::tableWidget_cellDoubleClicked(int row, int col)
{
    if (col != 2)
        return;

    QTableWidgetItem *item = ui->tableWidget->item(row, col);
    if (item->text().isEmpty())
        return;

    QString txtFileName = fileList[row].path() + QString("/output/") + fileList[row].fileName() + QString(".xml.txt");
    QDesktopServices::openUrl(QUrl::fromLocalFile(txtFileName));
}


void DecoderGui::handleStateReport(DecoderState state, QString fileName)
{
    int noOfLogFiles = fileList.size();
    int index = -1;
    for (int i = 0; i < noOfLogFiles; i ++)
    {
        if (fileName == fileList[i].filePath())
        {
            index = i;
            break;
        }
    }

    if (index != -1)
    {
        QTableWidgetItem *item = ui->tableWidget->item(index, 1);
        QTableWidgetItem *itemOut = ui->tableWidget->item(index, 2);
        if (item == nullptr || itemOut == nullptr)
            return;
        QString stateString;
        QBrush brush;
        switch (state)
        {
        case DECODER_STATE_STARTED:
            stateString = QString("Running");
            brush = QBrush(Qt::blue);
            item->setForeground(brush);
            break;
        case DECODER_STATE_SUCCESS:
            stateString = QString("Success(100%)");
            brush = QBrush(Qt::darkGreen);
            item->setForeground(brush);
            itemOut->setText(QString("output/") + QFileInfo(fileName).fileName() + QString(".txt"));
            itemOut->setToolTip("Double click to open");
            break;
        case DECODER_STATE_ERROR:
            stateString = QString("Error");
            brush = QBrush(Qt::red);
            item->setForeground(brush);
            break;
        case DECODER_STATE_CANCELED:
            stateString = QString("Canceled");
            brush = QBrush(Qt::magenta);
            item->setForeground(brush);
            break;
        default:
            break;
        }
        item->setText(stateString);
    }
}


void DecoderGui::handleProgressReport(QString fileName, unsigned int percentage)
{
    int noOfLogFiles = fileList.size();
    int index = -1;
    for (int i = 0; i < noOfLogFiles; i ++)
    {
        if (fileName == fileList[i].filePath())
        {
            index = i;
            break;
        }
    }

    if (index != -1)
    {
        QTableWidgetItem *item = ui->tableWidget->item(index, 1);
        item->setText(QString("Running(%1%)").arg(percentage));
    }
}


void DecoderGui::handleTimeStampReport(QString fileName, QDateTime start, QDateTime stop)
{
    int noOfLogFiles = fileList.size();
    int index = -1;
    for (int i = 0; i < noOfLogFiles; i ++)
    {
        if (fileName == fileList[i].filePath())
        {
            index = i;
            break;
        }
    }

    if (index != -1)
    {
        QTableWidgetItem *itemStart = ui->tableWidget->item(index, 3);
        QTableWidgetItem *itemStop = ui->tableWidget->item(index, 4);
        QTableWidgetItem *itemDuration = ui->tableWidget->item(index, 5);
        if (itemStart == nullptr || itemStop == nullptr || itemDuration == nullptr)
            return;

        itemStart->setText(start.toString(Qt::ISODateWithMs));
        itemStop->setText(stop.toString(Qt::ISODateWithMs));
        itemDuration->setText(QString::number(static_cast<double>(start.msecsTo(stop)) / 1000.0) + QString(" secs"));
    }
}


void DecoderGui::handleCompleted()
{
    ui->pushButtonRun->setText("Start");
    ui->pushButtonRun->setEnabled(true);
    state = RUNNING_STATE_STOPPED;
    QMessageBox::information(this, "Completed", "Decoding completed");
}


void DecoderGui::Log(std::string log, LogType type)
{
    ui->logger->Log(QString(log.c_str()), type);
}
