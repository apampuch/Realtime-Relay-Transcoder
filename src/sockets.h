#ifndef SOCKET_HANDLERS_H
#define SOCKET_HANDLERS_H

#include "glib.h"
#include "glibconfig.h"
#include "pipeline.h"

void setup_socket(StreamComponents *components);
void close_socket();

#endif
