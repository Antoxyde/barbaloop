#include <pulse/pulseaudio.h>

static pa_mainloop* mainloop = NULL;
static pa_mainloop_api* mainloop_api = NULL;
static pa_context* context = NULL;

// from https://gist.github.com/jasonwhite/1df6ee4b5039358701d2


static int pa_run(void) {

    int ret = 1;
    if (pa_mainloop_run(mainloop, &ret) < 0) {
        fprintf(stderr, "pa_mainloop_run() failed.\n");
        return ret;
    }
    
    return ret;
}

static void pa_quit(int ret) {
    
    mainloop_api->quit(mainloop_api, ret);

     if (context){
        pa_context_unref(context);
        context = NULL;
    }

    if (mainloop) {
        pa_mainloop_free(mainloop);
        mainloop = NULL;
        mainloop_api = NULL;
    }
}

static void sink_info_callback(pa_context *ctx, const pa_sink_info *i, int eol, void *userdata) {
    
    if (i) {
        float volume = (float)pa_cvolume_avg(&(i->volume)) / (float)PA_VOLUME_NORM;
        if (i->mute) {
            snprintf((char*)userdata, MAX_STATUS_SIZE, "%s%.0f%% (m)", CONF_COLOR_VOLUME_MUTED, volume * 100.0f);
        } else {
            snprintf((char*)userdata, MAX_STATUS_SIZE, "%s%.0f%%", CONF_COLOR_VOLUME, volume * 100.0f);
        }
        update_statusbar();
    }
}

static void server_info_callback(pa_context *ctx, const pa_server_info *i, void *userdata) {
    pa_context_get_sink_info_by_name(ctx, i->default_sink_name, sink_info_callback, userdata);
}

static void subscribe_callback(pa_context *ctx, pa_subscription_event_type_t type, uint32_t idx, void *userdata) {
    unsigned facility = type & PA_SUBSCRIPTION_EVENT_FACILITY_MASK;

    if (facility == PA_SUBSCRIPTION_EVENT_SINK) 
        pa_context_get_sink_info_by_index(ctx, idx, sink_info_callback, userdata);
}

// Called on context status change
static void context_state_callback(pa_context *ctx, void *userdata) {
    assert(ctx);

    switch (pa_context_get_state(ctx)) {
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
            break;

        case PA_CONTEXT_READY:
            fprintf(stderr, "PulseAudio connection established.\n");
            pa_context_get_server_info(ctx, server_info_callback, userdata);

            // Subscribe to sink events from the server. This is how we get
            // volume change notifications from the server.
            pa_context_set_subscribe_callback(ctx, subscribe_callback, userdata);
            pa_context_subscribe(ctx, PA_SUBSCRIPTION_MASK_SINK, NULL, NULL);
            break;

        case PA_CONTEXT_TERMINATED:
            pa_quit(0);
            fprintf(stderr, "PulseAudio connection terminated.\n");
            break;

        case PA_CONTEXT_FAILED:
        default:
            fprintf(stderr, "Connection failure: %s\n", pa_strerror(pa_context_errno(ctx)));
            pa_quit(1);
            break;
    }

}


static void *pa_setup(void* buf) {
    
    if ((mainloop = pa_mainloop_new()) == NULL) {
        fprintf(stderr, "pa_mainloop_new() failed.\n");
        return NULL;
    }
            
    mainloop_api = pa_mainloop_get_api(mainloop);

    if (pa_signal_init(mainloop_api) != 0) {
        fprintf(stderr, "pa_signal_init() failed\n");
        return NULL;
    }

    if ((context = pa_context_new(mainloop_api, "Barbaloop")) == NULL) {
        fprintf(stderr, "pa_context_new() failed\n");
        return NULL;
    }

    if (pa_context_connect(context, NULL, PA_CONTEXT_NOAUTOSPAWN, NULL) < 0) {
        fprintf(stderr, "pa_context_connect() failed: %s\n", pa_strerror(pa_context_errno(context)));
        return NULL;
    }

     pa_context_set_state_callback(context, context_state_callback, buf);

     pa_run();
     return NULL;
}
