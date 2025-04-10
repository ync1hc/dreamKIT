# ----------------------------------------------------------------------------------
# Stage 1: Build gRPC from source and install into /usr/local
# ----------------------------------------------------------------------------------

FROM ubuntu:20.04 AS grpc-builder

# Disable interactive prompts during package installation.
ENV DEBIAN_FRONTEND=noninteractive

# Install essential packages for building gRPC.
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    pkg-config \
    libssl-dev

# Clone gRPC along with its submodules. Adjust branch/tag as needed.
RUN git clone --recurse-submodules -b v1.44.0 https://github.com/grpc/grpc.git /grpc

WORKDIR /grpc
RUN mkdir -p cmake/build && cd cmake/build && \
    cmake -DgRPC_INSTALL=ON -DgRPC_BUILD_TESTS=OFF ../.. && \
    make -j$(nproc) && \
    make install

# ----------------------------------------------------------------------------------
# Stage 2: Build the Application
# ----------------------------------------------------------------------------------

FROM ubuntu:20.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

# Install build tools and dependencies.
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    pkg-config \
    libprotobuf-dev \
    protobuf-compiler \
    libssl-dev \
    nlohmann-json3-dev

# Copy gRPC installation from previous stage.
COPY --from=grpc-builder /usr/local/ /usr/local/

# Set the LD_LIBRARY_PATH so that runtime can find the installed libraries.
ENV LD_LIBRARY_PATH=/usr/local/lib

# Set working directory for your project.
WORKDIR /app

# Copy all project files (including your proto folder, CMakeLists.txt, and source) into the container.
COPY . /app

# Create and move into a build directory, then configure and build the project.
# We pass the gRPC_DIR so that CMake can find the gRPC installation.
RUN mkdir -p build && cd build && \
    cmake -DgRPC_DIR=/usr/local/lib/cmake/grpc .. && \
    make -j$(nproc)

# ----------------------------------------------------------------------------------
# Stage 3: Create a minimal runtime container with the built binary.
# ----------------------------------------------------------------------------------

FROM ubuntu:20.04 AS runtime

ENV DEBIAN_FRONTEND=noninteractive

# Install runtime libraries (adjust versions as needed).
RUN apt-get update && apt-get install -y \
    libprotobuf17 \
    ca-certificates && \
    rm -rf /var/lib/apt/lists/*

# Set LD_LIBRARY_PATH for runtime.
ENV LD_LIBRARY_PATH=/usr/local/lib

# Copy installed libraries (if needed) from the builder stage.
COPY --from=builder /usr/local/ /usr/local/

# Copy the built executable from the builder stage.
# Adjust the built executable name as necessary (here assumed to be "KuksaDatabrokerClient").
COPY --from=builder /app/build/KuksaDatabrokerClient /usr/local/bin/KuksaDatabrokerClient
COPY --from=builder /app/config.json /usr/local/bin/config.json

# (Optional) Verify that grpc_cpp_plugin exists.
RUN ls -l /usr/local/bin/grpc_cpp_plugin

# Expose any necessary port (optional)
# EXPOSE 55555

# Set the entrypoint to run your client.
ENTRYPOINT ["/usr/local/bin/KuksaDatabrokerClient"]
