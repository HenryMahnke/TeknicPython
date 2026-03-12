FROM ghcr.io/astral-sh/uv:latest AS uv
FROM ubuntu:24.04 

RUN apt-get update && apt-get install -y \
    ca-certificates \
    build-essential \
    curl \
    git \
    python3 \
    python3-pip \
    python3-dev \
    clang \
    clangd \
    g++ \
    tar \
    && rm -rf /var/lib/apt/lists/*


COPY --from=uv /uv /usr/local/bin/uv
COPY --from=uv /uvx /usr/local/bin/uvx

WORKDIR /workspace

WORKDIR /workspace/linux_sdk/sFoundation


WORKDIR /workspace


RUN uv --version

# build the sfoundation library here 


RUN git clone https://github.com/pybind/pybind11.git

CMD ["/bin/bash"]