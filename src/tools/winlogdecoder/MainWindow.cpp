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
 
 
#include <thread>
#include "MainWindow.h"
#include "EtlDecoderGui.h"
#include "ui_MainWindow.h"
#include "DialogAbout.h"
#include "Settings.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    etlDecoderGui = new EtlDecoderGui(this);
    setCentralWidget(etlDecoderGui);
    this->toolButtonSettings = new QToolButton();
    CreateToolBar();
    settingData.maxThreads = std::thread::hardware_concurrency();
    connect(ui->actionSettings, SIGNAL(triggered(bool)), this, SLOT(actionSettings_triggered()));
    connect(ui->actionExit, SIGNAL(triggered(bool)), this, SLOT(actionExit_triggered()));
    connect(ui->actionAbout, SIGNAL(triggered(bool)), this, SLOT(actionAbout_triggered()));
    connect(this->toolButtonSettings, SIGNAL(clicked(bool)), this, SLOT(actionSettings_triggered()));
    LoadSettings();
}


MainWindow::~MainWindow()
{
    SaveSettings();
    delete toolButtonSettings;
    delete etlDecoderGui;
    delete ui;
}


void MainWindow::CreateToolBar()
{
    this->toolButtonSettings->setIconSize(QSize(24,24));
    this->toolButtonSettings->setToolTip("Settings");
    this->toolButtonSettings->setIcon(QIcon(":/resource/gear.svg"));
    this->toolButtonSettings->setMinimumWidth(32);
    this->toolButtonSettings->setStyleSheet("border: none");
    this->toolButtonSettings->setCursor(QCursor(Qt::PointingHandCursor));
    ui->mainToolBar->addWidget(this->toolButtonSettings);
}


void MainWindow::LoadSettings()
{
    APP_SETTINGS;
    qDebug() << qPrintable(QString("Setting File: %1").arg(settings.fileName()));

    settingData.maxThreads = settings.value("maxThread", settingData.maxThreads).toUInt();
    if (settingData.maxThreads < 1 || settingData.maxThreads > MAX_NO_OF_THREADS)
        settingData.maxThreads = 8;

    etlDecoderGui->SetData(&settingData);
}


void MainWindow::SaveSettings()
{
    APP_SETTINGS;
    settings.setValue("maxThread", settingData.maxThreads);
}


void MainWindow::actionSettings_triggered()
{
    Settings *dlg = new Settings(&settingData, this);
    dlg->setFixedSize(dlg->size());
    dlg->exec();
    delete dlg;
    etlDecoderGui->SetData(&settingData);
}


void MainWindow::actionExit_triggered()
{
    close();
}


void MainWindow::actionAbout_triggered()
{
    DialogAbout *dlg = new DialogAbout(this, ":/resource/winlogdecoder.ico");
    dlg->setFixedSize(dlg->size());
    dlg->exec();
    delete dlg;
}

