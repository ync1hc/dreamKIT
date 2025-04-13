# Overview


# Presequisites

## Environment
-   Workspace: /home/developer/workspace/
-   Ip address for Host VM - S32G

```shell
# NetworkManager Restart
sudo systemctl restart NetworkManager

# Verify the Server Is Running and Listening on Port 55555
netstat -tuln | grep 55555
or
lsof -i :55555

# Fix error at sh files
sed -i -e 's/\r$//' *.sh
chmod +x *.sh
```

```shell
sudo ip link add name br0 type bridge
sudo ip link set dev br0 up
sudo ip link set dev enp0s8 master br0
# Orin
sudo ifconfig br0 192.168.56.48
sudo ip link set br0 address 5e:5e:c9:a7:22:55

# S32G
sudo ifconfig br0 192.168.56.49

pass: dev12345
```


```shell
# One time required
docker network create kuksa
# Execute the kuksa-databroker (v0.4.4)
docker run -dit --rm --name Server --network kuksa ghcr.io/eclipse-kuksa/kuksa-databroker:0.4.4 --insecure

```

# Development

## Locally with docker container

```shell
# Build - linux/arm64 (local-arch build)
docker build -t kuksa-databroker-client .

# Execute
docker run --rm --network kuksa kuksa-databroker-client:amd64
# Debug
docker run -it --entrypoint /bin/bash --network kuksa kuksa-databroker-client
```

```shell
docker buildx build --platform linux/arm64 -t kuksa-databroker-client:arm64 --load .
docker buildx build --platform linux/arm64/v8 -t kuksa-databroker-client:amd64_x8 --load .
docker buildx build --platform linux/amd64 -t kuksa-databroker-client:amd64 --load .

# docker cp <containerId>:/path/to/file/in/container /path/to/destination/on/host
docker cp kuksa-databroker-client:arm64://usr/local/bin/libKuksaClient.so $pwd/lib/arm64
docker cp kuksa-databroker-client:amd64_x8://usr/local/bin/libKuksaClient.so $pwd/lib/amd64_x8
docker cp kuksa-databroker-client:amd64://usr/local/bin/libKuksaClient.so $pwd/lib/amd64

```

## Publish with docker hub
```shell
# Specific - multi-arch build and push to docker hub
# - Enable Docker Buildx (if not already enabled) > Run the docker container
docker buildx create --use

# - Build - Establish connection to that docker container
docker buildx build --platform linux/amd64,linux/amd64,linux/arm64/v8 -t samtranbosch/kuksa-databroker-client:latest --push .

```

```shell
docker stop kuksa-databroker-client
docker rm kuksa-databroker-client
docker run -d -it --name kuksa-databroker-client --network host --restart unless-stopped --log-opt max-size=10m --log-opt max-file=3 samtranbosch/kuksa-databroker-client:latest

```


# Testing


```shell
# in a new terminal
docker run -it --rm --network kuksa ghcr.io/eclipse-kuksa/kuksa-databroker-cli:0.4.4 --server Server:55555
```

The CLI provides an interactive prompt which can be used to send commands to the Databroker (databroker.v1, not databroker.v2).
```shell
  ⠀⠀⠀⢀⣤⣶⣾⣿⢸⣿⣿⣷⣶⣤⡀
  ⠀⠀⣴⣿⡿⠋⣿⣿⠀⠀⠀⠈⠙⢿⣿⣦
  ⠀⣾⣿⠋⠀⠀⣿⣿⠀⠀⣶⣿⠀⠀⠙⣿⣷
  ⣸⣿⠇⠀⠀⠀⣿⣿⠠⣾⡿⠃⠀⠀⠀⠸⣿⣇⠀⠀⣶⠀⣠⡶⠂⠀⣶⠀⠀⢰⡆⠀⢰⡆⢀⣴⠖⠀⢠⡶⠶⠶⡦⠀⠀⠀⣰⣶⡀
  ⣿⣿⠀⠀⠀⠀⠿⢿⣷⣦⡀⠀⠀⠀⠀⠀⣿⣿⠀⠀⣿⢾⣏⠀⠀⠀⣿⠀⠀⢸⡇⠀⢸⡷⣿⡁⠀⠀⠘⠷⠶⠶⣦⠀⠀⢠⡟⠘⣷
  ⢹⣿⡆⠀⠀⠀⣿⣶⠈⢻⣿⡆⠀⠀⠀⢰⣿⡏⠀⠀⠿⠀⠙⠷⠄⠀⠙⠷⠶⠟⠁⠀⠸⠇⠈⠻⠦⠀⠐⠷⠶⠶⠟⠀⠠⠿⠁⠀⠹⠧
  ⠀⢿⣿⣄⠀⠀⣿⣿⠀⠀⠿⣿⠀⠀⣠⣿⡿
  ⠀⠀⠻⣿⣷⡄⣿⣿⠀⠀⠀⢀⣠⣾⣿⠟    databroker-cli
  ⠀⠀⠀⠈⠛⠇⢿⣿⣿⣿⣿⡿⠿⠛⠁     v0.4.1

Successfully connected to http://Server:55555/
sdv.databroker.v1 >
```

Get the vehicle's current speed
```shell
get Vehicle.Speed
```
Set the vehicle's current/target speed
```shell
#
feed Vehicle.Speed 18
#
set Vehicle.Speed 20
```

Others VSS (by default, the application will subscribe to all "signal" mentioned in config.json)
```shell
#
feed Vehicle.Body.Lights.Beam.Low.IsOn true
#
set Vehicle.Body.Lights.Beam.Low.IsOn true
```


# Script

## build_and_copy.sh
Explanation

1. Platforms Array:
You define a list of platforms (e.g. linux/arm64 and linux/amd64) you want to build for. You can add or remove elements as needed. The script extracts the architecture portion (after the slash) to tag the image and name the output directory.

2. Building the Image:
The command
docker buildx build --platform "${platform}" -t "${image_tag}" --load .
builds the image for the given platform and loads it into your local Docker images with an appropriate tag.

3. Extracting the .so File:
The script creates a temporary container using docker create from the built image (without actually running it). It then uses docker cp to copy the /usr/local/bin/libKuksaClient.so file from the container to a host folder corresponding to the target architecture ($PWD/lib/arm64 or $PWD/lib/amd64).

4. Cleanup:
After copying the file, the temporary container is removed using docker rm.

This script automates the multi-platform build process as well as the extraction of your specific shared library into a host directory.

