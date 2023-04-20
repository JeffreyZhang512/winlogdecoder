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
 
 
#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <QSettings>

namespace Ui {
class Settings;
}

#define APP_SETTINGS QSettings settings(QSettings::IniFormat, QSettings::UserScope, "", APPLICATION_SHORT_NAME)

typedef struct
{
    int maxThreads;
}SettingData;


class Settings : public QDialog
{
    Q_OBJECT

public:
    explicit Settings(SettingData *data, QWidget *parent = nullptr);
    ~Settings();

private:
    Ui::Settings *ui;
    SettingData *data = nullptr;

private slots:
    void buttonBox_accepted();
    void buttonBox_rejected();
};

#endif // SETTINGS_H
