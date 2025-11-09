FROM ubuntu:latest

EXPOSE 42000/udp

RUN apt-get update -y
RUN apt-get install -y --no-install-recommends ca-certificates git build-essential cmake

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
RUN (cd build/test/src/ && ctest)
