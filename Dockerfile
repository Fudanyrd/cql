# syntax=docker/dockerfile:1

FROM ubuntu:latest
WORKDIR /home/yrd
RUN apt-get update 
RUN apt-get -y install cmake 
RUN apt-get -y install g++

COPY . .
RUN mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Debug .. && make cql
RUN chmod +x /home/yrd/build/cql
CMD ["/home/yrd/build/cql"]
# run:
# ```
# docker build --tag cql .
# ```