#ifndef RCVMEDIA_H
#define RCVMEDIA_H

#include <gst/gst.h>

extern void on_incoming_stream(GstElement *webrtc, GstPad *pad, GstElement *pipe);

#endif
