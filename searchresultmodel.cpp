/*
 * See License.txt for software license. File created by Louai Al-Khanji, 13.8.2011.
 */

#include <QStringList>
#include <QSet>

#include "searchresultmodel.h"
#include "declarativealbumcoverprovider.h"

class SearchResultModelPrivate
{
public:
    SearchResultModel *model_;
    sp_search *search_;
    QSet<DeclarativeAlbumCoverProvider *> providers;

    SearchResultModelPrivate(SearchResultModel *m) : model_(m), search_(0) {}
    virtual ~SearchResultModelPrivate() {}

    virtual int rows() const = 0;
    virtual QVariant data(const QModelIndex &index, int role) const = 0;
    virtual QHash<int, QByteArray> roles() const = 0;
};

class TrackResultPrivate : public SearchResultModelPrivate
{
public:
    TrackResultPrivate(SearchResultModel *m) : SearchResultModelPrivate(m) {}

    virtual int rows() const {
        return sp_search_num_tracks(search_);
    }

    virtual QVariant data(const QModelIndex &index, int role) const {
        QVariant result;
        sp_track *track = sp_search_track(search_, index.row());

        switch (role) {
        case SearchResultModel::TrackNameRole:
            result = QString::fromUtf8(sp_track_name(track));
            break;
        case SearchResultModel::TrackArtistRole: {
            QStringList artists;
            int n_artists = sp_track_num_artists(track);
            for (int i = 0; i < n_artists; i++)
                artists << QString::fromUtf8(sp_artist_name(sp_track_artist(track, i)));
            result = artists;
            }
            break;
        case SearchResultModel::TrackAlbumRole:
            result = QString::fromUtf8(sp_album_name(sp_track_album(track)));
            break;
        case SearchResultModel::TrackDurationRole:
            result = sp_track_duration(track);
            break;
        case SearchResultModel::TrackPopularityRole:
            result = sp_track_popularity(track);
            break;
        }

        return result;
    }

    virtual QHash<int, QByteArray> roles() const {
        QHash<int, QByteArray> roles;
        roles[SearchResultModel::TrackNameRole]       = "trackName";
        roles[SearchResultModel::TrackArtistRole]     = "trackArtist";
        roles[SearchResultModel::TrackAlbumRole]      = "trackAlbum";
        roles[SearchResultModel::TrackDurationRole]   = "trackDuration";
        roles[SearchResultModel::TrackPopularityRole] = "trackPopularity";
        return roles;
    }
};

class AlbumResultPrivate : public SearchResultModelPrivate
{
public:
    AlbumResultPrivate(SearchResultModel *m) : SearchResultModelPrivate(m) {}

    virtual int rows() const {
        return sp_search_num_albums(search_);
    }

    virtual QVariant data(const QModelIndex &index, int role) const {
        QVariant result;
        sp_album *album = sp_search_album(search_, index.row());

        switch (role) {
        case SearchResultModel::AlbumNameRole:
            result = QString::fromUtf8(sp_album_name(album));
            break;
        case SearchResultModel::AlbumArtistRole:
            result = QString::fromUtf8(sp_artist_name(sp_album_artist(album)));
            break;
        case SearchResultModel::AlbumCoverRole: {
            const byte *id = sp_album_cover(album);
            QByteArray b(reinterpret_cast<const char *>(id), 20);
            foreach (DeclarativeAlbumCoverProvider *d, providers) {
                d->startLoadingImage(b);
            }
            QString s = QString::fromLatin1(b.toBase64());
            result = QLatin1String("image://spotify/") + s;
            }
            break;
        case SearchResultModel::AlbumYearRole:
            result = sp_album_year(album);
            break;
        case SearchResultModel::AlbumTypeRole:
            result = sp_album_type(album);
            break;
        }

        return result;
    }

    virtual QHash<int, QByteArray> roles() const {
        QHash<int, QByteArray> roles;
        roles[SearchResultModel::AlbumNameRole]   = "albumName";
        roles[SearchResultModel::AlbumArtistRole] = "albumArtist";
        roles[SearchResultModel::AlbumCoverRole]  = "albumCover";
        roles[SearchResultModel::AlbumYearRole]   = "albumYear";
        roles[SearchResultModel::AlbumTypeRole]   = "albumType";
        return roles;
    }
};

class ArtistResultPrivate : public SearchResultModelPrivate
{
public:
    ArtistResultPrivate(SearchResultModel *m) : SearchResultModelPrivate(m) {}

    virtual int rows() const {
        return sp_search_num_artists(search_);
    }

    virtual QVariant data(const QModelIndex &index, int role) const {
        QVariant result;
        sp_artist *artist = sp_search_artist(search_, index.row());

        switch (role) {
        case SearchResultModel::ArtistNameRole:
            result = QString::fromUtf8(sp_artist_name(artist));
            break;
        }

        return result;
    }

    virtual QHash<int, QByteArray> roles() const {
        QHash<int, QByteArray> roles;
        roles[SearchResultModel::ArtistNameRole] = "artistName";
        return roles;
    }
};

SearchResultModel::SearchResultModel(ResultType type, QObject *parent)
: QAbstractListModel(parent)
, m_d(0)
{
    switch (type) {
    case TrackResults:
        m_d = new TrackResultPrivate(this);
        break;
    case AlbumResults:
        m_d = new AlbumResultPrivate(this);
        break;
    case ArtistResults:
        m_d = new ArtistResultPrivate(this);
        break;
    }

    setRoleNames(m_d->roles());
}

SearchResultModel::~SearchResultModel()
{
    delete m_d;
}

void SearchResultModel::setSearch(sp_search *search)
{
    beginResetModel();
    m_d->search_ = search;
    endResetModel();
}

int SearchResultModel::rowCount(const QModelIndex &parent) const
{
    int rows = 0;

    if (m_d->search_ && sp_search_is_loaded(m_d->search_) && !parent.isValid()) {
        rows = m_d->rows();
    }

    return rows;
}

QVariant SearchResultModel::data(const QModelIndex &index, int role) const
{
    QVariant result;

    if (m_d->search_ && sp_search_is_loaded(m_d->search_) && index.isValid())
        result = m_d->data(index, role);

    return result;
}

void SearchResultModel::registerAlbumCoverProvider(DeclarativeAlbumCoverProvider *provider)
{
    m_d->providers.insert(provider);
}
