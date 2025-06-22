FROM ubuntu:25.10

ENV DEBIAN_FRONTEND=noninteractive
ENV SHELL=bash

# Install deps
RUN apt -y update && apt -y upgrade
RUN apt install -y python3 cmake gcc-arm-none-eabi build-essential git

# Set the workdir, this will be here once the coconut-fsw is made a volume at run
WORKDIR /coconut-fsw

ENV PICO_SDK_PATH="/coconut-fsw/lib/pico-sdk"
