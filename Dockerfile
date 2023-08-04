FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive
ENV SHELL=bash

# Install deps
RUN apt -y update && apt -y upgrade
RUN apt install -y python3 cmake gcc-arm-none-eabi build-essential git

# Get repo in the container
COPY . /coconut-fsw
WORKDIR /coconut-fsw

# Update submodules if needed
RUN git submodule update --init --recursive

# Build repo
ENV PICO_SDK_PATH="/coconut-fsw/lib/pico-sdk"
RUN ./build.sh