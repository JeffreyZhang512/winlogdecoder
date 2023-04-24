#ifndef DECODERINTERFACE_H
#define DECODERINTERFACE_H

#include <QObject>
#include <QProcess>
#include <QSemaphore>
#include <LogInterface.h>


typedef enum
{
    DECODER_STATE_STARTED,
    DECODER_STATE_SUCCESS,
    DECODER_STATE_ERROR,
    DECODER_STATE_CANCELED,
    DECODER_STATE_MAX
}DecoderState;


class DecoderInterface : public QObject
{
    Q_OBJECT
public:
    DecoderInterface(QObject *parent = nullptr);
    static void CancelDecoding(bool c);
    QProcess * extProcess = nullptr;
    QSemaphore readyToBeClosed = QSemaphore();

signals:
    void stateReport(DecoderState state, QString etlFileName);
    void progressReport(QString etlFileName, unsigned int percentage);
    void timeStampReport(QString etlFileName, QDateTime start, QDateTime stop);
    void log(std::string message,  LogType type);

protected:
    inline static volatile bool cancel = false;
};

#endif // DECODERINTERFACE_H
