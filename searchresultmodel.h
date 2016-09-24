/*
 * See License.txt for software license. File created by Louai Al-Khanji, 13.8.2011.
 */

#ifndef SEARCHRESULTMODEL_H
#define SEARCHRESULTMODEL_H

#include <QAbstractListModel>
#include "spotita.h"

class SearchResultModelPrivate;
class DeclarativeAlbumCoverProvider;

class SearchResultModel : public QAbstractListModel
{
    Q_OBJECT
public:
    enum ResultType { TrackResults, AlbumResults, ArtistResults };
    enum SearchResultRoles {
        TrackNameRole = Qt::UserRole,
        TrackArtistRole,
        TrackAlbumRole,
        TrackDurationRole,
        TrackPopularityRole,

        AlbumNameRole,
        AlbumArtistRole,
        AlbumCoverRole,
        AlbumYearRole,
        AlbumTypeRole,

        ArtistNameRole
    };

    explicit SearchResultModel(ResultType type = TrackResults, QObject *parent = 0);
    ~SearchResultModel();

    void setSearch(sp_search *search);

    virtual int rowCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;

    void registerAlbumCoverProvider(DeclarativeAlbumCoverProvider *provider);

private:
    SearchResultModelPrivate *m_d;

    QVariant trackData(const QModelIndex &index, int role) const;
    QVariant albumData(const QModelIndex &index, int role) const;
    QVariant artistData(const QModelIndex &index, int role) const;
};

#endif // SEARCHRESULTMODEL_H
