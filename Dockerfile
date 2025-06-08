FROM ubuntu:25.10

ENV DEBIAN_FRONTEND=noninteractive
ENV SHELL=bash

# Install deps
RUN apt -y update && apt -y upgrade
RUN apt install -y python3 cmake gcc-arm-none-eabi build-essential git

# Get repo in the container

# Copy pico-sdk separately to cache it - this is dockerignored to avoid copying it on the next step
COPY ./lib/pico-sdk /coconut-fsw/lib

COPY . /coconut-fsw --exclude="pico-sdk"
WORKDIR /coconut-fsw

# Update submodules if needed
#RUN git submodule update --init --recursive

# Build repo
ENV PICO_SDK_PATH="/coconut-fsw/lib/pico-sdk"

# Build both simulator and firmware versions
RUN ./deploy.sh -b -g
RUN ./deploy.sh -b -s
