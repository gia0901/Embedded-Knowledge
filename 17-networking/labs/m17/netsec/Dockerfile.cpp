# Tầng 1: build với đủ công cụ; tầng 2: chỉ binary + libssl, chạy non-root
FROM ubuntu:24.04 AS build
RUN apt-get update && apt-get install -y --no-install-recommends g++ cmake make libssl-dev nlohmann-json3-dev \
 && rm -rf /var/lib/apt/lists/*
WORKDIR /src
COPY CMakeLists.txt ./
COPY include include
COPY src src
COPY apps apps
COPY tests tests
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build -j --target orders_api

FROM ubuntu:24.04
RUN apt-get update && apt-get install -y --no-install-recommends libssl3t64 && rm -rf /var/lib/apt/lists/*
COPY --from=build /src/build/orders_api /usr/local/bin/orders_api
USER 10001
ENTRYPOINT ["/usr/local/bin/orders_api"]
