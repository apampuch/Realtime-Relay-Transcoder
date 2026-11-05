CFLAGS := $(shell pkg-config --cflags gstreamer-1.0)
LIBS := $(shell pkg-config --libs gstreamer-1.0 jansson)

all:
	gcc src/main.c src/sockets.c src/pipeline.c -o ./bin/rr-transcoder $(CFLAGS) $(LIBS)
