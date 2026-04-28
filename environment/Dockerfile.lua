FROM docker.io/devkitpro/devkitarm

RUN apt-get update && apt-get install -y \
    jq \
    && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /project/src /project/bin /project/lib /project/third-party

WORKDIR /third-party
RUN wget https://www.lua.org/ftp/lua-5.5.0.tar.gz 
RUN tar -xvzf lua-5.5.0.tar.gz 
RUN cp -r lua-5.5.0/src/* /project/third-party/
RUN rm -f /project/third-party/lua.c /project/third-party/luac.c

WORKDIR /project

COPY Makefile Makefile
COPY default_icon.png default_icon.png

CMD make 