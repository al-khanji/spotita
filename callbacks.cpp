/*
 * See License.txt for software license. File created by Louai Al-Khanji, 12.8.2011.
 */

#include "spotita.h"
#include "spotifysession.h"

#define EXTRACT_SESSION(ses) SpotifySession *s = static_cast<SpotifySession *>(sp_session_userdata(ses)); Q_ASSERT(s);

namespace callback {

void logged_in(sp_session *session, sp_error error)
{
    EXTRACT_SESSION(session);
    QMetaObject::invokeMethod(s, "loggedIn", Q_ARG(sp_error, error));
}

void notify_main_thread(sp_session *session)
{
    EXTRACT_SESSION(session);
    QMetaObject::invokeMethod(s, "notifyMainThread");
}

int music_delivery(sp_session *session, const sp_audioformat *format,
                   const void *frames, int num_frames)
{
    EXTRACT_SESSION(session);
    return s->deliverMusic(format, frames, num_frames);
}

void get_audio_buffer_stats(sp_session *session, sp_audio_buffer_stats *stats)
{
    EXTRACT_SESSION(session);
    s->fillAudioBufferStats(stats);
}

void start_playback(sp_session *session)
{
    EXTRACT_SESSION(session);
    QMetaObject::invokeMethod(s, "startPlayback");
}

void stop_playback(sp_session *session)
{
    EXTRACT_SESSION(session);
    QMetaObject::invokeMethod(s, "stopPlayback");
}

void search_complete_cb(sp_search *search, void *userdata)
{
    Q_UNUSED(search);
    EXTRACT_SESSION(static_cast<sp_session *>(userdata));
    QMetaObject::invokeMethod(s, "updateSearchModels");
}

}
