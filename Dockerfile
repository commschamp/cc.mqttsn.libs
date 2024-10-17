FROM debian:bookworm AS build

RUN set -eux; \
    apt-get update; \
    apt-get upgrade --yes; \
    apt-get install --yes --no-install-recommends \
      cmake \
      curl \
      gcc \
      g++ \
      git \
      make \
      ca-certificates  \
      libboost-system-dev \
      libboost-program-options-dev \
    ; \
    rm -rf /var/lib/apt/lists/*

ARG CC_CMAKE_VERSION=2024-10-01
ARG URL=https://github.com/commschamp/cc.cmake/archive/refs/tags/
RUN set -eux; \
    curl -L ${URL}/${CC_CMAKE_VERSION}.tar.gz -o /tmp/cc_cmake.tar.gz \
    && mkdir /cc_cmake/ \
    && tar -zxf /tmp/cc_cmake.tar.gz -C /cc_cmake --strip-components=1

COPY config.cmake /cc_cmake/config.cmake

ARG BUILD_DIR=/cc_cmake/build
RUN set -eux; \
    cd /cc_cmake/ \
    && mkdir -p ${BUILD_DIR} \
    && cmake . -B ${BUILD_DIR} -DCMAKE_PROJECT_INCLUDE=config.cmake \
    && cmake --build ${BUILD_DIR} --target cc.mqttsn.libs -- -j4

FROM debian:bookworm-slim
RUN set -eux; \
    apt-get update; \
    apt-get upgrade --yes; \
    apt-get install --yes --no-install-recommends \
      ca-certificates  \
      libboost-system-dev \
      libboost-program-options-dev \
    ; \
    rm -rf /var/lib/apt/lists/*

COPY --from=build /usr/local/bin/cc_mqttsn_gateway_app /usr/local/bin/cc_mqttsn_gateway_app
COPY --from=build /usr/local/etc/cc_mqttsn_gateway/cc_mqttsn_gateway.conf.example /usr/local/etc/cc_mqttsn_gateway/cc_mqttsn_gateway.conf

ENTRYPOINT ["cc_mqttsn_gateway_app"]
CMD ["-c", "/usr/local/etc/cc_mqttsn_gateway/cc_mqttsn_gateway.conf"]
