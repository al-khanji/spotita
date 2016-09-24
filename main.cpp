#include <QtGui/QApplication>
#include <QDeclarativeContext>
#include <QDeclarativeEngine>
#include <QGraphicsObject>
#include <QThread>
#include <QIcon>

#include "qmlapplicationviewer.h"
#include "spotita.h"
#include "spotifysession.h"
#include "declarativealbumcoverprovider.h"
#include "searchresultmodel.h"

#define s(x) QLatin1String(x)

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName(s("Spotita"));
    app.setOrganizationName(s("al-khanji"));
    app.setWindowIcon(QIcon(s(":/spotita.png")));

    SpotifySession session;
    QMetaObject::invokeMethod(&session, "createSession", Qt::QueuedConnection);

    SearchResultModel trackResults(SearchResultModel::TrackResults);
    SearchResultModel artistResults(SearchResultModel::ArtistResults);
    SearchResultModel albumResults(SearchResultModel::AlbumResults);
    session.addSearchResultModel(&trackResults);
    session.addSearchResultModel(&artistResults);
    session.addSearchResultModel(&albumResults);

    DeclarativeAlbumCoverProvider *coverProvider = new DeclarativeAlbumCoverProvider(&session);
    albumResults.registerAlbumCoverProvider(coverProvider);

    QmlApplicationViewer viewer;
    viewer.setOrientation(QmlApplicationViewer::ScreenOrientationAuto);
    viewer.engine()->addImageProvider(s("spotify"), coverProvider);
    viewer.rootContext()->setContextProperty(s("spotify_session"), &session);
    viewer.rootContext()->setContextProperty(s("track_results"), &trackResults);
    viewer.rootContext()->setContextProperty(s("artist_results"), &artistResults);
    viewer.rootContext()->setContextProperty(s("album_results"), &albumResults);
#ifdef Q_WS_WIN
    viewer.setMainQmlFile(s("qml/spotita/desktop.qml"));
#else
    viewer.setMainQmlFile(s("qml/spotita/main.qml"));
#endif
    viewer.showExpanded();

    QObject::connect(viewer.rootObject(), SIGNAL(log_in(QString, QString)),
                     &session, SLOT(logIn(QString,QString)));
    QObject::connect(viewer.rootObject(), SIGNAL(search(QString)),
                     &session, SLOT(search(QString)));
    QObject::connect(viewer.rootObject(), SIGNAL(quit()),
                     &app, SLOT(quit()));

    return app.exec();
}
