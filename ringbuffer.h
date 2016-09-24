/*
 * See License.txt for software license. File created by Louai Al-Khanji, 12.8.2011.
 */

#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <algorithm>

template <class T, class S>
class RingBuffer {
public:
    RingBuffer(S size)
        : m_data(new T[size])
        , m_size(size)
        , m_stored(0)
        , m_start(0)
        , m_end(0)
    {}

    virtual ~RingBuffer() { delete[] m_data; }

    S size() const { return m_size; }
    S availableData() const { return m_stored; }
    S availableSpace() const { return size() - availableData();  }
    void clear() { m_start = m_end = m_stored = 0; }

    S write(const T *data, S max) {
        S written = 0, n;

        if (m_end >= m_start && availableSpace()) {
            n = std::min(max, m_size - m_end);
            copyIn(data, n);
            written += n;
            data += n;
            max -= n;
        }

        if (m_end < m_start) {
            n = std::min(max, m_start - m_start);
            copyIn(data, n);
            written += n;
            data += n;
            max -= n;
        }

        return written;
    }

    S read(T *data, S max) {
        S read = 0, n;

        if (m_start >= m_end && availableData()) {
            n = qMin(max, m_size - m_start);
            copyOut(data, n);
            read += n;
            max -= n;
        }

        if (m_start < m_end) {
            n = qMin(max, m_end - m_start);
            copyOut(data, n);
            read += n;
            max -= n;
        }

        return read;
    }

private:
    void copyIn(const T *data, S n) {
        qMemCopy(m_data + m_end, data, n);
        m_end += n; m_stored += n; m_end %= m_size;
    }

    void copyOut(T *data, S n) {
        qMemCopy(data, m_data + m_start, n);
        m_start += n; m_stored -= n; m_start %= m_size;
    }

    T *m_data;
    const S m_size;
    S m_stored, m_start, m_end;
};

#endif // RINGBUFFER_H
