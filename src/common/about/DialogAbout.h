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
 
 
/*  -------------------------------------------------------------------------
    Copyright (C) 2015 Intel Deutschland GmbH

         Sec Class: Intel Confidential (IC)

     ---------------------------------------------------------------------- */
#ifndef DIALOGABOUT_H
#define DIALOGABOUT_H

#include <QDialog>

namespace Ui {
class DialogAbout;
}

class LogoApp : public QObject
{
    Q_OBJECT
public:
    explicit LogoApp(QObject * parent = nullptr);
    virtual bool eventFilter(QObject * watched, QEvent * event) Q_DECL_OVERRIDE;
};


class DialogAbout : public QDialog
{
    Q_OBJECT

public:
    explicit DialogAbout(QWidget *parent = nullptr, QString logoResource = "", QString Version = "");
    virtual ~DialogAbout();

private slots:
    void pushButtonOK_clicked();
    void labelLicense_linkActivated(const QString& link);
    void labelAuthor_linkActivated(const QString& link);

private:
    Ui::DialogAbout *ui;
    QString m_licenseFile;
    QString m_licenseFolder;
    LogoApp *logo = nullptr;
    QString GetBuildTime();
};

#endif // DIALOGABOUT_H
