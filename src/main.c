#include "glib-object.h"
#include "gst/gst.h"
#include <jansson.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "pipeline.h"
#include "sockets.h"


int main(int argc, char *argv[]) {
    // basically just wait for sockets lol

    /* Set up gstreamer stuff */
    StreamComponents *components = calloc(1, sizeof(StreamComponents));
    // GstMessage *msg;

    // initialize gstreamer
    gst_init(&argc, &argv);

    // setup_pipeline(&components);
    // add_probes(&components);

    // g_timeout_add(100, debug_timer, components.pipeline);

    /* Set up socket stuff */
    setup_socket(components);

    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    g_main_loop_run(loop);

    /* Free resources */
    gst_element_set_state(components->pipeline, GST_STATE_NULL);
    g_clear_object(&components->pipeline);
    // gst_object_unref(components->pipeline);

    close_socket();
    g_print("Exiting.\n");
    exit(0);
}
