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

 
#ifndef EVTXDECODER_H
#define EVTXDECODER_H

#include <QObject>
#include <QString>
#include <QXmlStreamReader>
#include <QDateTime>
#include <QRegularExpression>
#include "EtlDecoder.h"


class EvtxDecoder : public EtlDecoder
{
    Q_OBJECT
public:
    explicit EvtxDecoder(QObject *parent = nullptr);
    ~EvtxDecoder();

private:
    QRegularExpression reRecords = QRegularExpression("^Records:\\s+(\\d+)$", QRegularExpression::MultilineOption);

    void ParseNoOfRecords();
    bool ParseXml(QString xmlFileName);
    bool ParseEvent(QTextStream& txtOut);

private slots:
    virtual void doDecoding(QString evtxFileName, QString destFolder);
};

#endif // EVTXDECODER_H
