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
docker run -d -it --name dk_service_can_provider --net=host -e LOG_LEVEL=INFO -e CAN_PORT=vcan0 dk_service_can_provider
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
setTargetValue Vehicle.Body.Lights.Hazard.IsSignaling true/false
setTargetValue Vehicle.Body.Lights.IsBrakeOn true/false
setTargetValue Vehicle.Cabin.Seat.Row1.Pos1.Position {0-3}
setTargetValue Vehicle.Cabin.HVAC.Station.Row1.Right.FanSpeed {0-100}
setTargetValue Vehicle.Cabin.HVAC.Station.Row1.Left.FanSpeed {0-100}

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

### Prepare the docker image (arm46)
```shell
# Build for arm64
docker buildx build --platform linux/arm64 -t dk_service_can_provider:arm64 --load .
# Save docker image
docker save -o dk_service_can_provider.tar dk_service_can_provider:arm64
# Copy to S32G
scp dk_service_can_provider.tar root@192.168.56.49:~/sdvdemo/docker
```

### Deploy and run
```shell
cd ~/sdvdemo/docker
# Load the docker image
docker load -i dk_service_can_provider.tar
# Run the docker
docker stop dk_service_can_provider; docker rm dk_service_can_provider
docker run --restart unless-stopped -d --name dk_service_can_provider --net=host -e LOG_LEVEL=INFO  -e CAN_PORT=can1 dk_service_can_provider:arm64

# Remove (optional)
docker image rm -f dk_service_can_provider:arm64
```

### Check CAN Bus

```shell
#
#Test the CAN connection
cd ~/sdvdemo/tool
# LowBeam ~ On
./cansend can1 3E9#0000000000000000
# LowBeam ~ Off
./cansend can1 3E9#0100000000000000
```




