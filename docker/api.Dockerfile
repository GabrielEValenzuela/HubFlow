FROM ubuntu:24.04 AS build

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        clang cmake ninja-build git ca-certificates pkg-config \
        libmysqlclient-dev libsodium-dev libssl-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY CMakeLists.txt CMakePresets.json ./
COPY cmake ./cmake
COPY src ./src
COPY tests ./tests
COPY db ./db

RUN cmake --preset server \
    && cmake --build --preset server \
    && ctest --preset server -E Integration --output-on-failure \
    && cmake --install build/server --prefix /opt/hubflow \
    && install -m 0755 build/server/tests/hubflow_integration_tests /opt/hubflow/bin/hubflow_integration_tests

FROM ubuntu:24.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        libmysqlclient21 libsodium23 libssl3t64 ca-certificates curl \
    && rm -rf /var/lib/apt/lists/* \
    && id ubuntu \
    && mkdir -p /app/exports \
    && chown ubuntu:ubuntu /app/exports

WORKDIR /app
COPY --from=build /opt/hubflow/bin/ /usr/local/bin/
COPY web ./web
COPY db/migrations ./db/migrations

ENV HUBFLOW_PORT=8080 \
    HUBFLOW_WEB_ROOT=/app/web \
    HUBFLOW_EXPORT_DIR=/app/exports

USER ubuntu
EXPOSE 8080
HEALTHCHECK --interval=10s --timeout=3s --start-period=20s --retries=5 \
    CMD curl -fsS http://127.0.0.1:8080/api/v1/health || exit 1
CMD ["hubflow"]
