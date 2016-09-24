/*
 * See License.txt for software license. File created by Louai Al-Khanji, 13.8.2011.
 */

#include "declarativealbumcoverprovider.h"
#include "spotifysession.h"

static void SP_CALLCONV image_callback(sp_image *image, void *userdata)
{
    Q_UNUSED(image);
    DeclarativeAlbumCoverProvider *d = static_cast<DeclarativeAlbumCoverProvider *>(userdata);
    d->notify();
}

static QImage img_helper(const QImage * const source, QSize *size, QSize requestedSize)
{
    QImage result = requestedSize.isValid() ? source->scaled(requestedSize)
                                            : source->scaled(source->size());
    *size = result.size();
    return result;
}

DeclarativeAlbumCoverProvider::DeclarativeAlbumCoverProvider(SpotifySession *session, QObject *parent)
: QDeclarativeImageProvider(QDeclarativeImageProvider::Image)
, m_cache(15)
, m_session(session)
{}

QImage DeclarativeAlbumCoverProvider::requestImage(const QString &sid,
                                                   QSize *size,
                                                   const QSize &requestedSize)
{
    QByteArray id = QByteArray::fromBase64(sid.toLatin1());
    QImage img;
    QMutexLocker l(&m_mutex);

    forever {
        if (QImage *i = m_cache.object(id)) {
            img = img_helper(i, size, requestedSize);
            break;
        } else if (!m_pending.contains(id)) {
            m_pending[id] = m_session->requestImage(reinterpret_cast<const byte *>(id.constData()));
            sp_image_add_load_callback(m_pending[id], image_callback, this);
        } else if (sp_image_is_loaded(m_pending.value(id))) {
            size_t n_bytes = 0;
            const void *data = sp_image_data(m_pending.value(id), &n_bytes);
            QImage *i = new QImage;
            *i = QImage::fromData(static_cast<const uchar *>(data), n_bytes);
            m_cache.insert(id, i);
            img = img_helper(i, size, requestedSize);
            break;
        } else {
            m_wait.wait(&m_mutex);
        }
    }

    return img;
}

void DeclarativeAlbumCoverProvider::notify()
{
    QMutexLocker l(&m_mutex);
    m_wait.wakeAll();
}

void DeclarativeAlbumCoverProvider::startLoadingImage(const QByteArray &id)
{
    QMutexLocker l(&m_mutex);
    if (!m_pending.contains(id) && !m_cache.contains(id)) {
        m_pending[id] = m_session->requestImage(reinterpret_cast<const byte *>(id.constData()));
        sp_image_add_load_callback(m_pending[id], image_callback, this);
    }
}
