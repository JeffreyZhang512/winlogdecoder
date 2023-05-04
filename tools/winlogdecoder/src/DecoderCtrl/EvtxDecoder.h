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
    QRegularExpression reRecord = QRegularExpression("^Record\\s+(\\d+)\n");

    bool ParseXml(QString xmlFileName);
    bool ParseEvent(QTextStream& txtOut);

private slots:
    virtual void doDecoding(QString evtxFileName, QString destFolder);
};

#endif // EVTXDECODER_H
