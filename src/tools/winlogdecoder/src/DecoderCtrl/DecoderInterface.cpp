#include "DecoderInterface.h"

DecoderInterface::DecoderInterface(QObject *parent)
    : QObject{parent}
{

}

void DecoderInterface::CancelDecoding(bool c)
{
    cancel = c;
}
