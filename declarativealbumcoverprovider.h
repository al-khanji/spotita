/*
 * See License.txt for software license. File created by Louai Al-Khanji, 13.8.2011.
 */

#ifndef DECLARATIVEALBUMCOVERPROVIDER_H
#define DECLARATIVEALBUMCOVERPROVIDER_H

#include <QDeclarativeImageProvider>
#include <QHash>
#include <QCache>
#include <QWaitCondition>
#include <QMutex>

#include "spotita.h"

class SpotifySession;

class DeclarativeAlbumCoverProvider : public QDeclarativeImageProvider
{
public:
    DeclarativeAlbumCoverProvider(SpotifySession *session, QObject *parent = 0);
    virtual QImage requestImage(const QString &sid, QSize *size, const QSize &requestedSize);
    void notify();
    void startLoadingImage(const QByteArray &id);

private:
    QHash<QByteArray, sp_image*> m_pending;
    QCache<QByteArray, QImage> m_cache;
    QWaitCondition m_wait;
    QMutex m_mutex;
    SpotifySession *m_session;
};

#endif // DECLARATIVEALBUMCOVERPROVIDER_H
