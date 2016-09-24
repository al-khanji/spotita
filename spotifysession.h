/*
 * See License.txt for software license. File created by Louai Al-Khanji, 12.8.2011.
 */

#ifndef SPOTIFYSESSION_H
#define SPOTIFYSESSION_H

#include <QObject>
#include <QAudio>

#include "spotita.h"

class QTimer;
class QAudioOutput;

class AudioBuffer;
class SearchResultModel;

class SpotifySession : public QObject
{
    Q_OBJECT
public:
    explicit SpotifySession(QObject *parent = 0);
    virtual ~SpotifySession();
    qint64 deliverMusic(const sp_audioformat *format, const void * frames, int num_frames);
    void fillAudioBufferStats(sp_audio_buffer_stats *stats);
    void addSearchResultModel(SearchResultModel *model);
    sp_image *requestImage(const byte id[20]);

    Q_INVOKABLE void playSearchTrack(int index);

public slots:
    void createSession();
    void logIn(const QString &username, const QString &password);
    void notifyMainThread();
    void loggedIn(sp_error err);
    void musicDiscontinuity();
    void checkPlayback();
    void audioStateChanged(QAudio::State state);
    void search(const QString &query);
    void updateSearchModels();
    void startPlayback();
    void stopPlayback();

signals:
    void loginSuccess();
    void loginFailure(const QString &error_message);
    void fatalError(const QString &error);

private:
    sp_session *m_session;
    QTimer *m_notifyTimer;
    AudioBuffer *m_audioBuffer;
    QAudioOutput *m_audioOut;
    sp_audioformat m_last_format;
    int m_stutters;
    sp_search *m_search;
    QList<SearchResultModel *> m_searchModels;
};

#endif // SPOTIFYSESSION_H
