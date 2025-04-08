# First stage: Build dependencies in a full Python environment
FROM ubuntu:24.04 AS app-builder
#FROM debian:bookworm AS app-builder

# Define the build argument for the target architecture
#ARG TARGETARCH

WORKDIR /app/

# Install necessary packages for building the environment
RUN apt-get update && apt install -y git cmake build-essential libssl-dev libboost-all-dev curl qt6-base-dev qt6-base-private-dev qt6-declarative-dev qt6-declarative-private-dev libqt6quick6 qml6-module-qtquick qml6-module-qtquick-controls qml6-module-qtquick-layouts qml6-module-qtquick-window qml6-module-qtqml-workerscript qml6-module-qtquick-templates pax-utils

#COPY copy-app-lddtree.sh /app/copy-app-lddtree.sh
COPY src /app/src

# Install Python dependencies into a specific directory
RUN cd /app/src \
    && rm -rf build \   
    && mkdir build \
    && cd build \
    && cmake .. \
    && make -j4
    #&& chmod +x /app/copy-app-lddtree.sh \
    #&& /app/copy-app-lddtree.sh 

# Second stage: Create a minimal runtime environment
FROM ubuntu:24.04 AS target
#FROM debian:bookworm AS target

# Define the build argument for the target architecture
ARG TARGETARCH

#RUN apt-get update && apt install -y python3-minimal libgl1 libglx-mesa0 libopengl0
RUN apt-get update && apt install -y python3-minimal
RUN apt install -y libglx-mesa0
RUN apt install -y fontconfig
RUN apt install -y libpng-dev
#RUN apt install -y libpython3.11 

# Install necessary dependencies
RUN apt-get update \
    && apt-get install -y wget gnupg2 software-properties-common apt-transport-https \
    && wget -qO- https://packages.microsoft.com/keys/microsoft.asc | gpg --dearmor > packages.microsoft.gpg \
    && install -o root -g root -m 644 packages.microsoft.gpg /usr/share/keyrings/ \
    && sh -c 'echo "deb [arch=$(dpkg --print-architecture) signed-by=/usr/share/keyrings/packages.microsoft.gpg] https://packages.microsoft.com/repos/code stable main" > /etc/apt/sources.list.d/vscode.list' \
    && apt-get update \
    && apt-get install -y code \
    && rm -f packages.microsoft.gpg \
    && apt-get clean


WORKDIR /app

# Copy the Python packages from the builder stage to the Alpine image
COPY --from=app-builder /app/src/build/dk_ivi /app/exec/
COPY ./target/${TARGETARCH}/lib /app/exec/lib

# Copy application files
COPY start.sh /app/

# Set execute permission for the script
RUN chmod +x /app/start.sh

# Set environment variables
ENV LD_LIBRARY_PATH=/app/exec

# Execute the script
CMD ["/app/start.sh"]
