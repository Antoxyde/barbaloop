#include <mpd/idle.h>
#include <mpd/tag.h>
#include <mpd/status.h>
#include <mpd/client.h>

static struct mpd_connection *conn;

static char *host = "localhost";
static unsigned int port = 6600;
//static unsigned int reconnect_time = 10;
static char *state_name[4] = {"unknown", "stopped", "now playing", "paused"};
static char* state;


void mpd_run(void* buf) {

    char *title, *artist;

    for (;;) {
        int idle_info;
        struct mpd_song *song;
        struct mpd_status *status;

        if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS)
            return;

        mpd_send_idle_mask(conn, MPD_IDLE_PLAYER);
        idle_info = mpd_recv_idle(conn, 1);
        if (!idle_info) return;
        if (idle_info == MPD_IDLE_PLAYER) {
            mpd_send_status(conn);
            status = mpd_recv_status(conn);
            if (!status) return;
            state = state_name[mpd_status_get_state(status)];

            fprintf(stderr, "[mpd] new event: %s\n", state);

            //mpd_status_free(status);
            mpd_send_current_song(conn);
            while ((song = mpd_recv_song(conn)) != NULL) {
                title = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);
                artist = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0);

                if (!strcmp(state, "paused")) {
                    snprintf((char*)buf, MAX_STATUS_SIZE, "%s%s - %s", CONF_COLOR_MPD_PAUSED, title, artist);
                } else if (!strcmp(state, "now playing")) {
                    snprintf((char*)buf, MAX_STATUS_SIZE, "%s%s - %s", CONF_COLOR_MPD, title, artist);
                } else {
                    snprintf((char*)buf, MAX_STATUS_SIZE, "%s%s - %s", CONF_COLOR_MPD_UNKOWN, title, artist);
                }

                update_statusbar();
                
                if (title) free(title);
                if (artist) free(artist);
                //mpd_song_free(song);
            }
        }
    }
}

void *mpd_setup(void* arg) {
    conn = mpd_connection_new(host, port, 0);
    
    mpd_run(arg);
    printf("ded\n");

    return NULL;
}
