#include "appdata.h"
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <glib.h>

/** ZED Streaming*/
static gint cam_id = 0;
static gint cam_fps = 30;
static gint stream_type = 0;
static gint outWidth = 1280;
static gint outHeight = 720;
static gint x264_tune = 0x00000004;// GstX264EncTune: zerolatency
static gint x264_qp = 21;
static gint x264_preset = 4;// GstX264EncPreset: faster
static gboolean x264_threads = TRUE;

/*signaling + webrtc*/
static const gchar *our_id = NULL;
static const gchar *peer_id = NULL;
static const gchar *server_url = "http://61.220.23.239:8443";
static const gchar *stun_server = "stun://61.220.23.239";
static const gchar *turn_server = "turn://61.220.23.239";
static gboolean remote_is_offerer = FALSE;
static gboolean enable_turn_server = FALSE;

GOptionEntry entries[] = { {"cam-id", 0, 0, G_OPTION_ARG_INT, &cam_id, "Camera ID (int from 0 to 255)", "INT"}, 
                            {"stream-type", 0, 0, G_OPTION_ARG_INT, &stream_type, "zedsrc stream type", "INT"}, 

                            {"out-width", 0, 0, G_OPTION_ARG_INT, &outWidth, "Streaming width", "UINT"}, 
                            {"out-height", 0, 0, G_OPTION_ARG_INT, &outHeight, "Streaming height", "UINT"}, 

                            {"x264-tune", 0, 0, G_OPTION_ARG_INT, &x264_tune, "H264 tune", "INT"}, 
                            {"x264-qp", 0, 0, G_OPTION_ARG_INT, &x264_qp, "H264 quantizer", "INT"}, 
                            {"x264-preset", 0, 0, G_OPTION_ARG_INT, &x264_preset, "H264 preset", "INT"}, 

                            {"our-id", 0, 0, G_OPTION_ARG_STRING, &our_id, "Signaling ID of this local peer", "ID"}, 
                            {"peer-id", 0, 0, G_OPTION_ARG_STRING, &peer_id, "Signaling ID of the peer to connect to", "ID"}, 
                            {"server", 0, 0, G_OPTION_ARG_STRING, &server_url, "Signaling server to connect to", "URL"}, 
                            {"stun-server", 0, 0, G_OPTION_ARG_STRING, &stun_server, "STUN server", "URL"}, 
                            {"turn-server", 0, 0, G_OPTION_ARG_STRING, &turn_server, "TURN server", "URL"}, 
                            {"remote-offerer", 0, 0, G_OPTION_ARG_NONE, &remote_is_offerer, "Request that the peer generate the offer and we'll answer", NULL}, 
                            {"turn", 0, 0, G_OPTION_ARG_NONE, &enable_turn_server, "Enable turn server", NULL}, 
                            {NULL} };

extern void my_cmdline_check(void)
{
    g_print("cmdline parsed...\n");
    printf("--remote-offerer :%d\n", remote_is_offerer);
    printf("--server :%s\n", server_url);
    printf("--stun-server :%s\n", stun_server);
    printf("--turn-server :%s\n", turn_server);
    printf("--turn :%d\n", enable_turn_server);

    if (!our_id)
    {
        g_printerr("--our-id is a required argument\n");
        exit(-1);
    }
    printf("--our-id :%s\n", our_id);

    if (!remote_is_offerer)
    {
        if (!peer_id)
        {
            g_printerr("--peer-id is a required argument\n");
            exit(-1);
        }
        printf("--peer-id :%s\n", peer_id);
        printf("--cam-id :%d\n", cam_id);
        printf("--cam-fps :%d\n", cam_fps);
        printf("--stream-type :%d\n", stream_type);
        printf("--out-width :%d\n", outWidth);
        printf("--out-height :%d\n", outHeight);
        printf("--x264-tune :%d\n", x264_tune);
        printf("--x264-qp :%d\n", x264_qp);
        printf("--x264-preset :%d\n", x264_preset);
    }
}

/*app state*/
static enum AppState app_state = APP_STATE_UNKNOWN;
static enum AppState *get_appstate_addr()
{
    return &app_state;
}
extern enum AppState get_appstate()
{
    return app_state;
}
extern void set_appstate(enum AppState state)
{
    app_state = state;
}

/*MyAppData*/
static void MyAppData_init(MyAppData *app_data)
{
    app_data->app_state = &app_state;

    app_data->our_id = our_id;
    app_data->peer_id = peer_id;
    app_data->server_url = server_url;
    app_data->remote_is_offerer = remote_is_offerer;
    app_data->enable_turn_server = enable_turn_server;

    app_data->zedStreamProp.cam_id = cam_id;
    app_data->zedStreamProp.cam_fps = cam_fps;
    app_data->zedStreamProp.stream_type = stream_type;
    app_data->zedStreamProp.x264_tune = x264_tune;
    app_data->zedStreamProp.x264_qp = x264_qp;
    app_data->zedStreamProp.x264_preset = x264_preset;
    app_data->zedStreamProp.x264_threads = x264_threads;
    app_data->zedStreamProp.outWidth = outWidth;// Not supported yet
    app_data->zedStreamProp.outHeight = outHeight;// Not supported yet
}

extern MyAppData *MyAppData_new()
{
    g_print("new appdata\n");
    MyAppData *app_data = (MyAppData *)malloc(sizeof(MyAppData));
    memset(app_data, 0, sizeof(MyAppData));
    MyAppData_init(app_data);
    return app_data;
}
