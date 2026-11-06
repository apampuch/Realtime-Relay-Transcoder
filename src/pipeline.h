#ifndef PIPELINE_H
#define PIPELINE_H

#include "gst/gstelement.h"
typedef struct _StreamComponents {
    GstElement *pipeline, *source, *demux;
    GstElement *video_queue;
    GstElement *video_identity;
    GstElement *video_caps_in;
    GstElement *video_encoder;
    GstElement *video_caps_out;
    GstElement *video_parse;
    GstElement *audio_queue;
    GstElement *audio_identity;
    GstElement *audio_encoder;
    GstElement *audio_caps_out;
    GstElement *audio_parse;
    GstElement *whip_sink;
} StreamComponents;

int build_pipeline(StreamComponents *components, char *filename, char *server_addr);

#endif
