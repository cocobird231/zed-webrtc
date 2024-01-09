#ifndef APPDATA_H
#define APPDATA_H

#define GST_USE_UNSTABLE_API

#include "appenum.h"
#include <glib.h>
#include <gst/gst.h>
#include <libsoup/soup.h>

extern GOptionEntry entries[];
extern void my_cmdline_check();

/*app state*/
extern enum AppState get_appstate();
extern void set_appstate(enum AppState state);

/** ZED Camera Streaming Props*/
typedef struct _ZEDStreamProp
{
    // zedsrc params
    gint cam_id;
    gint cam_fps;
    gint stream_type;

    // Output capabilities (Not supported yet)
    gint outWidth;
    gint outHeight;

    // H264 encode
    gint x264_tune;
    gint x264_qp;
    gint x264_preset;
    gboolean x264_threads;
} ZEDStreamProp;

/*MyAppData*/
typedef struct _Custom_GstData
{
    GstElement *pipeline1;
    GstElement  *webrtc1;
    GObject *send_channel, *receive_channel;
} Custom_GstData;

typedef struct _MyAppData
{
    enum AppState *app_state;
    SoupWebsocketConnection *ws_conn; //Non-essential

    /* GLib's Main Loop */
    GMainLoop *main_loop;

    /*gstreamer*/
    Custom_GstData *gstdata;

    /*server & peer*/
    const gchar *our_id;
    const gchar *peer_id;
    const gchar *server_url;
    const gchar *stun_server;
    const gchar *turn_server;
    gboolean remote_is_offerer;
    gboolean enable_turn_server;

    /** ZED Streaming props*/
    ZEDStreamProp zedStreamProp;

    
} MyAppData;


extern MyAppData *MyAppData_new();

static inline const gchar *get_our_id(MyAppData *myappdata)
{
    return myappdata->our_id;
}
static inline const gchar *get_peer_id(MyAppData *myappdata)
{
    return myappdata->peer_id;
}
static inline const gchar *get_server_url(MyAppData *myappdata)
{
    return myappdata->server_url;
}
static inline gboolean get_remote_is_offerer(MyAppData *myappdata)
{
    return myappdata->remote_is_offerer;
}

#endif