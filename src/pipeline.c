
#include "glib-object.h"
#include "gst/gstpipeline.h"
#include "gst/gstutils.h"
#include <stdio.h>
#include <stdlib.h>

#include "pipeline.h"

static void pad_added_handler(GstElement * src, GstPad * new_pad, StreamComponents * data) {
    StreamComponents *components = (StreamComponents *)data;

    GstCaps *caps = gst_pad_query_caps(new_pad, NULL);

    GstStructure *str = gst_caps_get_structure(caps, 0);

    const gchar *new_pad_type = gst_structure_get_name(str);

    GstPad *sink_pad = NULL;

    // TODO maybe also check the suffix to make sure our codecs are good as well
    if (g_str_has_prefix(new_pad_type, "video/")) {
        sink_pad = gst_element_get_static_pad(components->video_queue, "sink");
    } else if (g_str_has_prefix(new_pad_type, "audio/")) {
        sink_pad = gst_element_get_static_pad(components->audio_queue, "sink");
    } else {
        g_print("Invalid pad type %s\n", new_pad_type);
        goto pad_handler_exit;
    }

    // if already linked, we're done
    if (gst_pad_is_linked (sink_pad)) {
        g_print ("We are already linked. Ignoring.\n");
        goto pad_handler_exit;
    }

    GstPadLinkReturn ret = gst_pad_link(new_pad, sink_pad);
    if (GST_PAD_LINK_FAILED(ret)) {
        g_print ("Type is '%s' but link failed.\n", new_pad_type);
    }

pad_handler_exit:
    /* Unreference the new pad's caps, if we got them */
    if (caps != NULL)
        gst_caps_unref (caps);

    /* Unreference the sink pad */
    if (sink_pad)
        gst_object_unref (sink_pad);
}

static gboolean bus_callback(GstBus *bus, GstMessage *msg, gpointer data) {
    StreamComponents *components = (StreamComponents*) data;

    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_EOS:
            g_print("EOS\n");
            break;

        case GST_MESSAGE_ERROR:
            GError *err = NULL;
            gchar *debug_info = NULL;

            gst_message_parse_error(
                msg,
                &err,
                &debug_info
            );

            g_printerr(
                "Error from %s: %s\n",
                GST_OBJECT_NAME(msg->src),
                err->message
            );

            g_printerr(
                "Debug info: %s\n",
                debug_info ? debug_info : "none"
            );

            g_clear_error(&err);
            g_free(debug_info);

            break;

        case GST_MESSAGE_STREAM_START:
            // seek to 0 with segment mode so we're always playing in segment mode
            gboolean ok = gst_element_seek_simple(
                components->pipeline,
                GST_FORMAT_TIME,
                GST_SEEK_FLAG_KEY_UNIT | GST_SEEK_FLAG_SNAP_BEFORE | GST_SEEK_FLAG_SEGMENT,
                0
            );

            break;

        case GST_MESSAGE_SEGMENT_DONE:
            GstStateChangeReturn ret = gst_element_set_state(components->pipeline, GST_STATE_PAUSED);
            break;
    }
    
    return TRUE;
}

static void check_components(StreamComponents *components) {
    if (!components->pipeline)           { g_printerr("Failed to create component: pipeline\n");         exit(EXIT_FAILURE); }
    if (!components->source)             { g_printerr("Failed to create component: source\n");           exit(EXIT_FAILURE); }
    if (!components->demux)              { g_printerr("Failed to create component: demux\n");            exit(EXIT_FAILURE); }
    if (!components->video_queue)        { g_printerr("Failed to create component: video_queue\n");      exit(EXIT_FAILURE); }
    if (!components->video_identity)     { g_printerr("Failed to create component: video_identity\n");   exit(EXIT_FAILURE); }
    if (!components->video_caps_in)      { g_printerr("Failed to create component: video_caps_in\n");    exit(EXIT_FAILURE); }
    if (!components->video_encoder)      { g_printerr("Failed to create component: video_encoder\n");    exit(EXIT_FAILURE); }
    if (!components->video_caps_out)     { g_printerr("Failed to create component: video_caps_out\n");   exit(EXIT_FAILURE); }
    if (!components->video_parse)        { g_printerr("Failed to create component: video_parse\n");      exit(EXIT_FAILURE); }
    if (!components->audio_queue)        { g_printerr("Failed to create component: audio_queue\n");      exit(EXIT_FAILURE); }
    if (!components->audio_identity)     { g_printerr("Failed to create component: audio_identity\n");   exit(EXIT_FAILURE); }
    if (!components->audio_encoder)      { g_printerr("Failed to create component: audio_encoder\n");    exit(EXIT_FAILURE); }
    if (!components->audio_parse)        { g_printerr("Failed to create component: audio_parse\n");      exit(EXIT_FAILURE); }
    if (!components->whip_sink)          { g_printerr("Failed to create component: whip_sink\n");        exit(EXIT_FAILURE); }
}

int build_pipeline(StreamComponents *components, char *filename, char *server_addr) {
    // tear down the old pipeline
    g_clear_object(&components->pipeline);
    memset(components, 0, sizeof(*components));

    // TODO choose the demux and video parser based on the file
    // e.g. go with av1parse or mkvdemux

    // build the source, sink, filters
    components->source           = gst_element_factory_make("filesrc", "source");
    components->demux            = gst_element_factory_make("qtdemux", "demux");                 // TODO change based on if file is mkv or mp4
    components->video_queue      = gst_element_factory_make("queue2", "video_queue");
    components->video_identity   = gst_element_factory_make("identity", "video_identity");
    components->video_caps_in    = gst_element_factory_make("capsfilter", "video_caps_in");
    components->video_encoder    = gst_element_factory_make("x264enc", "video_encoder");         // TODO allow for different codecs
    components->video_caps_out   = gst_element_factory_make("capsfilter", "video_caps_out");
    components->video_parse      = gst_element_factory_make("h264parse", "video_parse");
    components->audio_queue      = gst_element_factory_make("queue2", "audio_queue");
    components->audio_identity   = gst_element_factory_make("identity", "audio_identity");
    components->audio_encoder    = gst_element_factory_make("opusenc", "audio_encoder");         // TODO allow for different codecs
    components->audio_parse      = gst_element_factory_make("opusparse", "audio_parse");
    components->whip_sink        = gst_element_factory_make("whipclientsink", "sink");

    components->pipeline = gst_pipeline_new("main-pipeline");

    check_components(components);

    // build the pipeline
    gst_bin_add_many(
        GST_BIN(components->pipeline), 
        components->source,
        components->demux,
        components->video_queue,
        components->video_identity,
        components->video_caps_in,
        components->video_encoder,
        components->video_caps_out,
        components->video_parse,
        components->audio_queue,
        components->audio_identity,
        components->audio_encoder,
        components->audio_parse,
        components->whip_sink,
        NULL
    );

    // link relevant elements, not pads 
    gboolean try_link;

    try_link = gst_element_link(components->source,
                    components->demux);

    if (!try_link) {
        g_printerr ("Source and demux could not be linked.\n");
        gst_object_unref (components->pipeline);
        return -1;
    }

    try_link = gst_element_link_many(
        components->video_queue,
        components->video_identity,
        components->video_caps_in,
        components->video_encoder,
        components->video_caps_out,
        components->video_parse,
        components->whip_sink,
        NULL);

    if (!try_link) {
        g_printerr ("Video components could not be linked.\n");
        gst_object_unref (components->pipeline);
        return -1;
    }

    try_link = gst_element_link_many(
        components->audio_queue,
        components->audio_identity,
        components->audio_encoder,
        components->audio_parse,
        components->whip_sink,
        NULL);

    if (!try_link) {
        g_printerr ("Audio components could not be linked.\n");
        gst_object_unref (components->pipeline);
        return -1;
    }

    // set the filename
    g_object_set(components->source, "location", filename, NULL);

    // configure queues for livestreaming
    g_object_set(
        components->video_queue,
        "max-size-buffers", 0,
        "max-size-bytes", 0,
        "max-size-time", 500 * GST_MSECOND,
        // "leaky", 2,  // apparently queue2 doesn't have a leaky property
        NULL
    );
    g_object_set(
        components->audio_queue,
        "max-size-buffers", 0,
        "max-size-bytes", 0,
        "max-size-time", 500 * GST_MSECOND,
        NULL
    );

    // configure identity filters
    g_object_set(
        components->video_identity,
        "sync", TRUE,
        "signal-handoffs", FALSE,
        // "single-segment", TRUE,
        NULL
    );
    g_object_set(
        components->audio_identity,
        "sync", TRUE,
        "signal-handoffs", FALSE,
        // "single-segment", TRUE,
        NULL
    );

    // configure video and audio encoders
    // x264 options
    g_object_set(
        components->video_encoder,
        "speed-preset", "slow",
        "pass", "cbr",
        "bitrate", 10000,       // TODO get this from options
        "bframes", 0,
        "key-int-max", 24,
        "nal-hrd", 2,           // do we need this?
        "option-string", "vbv-maxrate=10000:vbv-bufsize=5000",
        NULL
    );
    g_object_set(
        components->audio_encoder,
        NULL
    );

    // configure video caps filters
    GstCaps *caps_in = gst_caps_from_string(
        "video/x-raw,format=I420"
    );
    g_object_set(
        components->video_caps_in,
        "caps", caps_in,
        NULL);
    gst_caps_unref(caps_in);

    GstCaps *caps_out = gst_caps_from_string(
        "video/x-h264, profile=baseline, level=(string)4.1"
    );
    g_object_set(
        components->video_caps_out,
        "caps", caps_out,
        NULL
    );
    gst_caps_unref(caps_out);

    // inject SPS/PPS
    g_object_set(
        components->video_parse,
        "config-interval", 1,
        NULL
    );

    // add async handling to whipclientsink
    gst_util_set_object_arg(
        G_OBJECT(components->whip_sink),
        "async-handling",
        "true"
    );

    // fix bitrates for whip sink
    // TODO make it possible to set this in docker somehow
    g_object_set(components->whip_sink,
        "max-bitrate", 12000000,
        NULL); // 12 Mbps ceiling

    g_object_set(components->whip_sink,
        "start-bitrate", 10000000,
        NULL); // start near actual rate
    
    // set pipeline latency
    g_object_set(
        components->pipeline,
        "latency",
        50 * GST_MSECOND,
        NULL
    );

    // set the whip endpoint
    GObject *signaller = NULL;
    g_object_get(
        components->whip_sink,
        "signaller",
        &signaller,
        NULL
    );

    g_object_set(
        signaller,
        "whip-endpoint",
        server_addr,
        NULL
    );

    g_object_unref(signaller);

    // set congestion control
    g_object_set(components->whip_sink, "congestion-control", 0, NULL);

    /* Connect to the pad-added signal */
    g_signal_connect(components->demux, "pad-added", G_CALLBACK (pad_added_handler), components);

    // add bus watch to watch for signals
    GstBus *bus = gst_element_get_bus(components->pipeline);
    gst_bus_add_watch(bus, bus_callback, components);

    return 0;
}
