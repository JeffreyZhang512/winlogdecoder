#ifndef EVTXDECODER_H
#define EVTXDECODER_H

#include <QObject>

class EvtxDecoder : public QObject
{
    Q_OBJECT
public:
    explicit EvtxDecoder(QObject *parent = nullptr);

signals:

};

#endif // EVTXDECODER_H
