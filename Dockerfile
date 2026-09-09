FROM ubuntu:latest

ENV DEBIAN_FRONTEND=noninteractive

EXPOSE 42000/udp

RUN apt-get update -y
RUN apt-get install -y --no-install-recommends \
    build-essential \
    ca-certificates \
    cmake \
    gdb \
    git

COPY ./include /Wink/include
COPY ./samples /Wink/samples
COPY ./src /Wink/src
COPY ./test /Wink/test
COPY ./CMakeLists.txt /Wink/CMakeLists.txt
COPY ./LICENSE /Wink/LICENSE
COPY ./README.md /Wink/README.md

WORKDIR /Wink

RUN cmake -S . -B build
RUN cmake --build build
RUN ctest --test-dir build --output-on-failure
RUN cmake --install build
