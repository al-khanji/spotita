/*
 * See License.txt for software license. File created by Louai Al-Khanji, 12.8.2011.
 */

#include <QMutexLocker>

#include "audiobuffer.h"

AudioBuffer::AudioBuffer(QObject *parent)
: QIODevice(parent)
, m_buffer(1024 * 1024 * 1) // 1 MB
{
    open(QIODevice::ReadWrite);
}

bool AudioBuffer::isSequential() const
{
    return true;
}

qint64 AudioBuffer::readData(char *data, qint64 maxlen)
{
    QMutexLocker lock(&m_buffer_lock);
    return m_buffer.read(data, maxlen);
}

qint64 AudioBuffer::writeData(const char *data, qint64 len)
{
    m_buffer_lock.lock();
    qint64 written = m_buffer.write(data, len);
    m_buffer_lock.unlock();

    if (written > 0) {
        emit readyRead();
        emit bytesWritten(written);
    }

    return written;
}

bool AudioBuffer::reset()
{
    QMutexLocker lock(&m_buffer_lock);
    m_buffer.clear();
    return true;
}

bool AudioBuffer::atEnd() const
{
    return m_buffer.availableData() != 0;
}

qint64 AudioBuffer::bytesAvailable() const
{
    return m_buffer.availableData();
}
