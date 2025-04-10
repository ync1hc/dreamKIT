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
docker run -it --rm --name Server --network kuksa ghcr.io/eclipse-kuksa/kuksa-databroker:0.4.4 --insecure

```

# Development

## Locally with docker container

```shell
# Build - linux/arm64 (local-arch build)
docker build -t kuksa-databroker-client .

# Execute
docker run --rm --network kuksa kuksa-databroker-client

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
