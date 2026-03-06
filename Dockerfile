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
    && rm -rf /var/lib/apt/lists/*


WORKDIR /workspace


# build the sfoundation library here 

RUN curl -LsSf https://astral.sh/install | sh
ENV PATH="/root/.local/bin:$PATH"

RUN git clone https://github.com/pybind/pybind11.git

CMD ["/bin/bash"]