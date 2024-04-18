FROM ubuntu:20.04 as builder

RUN apt-get update && apt-get install -y \
    cmake \
    g++ \
    make

WORKDIR /app

COPY CMakeLists.txt /app/
COPY src/ /app/src/
COPY public/ /app/public/

RUN mkdir -p /app/build && \
    cd /app/build && \
    cmake .. && \
    make

FROM ubuntu:20.04

WORKDIR /app

COPY --from=builder /app/build/WebServer /app/WebServer
COPY --from=builder /app/public/ /app/public/

EXPOSE 80

CMD ["./WebServer"]


