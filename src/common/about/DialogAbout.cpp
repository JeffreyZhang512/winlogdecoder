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
    Copyright (C) 2015-2017 Intel Deutschland GmbH

         Sec Class: Intel Confidential (IC)

     ---------------------------------------------------------------------- */

// Qt includes ---------------------------------------------------------------------------------------------------------
#include <QApplication>
#include <QDesktopServices>
#include <QMessageBox>
#include "DialogAbout.h"
#include "ui_DialogAbout.h"
#include "version.h"
#include "QDate"


LogoApp::LogoApp(QObject * parent) :
    QObject(parent)
{

}


bool LogoApp::eventFilter(QObject * watched, QEvent * event)
{
    if (event->type() == QEvent::MouseButtonPress ||
        event->type() == QEvent::MouseButtonRelease)
    {
        return true;
    }

    return false;
}



//---------------------------------------------------------------------------

DialogAbout::DialogAbout(QWidget *parent, QString logoResource, QString Version) :
  QDialog(parent),
  ui(new Ui::DialogAbout)
{
    ui->setupUi(this);

    connect(ui->labelLicense, SIGNAL(linkActivated(QString)), this, SLOT(labelLicense_linkActivated(QString)));
    connect(ui->labelAuthor, SIGNAL(linkActivated(QString)), this, SLOT(labelAuthor_linkActivated(QString)));
    connect(ui->pushButtonOK, SIGNAL(clicked()), this, SLOT(pushButtonOK_clicked()));

    QString LGPL_String = "LGPL2.1";
    if(QT_VERSION >= QT_VERSION_CHECK(5, 7, 0))
    {
        LGPL_String = "LGPLv3";
    }

    QString labelText = ui->labelLicense->text();
    labelText.replace("LGPLVERSION", LGPL_String);
    ui->labelLicense->setText(labelText);

#if defined APPLICATION_NAME
    ui->labelTitle->setText(APPLICATION_NAME);
#endif

    logo = new LogoApp(this);
    ui->toolButtonLogo->installEventFilter(logo);

    // Set the logo/icon (if present):
    if(!logoResource.isEmpty())
    {
        ui->toolButtonLogo->setIcon(QIcon(logoResource));
    }
    else
    {
        ui->toolButtonLogo->setVisible(false);
    }

#if defined APP_VERSION
    ui->labelVersion->setText(QString("Version %1\nBuild: %2").arg(APP_VERSION, GetBuildTime()));
#endif

#if defined APP_COPYRIGHT
    ui->labelCopyright->setText(QString(APP_COPYRIGHT).replace("YYYY", QString::number(QDate::currentDate().year())));
#endif
}
//---------------------------------------------------------------------------


DialogAbout::~DialogAbout()
{
    delete ui;
}
//---------------------------------------------------------------------------


QString DialogAbout::GetBuildTime()
{
    QString dateTime;
    dateTime += __DATE__;
    dateTime += " ";
    dateTime += __TIME__;
    return dateTime;
}
//---------------------------------------------------------------------------


void DialogAbout::pushButtonOK_clicked()
{
    QDialog::accept();
}
//---------------------------------------------------------------------------


void DialogAbout::labelLicense_linkActivated(const QString &link)
{
    Q_UNUSED(link)
    QApplication::aboutQt();
}
//---------------------------------------------------------------------------


void DialogAbout::labelAuthor_linkActivated(const QString& link)
{
    if(QDesktopServices::openUrl(QUrl(link)) == false)
    {
        QMessageBox::warning(this, "Warning", "Open Email client failed!");
    }
}
//---------------------------------------------------------------------------
