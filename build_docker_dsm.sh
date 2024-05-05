#!/bin/bash

./stop_docker_dsm.sh
cmake -B build
cmake --build build -j10
docker image rm dsm:latest
docker build -t dsm:latest .