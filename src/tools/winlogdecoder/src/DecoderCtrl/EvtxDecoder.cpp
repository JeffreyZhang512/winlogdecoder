#include "EvtxDecoder.h"

EvtxDecoder::EvtxDecoder(QObject *parent)
{

}


EvtxDecoder::~EvtxDecoder()
{

}


void EvtxDecoder::doDecoding(QString evtxFileName, QString destFolder)
{
    emit stateReport(DECODER_STATE_ERROR, evtxFileName.replace("\\", "/"));
    readyToBeClosed.acquire(1);
    deleteLater();
}
