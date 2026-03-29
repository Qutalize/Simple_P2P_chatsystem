FROM golang:1.21-bullseye

RUN apt-get update && apt-get install -y \
    iproute2 \
    iputils-ping \
    tcpdump \
    sudo \
    && rm -rf /var/lib/apt/lists/*

ARG UID=1000
ARG GID=1000
RUN groupadd -g $GID developer && \
    useradd -m -u $UID -g $GID -s /bin/bash developer && \
    passwd -d developer && usermod -L developer && \
    adduser developer sudo && \
    echo '%sudo ALL=(ALL) NOPASSWD:ALL' >> /etc/sudoers

USER developer
WORKDIR /home/developer/source
