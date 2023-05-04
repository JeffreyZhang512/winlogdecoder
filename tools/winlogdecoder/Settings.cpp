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
 
 
#include "Settings.h"
#include "ui_Settings.h"

Settings::Settings(SettingData *data, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Settings)
{
    ui->setupUi(this);
    this->data = data;
    ui->spinBoxMaxThreads->setValue(data->maxThreads);
    connect(ui->buttonBox, SIGNAL(accepted()), this, SLOT(buttonBox_accepted()));
    connect(ui->buttonBox, SIGNAL(rejected()), this, SLOT(buttonBox_rejected()));
}

Settings::~Settings()
{
    delete ui;
}


void Settings::buttonBox_accepted()
{
    data->maxThreads = ui->spinBoxMaxThreads->value();
    this->done(QDialog::Accepted);
}


void Settings::buttonBox_rejected()
{
    this->done(QDialog::Rejected);
}
