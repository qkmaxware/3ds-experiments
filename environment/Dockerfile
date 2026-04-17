FROM docker.io/devkitpro/devkitarm

RUN apt-get update && apt-get install -y \
    jq \
    && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /project/src /project/bin /project/lib
WORKDIR /project

COPY Makefile Makefile
COPY default_icon.png default_icon.png

CMD make 