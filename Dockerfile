FROM ubuntu:24.04 AS builder
WORKDIR /coursework
RUN apt-get update && apt-get install -y g++ cmake git libssl-dev libasio-dev && rm -rf /var/lib/apt/lists/*
COPY CMakeLists.txt main.cpp ./
RUN cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j$(nproc)
FROM ubuntu:24.04 AS runtime
WORKDIR /coursework
RUN apt-get update && apt-get install -y libstdc++6 ca-certificates curl && rm -rf /var/lib/apt/lists/*
COPY --from=builder /coursework/build/Coursework_PC /coursework/Coursework_PC
EXPOSE 8080
CMD ["/coursework/Coursework_PC"]