FROM debian:bookworm AS build

RUN set -eux; \
    apt-get update; \
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

ARG MQTTSN_LIBS_VERSION=v2.0.3
ARG URL=https://github.com/commschamp/cc.mqttsn.libs/archive/refs/tags
RUN set -eux; \
    curl -L ${URL}/${MQTTSN_LIBS_VERSION}.tar.gz -o /tmp/mqttsn.tar.gz \
    && mkdir /mqttsn/ \
    && tar -zxf /tmp/mqttsn.tar.gz -C /mqttsn --strip-components=1

ARG BUILD_DIR=/mqttsn/build

RUN set -eux; \
    cd mqttsn/ \
    && mkdir -p ${BUILD_DIR} \
    && BUILD_DIR=${BUILD_DIR} ./script/prepare_externals.sh \
    && cmake . -B ${BUILD_DIR} \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_STANDARD=20 \
      -DCC_MQTTSN_CLIENT_DEFAULT_LIB=OFF \
      -DCC_MQTTSN_WARN_AS_ERR=OFF \
      -DCMAKE_PREFIX_PATH=${BUILD_DIR}/externals/comms/build/install\;${BUILD_DIR}/externals/cc.mqttsn.generated/build/install\;${BUILD_DIR}/externals/cc.mqtt311.generated/build/install \
    && cmake --build ${BUILD_DIR} --target install -- -j4

FROM debian:bookworm-slim
COPY --from=build /usr/local/bin/cc_mqttsn_gateway_app /usr/local/bin/cc_mqttsn_gateway_app
COPY --from=build /usr/local/etc/cc_mqttsn_gateway/cc_mqttsn_gateway.conf.example /usr/local/etc/cc_mqttsn_gateway/cc_mqttsn_gateway.conf
COPY --from=build /lib/x86_64-linux-gnu/libboost* /lib/x86_64-linux-gnu/

ENTRYPOINT ["cc_mqttsn_gateway_app"]
CMD ["-c", "/usr/local/etc/cc_mqttsn_gateway/cc_mqttsn_gateway.conf"]
