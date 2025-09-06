FROM ubuntu:24.04
# EOL  31 May 2029

ENV DEBIAN_FRONTEND=noninteractive
ENV SHELL=bash

RUN apt-get update && apt-get install -y \
    git=1:2.43.0-1ubuntu7 \
    python3-dev \
    gcc-arm-none-eabi=15:13.2.rel1-2 \
    socat=1.8.0.0-4build3 \
    cmake=3.28.3-1build7 \
    build-essential=12.10ubuntu1 \
    && rm -rf /var/lib/apt/lists/*

# Python 3.12 - EOL October 2028

# Prevent package upgrades by holding specific versions
RUN apt-mark hold \
    git \
    python3 \
    gcc-arm-none-eabi \
    socat \
    cmake \
    build-essential

RUN git clone --recurse-submodules https://github.com/ASU-SDSL/coconut-fsw

WORKDIR /coconut-fsw

# Default command
CMD ["/bin/bash"]
