/*
 * See License.txt for software license. File created by Louai Al-Khanji, 12.8.2011.
 */

#include <QApplication>
#include <QTimer>
#include <QSettings>
#include <QFileInfo>
#include <QDesktopServices>
#include <QDir>
#include <QAudioOutput>
#include <QSysInfo>

#include "spotifysession.h"
#include "spotita.h"
#include "audiobuffer.h"
#include "searchresultmodel.h"

SpotifySession::SpotifySession(QObject *parent)
: QObject(parent)
, m_session(0)
, m_notifyTimer(new QTimer(this))
, m_audioBuffer(new AudioBuffer(this))
, m_audioOut(0)
, m_stutters(0)
, m_search(0)
{}

SpotifySession::~SpotifySession()
{
    if (m_session) {
        m_notifyTimer->stop();
        if (sp_session_connectionstate(m_session) != SP_CONNECTION_STATE_LOGGED_OUT)
            sp_session_logout(m_session);
        sp_session_release(m_session);
    }
}

void SpotifySession::logIn(const QString &username, const QString &password)
{
    sp_session_login(m_session,
                     username.toUtf8().constData(),
                     password.toUtf8().constData());
}

void SpotifySession::notifyMainThread()
{
    int timeout;

    sp_session_process_events(m_session, &timeout);
    m_notifyTimer->start(timeout);
}

void SpotifySession::loggedIn(sp_error err)
{
    if (err == SP_ERROR_OK)
        emit loginSuccess();
    else
        emit loginFailure(QString::fromUtf8(sp_error_message(err)));
}

void SpotifySession::createSession()
{
    sp_session_config conf;
    sp_session_callbacks call;
    sp_error err;

    connect(m_notifyTimer, SIGNAL(timeout()), SLOT(notifyMainThread()));

    qMemSet(&call, 0, sizeof(call));
    call.logged_in = callback::logged_in;
    call.notify_main_thread = callback::notify_main_thread;
    call.music_delivery = callback::music_delivery;
    call.get_audio_buffer_stats = callback::get_audio_buffer_stats;
    call.start_playback = callback::start_playback;
    call.stop_playback = callback::stop_playback;

    conf.api_version = SPOTIFY_API_VERSION;
    QByteArray cache_loc = QDesktopServices::storageLocation(QDesktopServices::DataLocation).toUtf8();
    conf.cache_location = cache_loc.constData();
    QSettings settings(QSettings::IniFormat, QSettings::UserScope, qApp->organizationName(), qApp->applicationName());
    QByteArray settings_loc = QFileInfo(settings.fileName()).dir().path().toUtf8();
    conf.settings_location = settings_loc.constData();
    conf.application_key = global::appkey;
    conf.application_key_size = global::appkey_size;
    conf.user_agent = "Spotita";
    conf.callbacks = &call;
    conf.userdata = this;
    conf.compress_playlists = false;
    conf.dont_save_metadata_for_playlists = false;
    conf.initially_unload_playlists = false;

    if ((err = sp_session_create(&conf, &m_session)) != SP_ERROR_OK) {
        emit fatalError(QString::fromUtf8(sp_error_message(err)));
    }
}

qint64 SpotifySession::deliverMusic(const sp_audioformat *format, const void *frames, int num_frames)
{
    if (num_frames == 0) {
        QMetaObject::invokeMethod(this, "musicDiscontinuity", Qt::QueuedConnection);
        return 0;
    } else {
        qint64 factor = format->channels * global::sample_bytes(format->sample_type);
        qint64 written = m_audioBuffer->write(static_cast<const char *>(frames), num_frames * factor);
        m_last_format = *format;
        QMetaObject::invokeMethod(this, "checkPlayback", Qt::QueuedConnection);
        return written / factor;
    }
}

void SpotifySession::musicDiscontinuity()
{
    if (m_audioOut) {
        m_audioOut->stop();
        m_audioOut->reset();
        m_audioBuffer->reset();
    }
}

void SpotifySession::checkPlayback()
{
    if (!m_audioOut) {
        QAudioFormat format;
        format.setCodec(QLatin1String("audio/pcm"));
        format.setChannelCount(m_last_format.channels);
        format.setSampleRate(m_last_format.sample_rate);

        switch (m_last_format.sample_type) {
        case SP_SAMPLETYPE_INT16_NATIVE_ENDIAN:
            format.setSampleSize(16);
            format.setSampleType(QAudioFormat::SignedInt);
            format.setByteOrder(QSysInfo::ByteOrder == QSysInfo::LittleEndian ? QAudioFormat::LittleEndian
                                                                              : QAudioFormat::BigEndian);
            break;
        }

        m_audioOut = new QAudioOutput(format, this);
        connect(m_audioOut, SIGNAL(stateChanged(QAudio::State)), SLOT(audioStateChanged(QAudio::State)));
        m_audioOut->start(m_audioBuffer);
    }

    if (m_audioOut->state() == QAudio::IdleState) {
        m_audioOut->start(m_audioBuffer);
    }
}

void SpotifySession::audioStateChanged(QAudio::State state)
{
    switch (state) {
    case QAudio::IdleState:
        m_stutters += 1;
        break;
    case QAudio::StoppedState:
        break;
    default:
        break;
    }
}

void SpotifySession::fillAudioBufferStats(sp_audio_buffer_stats *stats)
{
    stats->stutter = m_stutters;
    m_stutters = 0;

    stats->samples = 0;
    if (m_audioOut) {
        stats->samples = m_audioBuffer->bytesAvailable();
        stats->samples /= m_audioOut->format().channelCount();
        stats->samples /= m_audioOut->format().sampleSize() / 8;
    }
}

void SpotifySession::search(const QString &query)
{
    if (m_search) {
        sp_search *s = m_search;
        m_search = 0;
        updateSearchModels();
        sp_search_release(s);
    }

    m_search = sp_search_create(m_session, query.toUtf8().constData(),
                                0, 10, 0, 10, 0, 10,
                                callback::search_complete_cb, m_session);
}

void SpotifySession::updateSearchModels()
{
    foreach (SearchResultModel *m, m_searchModels)
        m->setSearch(m_search);
}

void SpotifySession::startPlayback()
{
    checkPlayback();
    m_audioOut->start(m_audioBuffer);
}

void SpotifySession::stopPlayback()
{
    if (m_audioOut)
        m_audioOut->stop();
}

void SpotifySession::addSearchResultModel(SearchResultModel *model)
{
    if  (!m_searchModels.contains(model)) {
        m_searchModels.append(model);
        if (m_search && sp_search_is_loaded(m_search)) {
            model->setSearch(m_search);
        }
    }
}

sp_image *SpotifySession::requestImage(const byte id[])
{
    return sp_image_create(m_session, id);
}

void SpotifySession::playSearchTrack(int index)
{
    if (m_search && sp_search_is_loaded(m_search)) {
        sp_track *track = sp_search_track(m_search, index);
        musicDiscontinuity();
        sp_session_player_load(m_session, track);
        sp_session_player_play(m_session, true);
    }
}
