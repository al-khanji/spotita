/*
 * See License.txt for software license. File created by Louai Al-Khanji, 12.8.2011.
 */

#ifndef SPOTITA_H
#define SPOTITA_H

#include "libspotify/api.h"

#include <stdint.h>
#include <stdlib.h>

namespace global {
extern const uint8_t appkey[];
extern const size_t appkey_size;

inline int sample_bytes(sp_sampletype t) {
    int retval = 0;
    switch (t) {
    case SP_SAMPLETYPE_INT16_NATIVE_ENDIAN:
        return 2;
    }
    return retval;
}

}

namespace callback {
void SP_CALLCONV logged_in(sp_session *session, sp_error error);
void SP_CALLCONV notify_main_thread(sp_session *session);
int SP_CALLCONV music_delivery(sp_session *session,
                               const sp_audioformat *format,
                               const void *frames, int num_frames);
void SP_CALLCONV get_audio_buffer_stats(sp_session *session, sp_audio_buffer_stats *stats);
void SP_CALLCONV start_playback(sp_session *session);
void SP_CALLCONV stop_playback(sp_session *session);
void SP_CALLCONV search_complete_cb(sp_search *search, void *userdata);
}

#endif // SPOTITA_H
