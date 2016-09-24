/*
 * See License.txt for software license. File created by Louai Al-Khanji, 12.8.2011.
 */

#ifndef AUDIOBUFFER_H
#define AUDIOBUFFER_H

#include <QIODevice>
#include <QMutex>

#include "spotita.h"
#include "ringbuffer.h"

class AudioBuffer : public QIODevice
{
    Q_OBJECT
public:
    explicit AudioBuffer(QObject *parent = 0);
    virtual bool isSequential() const;
    virtual bool reset();
    virtual bool atEnd() const;
    virtual qint64 bytesAvailable() const;

protected:
    virtual qint64 readData(char *data, qint64 maxlen);
    virtual qint64 writeData(const char *data, qint64 len);

private:
    RingBuffer<char, qint64> m_buffer;
    QMutex m_buffer_lock;

};

#endif // AUDIOBUFFER_H
