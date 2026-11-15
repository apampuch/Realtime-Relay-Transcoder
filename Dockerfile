FROM ubuntu:24.04
WORKDIR /app

RUN mkdir /sockets

RUN apt-get update && apt-get install -y \
gstreamer1.0-tools \
gstreamer1.0-plugins-base \
gstreamer1.0-plugins-good \
gstreamer1.0-plugins-bad \
gstreamer1.0-libav \
gstreamer1.0-nice \
gstreamer1.0-plugins-ugly \
libjansson-dev \
nodejs npm curl \
&& rm -rf /var/lib/apt/lists/*

COPY ./libs/            /app/libs/
COPY ./bin/             /app/bin/
COPY ./bridge/          /app/bridge/
COPY ./html-interface/  /app/html-interface/
COPY entrypoint.sh      /app/

ENV GST_PLUGIN_PATH="/app/libs/"

# RUN gst-inspect-1.0 whipclientsink

CMD ["./entrypoint.sh"]
