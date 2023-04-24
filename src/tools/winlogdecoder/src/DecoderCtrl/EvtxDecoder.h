#ifndef EVTXDECODER_H
#define EVTXDECODER_H

#include <QObject>
#include <QString>
#include "DecoderInterface.h"

class EvtxDecoder : public DecoderInterface
{
    Q_OBJECT
public:
    explicit EvtxDecoder(QObject *parent = nullptr);
    ~EvtxDecoder();

private slots:
    void doDecoding(QString evtxFileName, QString destFolder);
};

#endif // EVTXDECODER_H
