#include <gst/gst.h>
#include <gst/sdp/sdp.h>

#include <gst/webrtc/webrtc.h>

#include <string.h>

/* MY function*/
#include "appdata.h"
#include "pipeline.h"
#include "my_webrtc/my_webrtc.h"
#include "media/recievemedia.h"

// #define STUN_SERVER_URL "stun://61.220.23.239"
// #define TURN_SERVER_URL "turn://61.220.23.239"

static gboolean bus_message_cb(GstBus *bus, GstMessage *message, Custom_GstData *gstdata)
{
    g_print("BUS: got %s message\n", GST_MESSAGE_TYPE_NAME(message));

    switch(GST_MESSAGE_TYPE(message))
    {
        case GST_MESSAGE_ERROR:
        {
            GError *err;
            gchar *debug_info;

            /* Print error details on the screen */
            gst_message_parse_error(message, &err, &debug_info);
            g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME (message->src), err->message);
            g_printerr("Debugging information: %s\n", debug_info ? debug_info : "none");
            g_clear_error(&err);
            g_free(debug_info);

            //pipeline replay
            gst_element_set_state(gstdata->pipeline1, GST_STATE_NULL);
            gst_element_set_state(gstdata->pipeline1, GST_STATE_PLAYING);

            //g_main_loop_quit (app_data->main_loop);
            break;
        }
        case GST_MESSAGE_STATE_CHANGED:
        {
            if (GST_MESSAGE_SRC(message) == GST_OBJECT(gstdata->pipeline1))
            {
                GstState old_state, new_state, pending_state;
                gst_message_parse_state_changed(message, &old_state, &new_state, &pending_state);
                g_print("Pipeline state changed from %s to %s:\n", gst_element_state_get_name (old_state), gst_element_state_get_name (new_state));
            }
            break;
        }
        default:
            break;
    }
    return TRUE;
}

static void error_cb(GstBus *bus, GstMessage *msg, Custom_GstData *gstdata)
{
    GError *err;
    gchar *debug_info;

    /* Print error details on the screen */
    gst_message_parse_error(msg, &err, &debug_info);
    g_printerr("Error received from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
    g_printerr("Debugging information: %s\n", debug_info ? debug_info : "none");
    g_clear_error(&err);
    g_free(debug_info);

    //pipeline replay
    gst_element_set_state(gstdata->pipeline1, GST_STATE_NULL);
    gst_element_set_state(gstdata->pipeline1, GST_STATE_PLAYING);

    //g_main_loop_quit (app_data->main_loop);
}

static void state_changed_cb(GstBus *bus, GstMessage *msg, Custom_GstData *gstdata)
{
    if (GST_MESSAGE_SRC(msg) == GST_OBJECT(gstdata->pipeline1))
    {
    	GstState old_state, new_state, pending_state;
    	gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
    	g_print("GSTREAMER: Pipeline state changed from %s to %s:\n", gst_element_state_get_name(old_state), gst_element_state_get_name(new_state));
    }
}

static void stream_status_cb(GstBus *bus, GstMessage *message, gpointer user_data)
{
    GstStreamStatusType type;
    GstElement *owner;
    const GValue *val;
    GstTask *task = NULL;

    gst_message_parse_stream_status(message, &type, &owner);

    val = gst_message_get_stream_status_object(message);

    /* see if we know how to deal with this object */
    if (G_VALUE_TYPE (val) == GST_TYPE_TASK)
    {
        task = g_value_get_object(val);
    }

    switch (type)
    {
        case GST_STREAM_STATUS_TYPE_CREATE:
            if (task)
            {
                /*
                GstTaskPool *pool;
                pool = test_rt_pool_new();
                gst_task_set_pool (task, pool);
                */
                g_print("BUS_MESSAGE: new stream intend to created\n");
            }
            break;
        case GST_STREAM_STATUS_TYPE_ENTER:
            if (task)
            {
                g_print("BUS_MESSAGE: stream task enter loop\n");
            }
            break;
        default:
            break;
    }
}

static void pipeline_error(Custom_GstData *gstdata)
{
    if (gstdata->pipeline1)
        g_clear_object (&gstdata->pipeline1);
    if (gstdata->webrtc1)
        gstdata->webrtc1 = NULL;
}

// zedsrc ! autovideoconvert ! queue ! x264enc ! rtph264pay
static void create_media_bins_and_link(GstElement *pipe, GstElement *wbin, MyAppData *app_data)
{
    GstPad *r_pad;
    GstPad *w_pad;
    GstElement *source, *convert, *scale, *x264e, *queue, *rh264pay;
    GstCaps *caps;
    
    g_print("Trying to create media bins with: zedsrc camera-resolution=1 camera-fps=30 ! autovideoconvert ! videoscale ! 'video/x-raw,format=(string)I420,width=1280,height=720' ! queue ! x264enc tune=zerolatency ! rtph264pay ! webrtcbin.pad \n");
    source = gst_element_factory_make("zedsrc", "source");
    g_assert_nonnull (source);
    convert = gst_element_factory_make("autovideoconvert", "convert");
    g_assert_nonnull (convert);
    scale = gst_element_factory_make("videoscale", "scale");
    g_assert_nonnull (scale);
    caps = gst_caps_new_simple("video/x-raw",
                                "format", G_TYPE_STRING, "I420",
                                // "framerate", GST_TYPE_FRACTION, 30, 1,
                                // "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
                                "width", G_TYPE_INT, app_data->zedStreamProp.outWidth,
                                "height", G_TYPE_INT, app_data->zedStreamProp.outHeight,
                                NULL);
    g_assert_nonnull (caps);
    queue = gst_element_factory_make("queue", "queue");
    g_assert_nonnull (queue);
    x264e = gst_element_factory_make("x264enc", "x264enc");
    g_assert_nonnull (x264e);
    rh264pay = gst_element_factory_make("rtph264pay", NULL);
    g_assert_nonnull (rh264pay);
    
    gst_bin_add_many (GST_BIN (pipe), source, convert, scale, queue, x264e, rh264pay, NULL);
    gst_element_sync_state_with_parent(source);
    gst_element_sync_state_with_parent(convert);
    gst_element_sync_state_with_parent(scale);
    gst_element_sync_state_with_parent(queue);
    gst_element_sync_state_with_parent(x264e);
    gst_element_sync_state_with_parent(rh264pay);

    gst_element_link_many(source, convert, scale, NULL);
    gst_element_link_filtered(scale, queue, caps);
    gst_element_link_many(queue, x264e, rh264pay, NULL);

    // Ref: https://github.com/stereolabs/zed-gstreamer
    g_object_set(source, "camera-id", app_data->zedStreamProp.cam_id, NULL);
    g_object_set(source, "camera-resolution", 1, NULL);
    g_object_set(source, "camera-fps", app_data->zedStreamProp.cam_fps, NULL);
    g_object_set(source, "stream-type", app_data->zedStreamProp.stream_type, NULL);

    // H264 Setting Ref: https://trac.ffmpeg.org/wiki/Encode/H.264
    // Ref: https://gstreamer.freedesktop.org/documentation/x264/index.html?gi-language=c
    g_object_set(x264e, "tune", app_data->zedStreamProp.x264_tune, NULL);
    g_object_set(x264e, "quantizer", app_data->zedStreamProp.x264_qp, NULL);
    g_object_set(x264e, "speed-preset", app_data->zedStreamProp.x264_preset, NULL);
    g_object_set(x264e, "sliced-threads", 1, NULL);// True

    r_pad= gst_element_get_static_pad(rh264pay, "src");
    GstPadTemplate *templ;
    templ = gst_element_class_get_pad_template(GST_ELEMENT_GET_CLASS(wbin), "sink_%u");
    w_pad = gst_element_request_pad(wbin, templ, NULL, NULL);

    GstPadLinkReturn ret;
    ret = gst_pad_link(r_pad, w_pad);
    g_assert_cmphex (ret, ==, GST_PAD_LINK_OK);
    gst_object_unref(r_pad);
    gst_object_unref(w_pad);
    g_print("Try set end\n");
}

extern gboolean pipeline_init(MyAppData *app_data)
{
    g_print("pipeline init...\n");
    /*Custom_GstData new init*/
    app_data->gstdata = (Custom_GstData*)malloc(sizeof(Custom_GstData));
    memset(app_data->gstdata, 0, sizeof(Custom_GstData));
    g_assert_nonnull(app_data->gstdata);
    Custom_GstData *gstdata = app_data->gstdata;
    GstElement *pipe = NULL;
    GstElement *webrtc = NULL;

    GstBus *bus;
    GError *error = NULL;

    /*pipe*/
    pipe = gst_pipeline_new(NULL);
    g_assert_nonnull(pipe);
    gstdata->pipeline1 = pipe;

    /*bus*/
    guint bus_watch_id;
    bus = gst_element_get_bus(gstdata->pipeline1);
    gst_bus_add_signal_watch(bus);
    g_signal_connect(G_OBJECT(bus), "message::error", (GCallback)error_cb, gstdata);
    g_signal_connect(G_OBJECT(bus), "message::state-changed", (GCallback)state_changed_cb, gstdata);
    g_signal_connect(G_OBJECT(bus), "message::stream-status", (GCallback) stream_status_cb, NULL);

    gst_object_unref(bus);

    /*webrtc*/
    webrtc = gst_element_factory_make("webrtcbin", NULL);
    g_assert_nonnull(webrtc);
    if (app_data->enable_turn_server)
    {
        g_object_set(G_OBJECT(webrtc), "turn-server", app_data->turn_server, NULL);
        g_print("APP: add turn server [%s]\n", app_data->turn_server);
    }
    
    g_object_set(G_OBJECT(webrtc), "stun-server", app_data->stun_server, NULL);
    gstdata->webrtc1 = webrtc;
    gst_bin_add_many(GST_BIN (pipe), webrtc, NULL);

    if(get_remote_is_offerer(app_data))
    {
        // Incoming streams will be exposed via this signal 
        g_signal_connect (webrtc, "pad-added", G_CALLBACK (on_incoming_stream), pipe);
    }
    else
    {
        create_media_bins_and_link(pipe, webrtc, app_data);
    }
    /* This is the gstwebrtc entry point where we create the offer and so on. It
    * will be called when the pipeline goes to PLAYING. */
    my_webrtcbin_init(app_data);

    gst_element_set_state(webrtc, GST_STATE_READY);
    /*webrtc init second*/

    /*pipe ready*/
    gst_element_set_state(pipe, GST_STATE_READY);
    g_print ("pipeline ready\n");

    return TRUE;
}

extern gboolean start_pipeline(Custom_GstData *gstdata, gboolean remote_is_offerer)
{
    GstElement *pipe = gstdata->pipeline1;
    GstElement *webrtc = gstdata->webrtc1;
    GObject *send_channel = NULL;

    /*pipeline*/
    g_print("Starting pipeline\n");
    GstStateChangeReturn ret;
    ret = gst_element_set_state(GST_ELEMENT(pipe), GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE)
    {
        pipeline_error(gstdata);
        return FALSE;
    }
    return TRUE;
}
