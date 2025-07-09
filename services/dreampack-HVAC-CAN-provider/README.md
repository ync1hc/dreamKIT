# dk_service_can_provider

> **Overview**  
> This Docker image provides a service to communicate with CAN bus based on CAN signals defined in `.dbc` files and VSS signals.



## Table of Contents

- [dk\_service\_can\_provider](#dk_service_can_provider)
  - [Table of Contents](#table-of-contents)
  - [Prerequisites](#prerequisites)
  - [Local Development and Testing](#local-development-and-testing)
    - [Fix Shell Script Line Endings and Permissions](#fix-shell-script-line-endings-and-permissions)
    - [Network Setup (Ubuntu VM \<-\> Jetson Orin \<-\> S32G)](#network-setup-ubuntu-vm---jetson-orin---s32g)
    - [Local - Build and Test](#local---build-and-test)
  - [S32G - Build and Test](#s32g---build-and-test)
    - [Prepare the docker image (arm46)](#prepare-the-docker-image-arm46)
    - [Deploy and run](#deploy-and-run)
    - [Check CAN Bus](#check-can-bus)



## Prerequisites

- **dreamOS** must be installed prior to using this service.  
  Refer to the installation script:  
  `installation-scripts/jetson-orin/dk_install.sh`



## Local Development and Testing

### Fix Shell Script Line Endings and Permissions

Fix potential error with sh file
```shell
# Fix error at sh files
sed -i -e 's/\r$//' *.sh
chmod +x *.sh
```

### Network Setup (Ubuntu VM <-> Jetson Orin <-> S32G)

Having the Ubuntu VM connects to LAN network of Jetson Orin (addr:192.168.56.48) <> S32G (addr:192.168.56.49)
```shell
#
# Ubuntu VM
sudo ip link add name br0 type bridge
sudo ip link set dev br0 up
sudo ip link set dev enp0s8 master br0
sudo ifconfig br0 192.168.56.3
sudo ip link set br0 address 5e:5e:c9:a7:22:55

pass: dev12345

#
# Connect to Jetson Orin
ssh sdv-orin@192.168.56.48
pass: 123456
# Check the list of docker
sdv-orin@ubuntu:~$ docker ps -a
CONTAINER ID   IMAGE                                          COMMAND                  CREATED          STATUS                       PORTS                                                     NAMES
1a34ceaeef0b   boschvn/sdv-runtime:latest                     "/start_services.sh"     17 hours ago     Up 15 hours                  3090/tcp, 0.0.0.0:55555->55555/tcp, :::55555->55555/tcp   sdv-runtime
218d7048ec82   phongbosch/dk_manager:latest                   "/app/start.sh"          3 days ago       Up 15 hours                                                                            dk_manager
d5d708ec0fd5   phongbosch/dk_ivi:latest                       "/app/start.sh"          3 days ago       Up 15 hours                                                                            dk_ivi

#
# Connect to S32G
ssh root@192.168.56.49
# Connect to Pi
ssh phong@192.168.56.49
#
# Check the list of docker
root@s32g274ardb2:~# docker ps -a
CONTAINER ID   IMAGE                                        COMMAND                  CREATED          STATUS                     PORTS                                                                                                                                     NAMES
acf0daebb4fc   dk_service_can_provider:arm64                "/app/start.sh"          28 minutes ago   Up 24 minutes                                                                                                                                                        dk_service_can_provider
```

### Local - Build and Test

Create the Virtual CAN
```shell
./prepare-dbc-file/createvcan.sh vcan0
# Observe the CAN network
candump vcan0 &
```

Build & run Docker image
```shell
#
# Build
docker build -t dk_service_can_provider:latest --file Dockerfile .
# 
# Run the docker
docker stop dk_service_can_provider; docker rm dk_service_can_provider
docker run -d -it --name dk_service_can_provider --net=host -e LOG_LEVEL=INFO -e CAN_PORT=vcan0 -e KUKSA_ADDRESS=localhost dk_service_can_provider
```

Debug
```shell
# The console with output
2025-04-15 09:48:38,167 INFO dbcfeeder: Reading configuration from file: config/dbc_feeder.ini
2025-04-15 09:48:38,168 INFO dbcfeeder: DBC2VAL mode is: True
2025-04-15 09:48:38,168 INFO dbcfeeder: VAL2DBC mode is: True
2025-04-15 09:48:38,169 INFO dbcfeeder: Path to token information not given
2025-04-15 09:48:38,169 INFO dbcfeeder: Starting CAN feeder
2025-04-15 09:48:38,169 INFO dbcfeederlib.dbcparser: Reading definitions from bus description file ModelCAN.dbc
...
2025-04-15 09:48:38,546 INFO dbcfeederlib.databrokerclientwrapper: Vehicle.Body.Lights.Beam.Low.IsOn is already registered with type BOOLEAN
...
2025-04-15 09:48:38,507 INFO dbcfeederlib.databrokerclientwrapper: Connectivity as string: ChannelConnectivity.READY
2025-04-15 09:48:38,507 INFO dbcfeederlib.databrokerclientwrapper: Connected to data broker
2025-04-15 09:48:38,517 INFO can.interfaces.socketcan.socketcan: Created a socket
2025-04-15 09:48:38,517 INFO dbcfeederlib.dbcreader: Starting to receive CAN messages fom bus
2025-04-15 09:48:38,518 INFO dbcfeeder: Check that datapoints are registered
2025-04-15 09:48:38,518 INFO dbcfeeder: Starting thread for processing VSS Data Entry changes, writing to CAN device vcan0
2025-04-15 09:48:38,520 INFO can.interfaces.socketcan.socketcan: Created a socket
2025-04-15 09:48:38,523 INFO dbcfeederlib.databrokerclientwrapper: Subscribe entry: SubscribeEntry(path='Vehicle.Body.Lights.Beam.Low.IsOn', view=<View.FIELDS: 10>, fields=[<Field.ACTUATOR_TARGET: 3>])
...
2025-04-15 09:48:38,672 INFO dbcfeeder: Starting to process CAN signals
```

Test
```shell
#
# Testing with executing the kuksa-client python version
kuksa-client grpc://127.0.0.1:55555
# or
kuksa-client grpc://192.168.56.48:55555

#
# Supported API
setTargetValue Vehicle.Body.Lights.Beam.Low.IsOn true/false
setTargetValue Vehicle.Body.Lights.Beam.High.IsOn true/false
setTargetValue Vehicle.Body.Lights.Hazard.IsSignaling true/false
setTargetValue Vehicle.Cabin.Seat.Row1.DriverSide.Position {0-3}
setTargetValue Vehicle.Cabin.HVAC.Station.Row1.Passenger.FanSpeed {0-100}
setTargetValue Vehicle.Cabin.HVAC.Station.Row1.Driver.FanSpeed {0-100}

#
# Request the LowBeam state
Test Client> setTargetValue Vehicle.Body.Lights.Beam.Low.IsOn true
OK
Test Client> setTargetValue Vehicle.Body.Lights.Beam.Low.IsOn false
OK
#
# Expected with receiving CAN message
# LowBeam ~ On
vcan0  3E9   [8]  00 00 00 00 00 00 00 00
# LowBeam ~ Off
vcan0  3E9   [8]  01 00 00 00 00 00 00 00
```


## S32G - Build and Test

### Prepare the docker image (arm64)
```shell
# Locally build for arm64
docker buildx create --name dk_service_can_provider_build --use
docker buildx use dk_service_can_provider_build
docker buildx build --platform linux/arm64 -t dk_service_can_provider:arm64 --load .
```

### Deploy and run
```shell
# Interact with Orin - Update the Docker Local registry
docker tag dk_service_can_provider:latest 192.168.56.48:5000/dk_service_can_provider:arm64
docker push 192.168.56.48:5000/dk_service_can_provider:arm64

# Interact with S32G - pull the docker images from local host at Orin
sshpass -p '' ssh -o StrictHostKeyChecking=no root@192.168.56.49 'docker pull 192.168.56.48:5000/dk_service_can_provider:arm64 ; mkdir -p ~/.dk/dk_installedservices'
sshpass -p '' ssh -o StrictHostKeyChecking=no root@192.168.56.49 'docker image prune -f'
# Interact with S32G - run the docker container
sshpass -p '' ssh -o StrictHostKeyChecking=no root@192.168.56.49 'docker kill dk_service_can_provider;docker rm dk_service_can_provider'
sshpass -p '' ssh -o StrictHostKeyChecking=no root@192.168.56.49 'docker run -d -it --name dk_service_can_provider --log-opt max-size=10m --log-opt max-file=3 --network host --privileged 192.168.56.48:5000/dk_service_can_provider:arm64'
# Interact with S32G - check the running
sshpass -p '' ssh -o StrictHostKeyChecking=no root@192.168.56.49 'docker ps'
sshpass -p '' ssh -o StrictHostKeyChecking=no root@192.168.56.49 'docker logs dk_service_can_provider'
```

### Check CAN Bus

```shell
#
# Test the CAN connection
# LowBeam ~ On
cansend can1 3E9#0000000000000000
# LowBeam ~ Off
cansend can1 3E9#0100000000000000
```

# Sonar Parking

## Deployment
```shell
# Jetson Orin/ Ubuntu Machine
docker save -o dk_service_can_provider.tar dk_service_can_provider:latest
sshpass -p '' ssh -o StrictHostKeyChecking=no root@192.168.56.49 'mkdir -p /home/root/.dk/nxp-s32g/docker/'
scp dk_service_can_provider.tar root@192.168.56.49:/home/root/.dk/nxp-s32g/docker/dk_service_can_provider.tar

# Option 1: with SSH
ssh root@192.168.56.49
cd /home/root/.dk/nxp-s32g/docker/
docker load -i dk_service_can_provider:latest
docker kill dk_service_can_provider;docker rm dk_service_can_provider
docker run -d -it --name dk_service_can_provider --restart unless-stopped --log-opt max-size=10m --log-opt max-file=3 --network host -e CAN_PORT=can0 --privileged dk_service_can_provider:latest


# Option 2: If you don't want to have the ssh than replacing 'docker ps' by your command
sshpass -p '' ssh -o StrictHostKeyChecking=no root@192.168.56.49 'docker load -i /home/root/.dk/nxp-s32g/docker/dk_service_can_provider.tar'
sshpass -p '' ssh -o StrictHostKeyChecking=no root@192.168.56.49 'docker kill dk_service_can_provider;docker rm dk_service_can_provider'
sshpass -p '' ssh -o StrictHostKeyChecking=no root@192.168.56.49 'docker run -d -it --name dk_service_can_provider --restart unless-stopped --log-opt max-size=10m --log-opt max-file=3 --network host -e CAN_PORT=can0 --privileged dk_service_can_provider:latest'
sshpass -p '' ssh -o StrictHostKeyChecking=no root@192.168.56.49 'docker ps'
sshpass -p '' ssh -o StrictHostKeyChecking=no root@192.168.56.49 'docker logs dk_service_can_provider'
```

## Testing
### Via CAN -> Kuksa Client

```shell
# 
# Kuksa Client
#
# Testing with executing the kuksa-client python version
kuksa-client grpc://127.0.0.1:55555
# or
kuksa-client grpc://192.168.56.48:55555

# Subsribe
subscribe Vehicle.ADAS.ObstacleDetectionFront.Left.Distance
subscribe Vehicle.ADAS.ObstacleDetectionFront.CornerLeft.Distance
subscribe Vehicle.ADAS.ObstacleDetectionFront.Center.Distance
subscribe Vehicle.ADAS.ObstacleDetectionFront.CenterRight.Distance
subscribe Vehicle.ADAS.ObstacleDetectionFront.CornerRight.Distance
subscribe Vehicle.ADAS.ObstacleDetectionFront.Right.Distance
subscribe Vehicle.ADAS.ObstacleDetectionRear.Right.Distance
subscribe Vehicle.ADAS.ObstacleDetectionRear.CornerRight.Distance
subscribe Vehicle.ADAS.ObstacleDetectionRear.CenterRight.Distance
subscribe Vehicle.ADAS.ObstacleDetectionRear.Center.Distance
subscribe Vehicle.ADAS.ObstacleDetectionRear.CornerLeft.Distance
subscribe Vehicle.ADAS.ObstacleDetectionRear.Left.Distance
```

```shell
# Then obervation on the update whener send message on CAN bus
cansend vcan0 331#000007d200188002
```

### Via Kuksa Client -> IVI

```shell
#
# Testing with executing the kuksa-client python version
kuksa-client grpc://127.0.0.1:55555
# or
kuksa-client grpc://192.168.56.48:55555

# Report current status from Sensor
setValue Vehicle.ADAS.ObstacleDetectionFront.Left.Distance {}

# 
# Observation on IVI for the update
```

