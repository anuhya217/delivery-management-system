FROM ubuntu:22.04

RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y \
    build-essential \
    cmake \
    libmysqlclient-dev \
    libasio-dev \
    git \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY . .

RUN g++ -std=c++17 \
    api/server.cpp \
    -o server \
    -I/usr/include \
    -lmysqlclient \
    -lpthread

EXPOSE 18080

CMD ["./server"]