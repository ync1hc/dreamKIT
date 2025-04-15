# dk_service_can_provider

> Information: 
This docker image provides a service to communicate with CAN bus based on CAN signal in .dbc file and vss singal.  

## Presequisites
dreamOS must be installed first. Refer to this script (\installation-scripts\jetson-orin\dk_install.sh) for dreamOS installation.  


# For localy Dev-Test

## Presequisites
Fix potential error with sh file
```shell
# Fix error at sh files
sed -i -e 's/\r$//' *.sh
chmod +x *.sh
```

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
# Check the list of docker
root@s32g274ardb2:~# docker ps -a
CONTAINER ID   IMAGE                                        COMMAND                  CREATED          STATUS                     PORTS                                                                                                                                     NAMES
acf0daebb4fc   dk_service_can_provider:arm64                "/app/start.sh"          28 minutes ago   Up 24 minutes                                                                                                                                                        dk_service_can_provider
```


## Ubuntu
### [ Ubuntu ] Run the vehicle service

```shell
#
# Having the design with your vss signal list
Example with dbc_overlay.vspec (at /mapping/vss_4.0_custom/ folder)
- vss: Vehicle.Body.Lights.Beam.Low.IsOn
- vss2dbc: enabled - to receive the request from playgroud
- --> transform/mapping: is required
- dbc2vss: disable - to send the actual state back to playgroud

Generate the vss_dbc.json file
Refer to
- https://github.com/eclipse-kuksa/kuksa-can-provider
- https://github.com/COVESA/vss-tools 

Update the dbc_default_values.json file for the default value of new added vss signals
Example with
"DAS_headlightRequest" : 0,

```

```shell
#
# Create the Virtual CAN
./createvcan.sh vcan0
# Observe the CAN network
candump vcan0 &

# Execute Natively
python3 ./dbcfeeder.py --val2dbc --dbc2val --use-socketcan --mapping mapping/vss_4.0_custom/vss_dbc.json --canport vcan0
# The console with output
2025-04-15 09:48:38,167 INFO dbcfeeder: Reading configuration from file: config/dbc_feeder.ini
2025-04-15 09:48:38,168 INFO dbcfeeder: DBC2VAL mode is: True
2025-04-15 09:48:38,168 INFO dbcfeeder: VAL2DBC mode is: True
2025-04-15 09:48:38,169 INFO dbcfeeder: Path to token information not given
2025-04-15 09:48:38,169 INFO dbcfeeder: Starting CAN feeder
2025-04-15 09:48:38,169 INFO dbcfeederlib.dbcparser: Reading definitions from bus description file Model3CAN.dbc
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


### [ Ubuntu > Jetson Orin ] Run the kuksa-client
```shell
#
# Testing with executing the kuksa-client python version
kuksa-client

# the console output with
sdv-orin@ubuntu:~$ kuksa-client

     ⢀⣤⣶⣾⣿⢸⣿⣿⣷⣶⣤⡀
    ⣴⣿⡿⠋⣿⣿   ⠈⠙⢿⣿⣦
   ⣾⣿⠋  ⣿⣿  ⣶⣿  ⠙⣿⣷
  ⣸⣿⠇   ⣿⣿⠠⣾⡿⠃   ⠸⣿⣇  ⣶ ⣠⡶⠂ ⣶  ⢰⡆ ⢰⡆⢀⣴⠖ ⢠⡶⠶⠶⡦   ⣰⣶⡀
  ⣿⣿    ⠿⢿⣷⣦⡀     ⣿⣿  ⣿⢾⣏   ⣿  ⢸⡇ ⢸⡷⣿⡁  ⠘⠷⠶⠶⣦  ⢠⡟⠘⣷
  ⢹⣿⡆   ⣿⣶⠈⢻⣿⡆   ⢰⣿⡏  ⠿ ⠙⠷⠄ ⠙⠷⠶⠟⠁ ⠸⠇⠈⠻⠦ ⠐⠷⠶⠶⠟ ⠠⠿⠁ ⠹⠧
   ⢿⣿⣄  ⣿⣿  ⠿⣿  ⣠⣿⡿
    ⠻⣿⣷⡄⣿⣿   ⢀⣠⣾⣿⠟    kuksa-client CLI
     ⠈⠛⠇⢿⣿⣿⣿⣿⡿⠿⠛⠁     0.4.2

Default tokens directory: /home/sdv-orin/.local/lib/python3.8/site-packages/kuksa_client/kuksa_server_certificates/jwt

Connecting to VSS server at 127.0.0.1 port 55555 using KUKSA GRPC protocol.
TLS will not be used.
INFO 2025-04-15 09:49:07,844 kuksa_client.grpc No Root CA present, it will not be possible to use a secure connection!
INFO 2025-04-15 09:49:07,844 kuksa_client.grpc.aio Establishing insecure channel
gRPC channel connected.
Test Client> 


# Request the LowBeam state
Test Client> setTargetValue Vehicle.Body.Lights.Beam.Low.IsOn true
OK
Test Client> setTargetValue Vehicle.Body.Lights.Beam.Low.IsOn false
OK

# To exit the kuksa-client
Test Client> exit
gRPC channel disconnected.


# Expected with receiving CAN message
# LowBeam ~ On
vcan0  3E9   [8]  00 00 00 00 00 00 00 00
# LowBeam ~ Off
vcan0  3E9   [8]  01 00 00 00 00 00 00 00
```


## S32G

Prepare the docker image (arm46) in Ubuntu and deploy to S32G
```shell
# Build for arm64
docker buildx build --platform linux/arm64 -t dk_service_can_provider:arm64 --load .
# Save docker image
docker save -o dk_service_can_provider.tar dk_service_can_provider:arm64
# Copy to S32G
scp dk_service_can_provider.tar root@192.168.56.49:~/sdvdemo/docker
```

Load and run the docker container
```shell
cd ~/sdvdemo/docker
# Load the docker image
docker load -i dk_service_can_provider.tar
# Run the docker
docker stop dk_service_can_provider; docker rm dk_service_can_provider
docker run --restart=always -d --name dk_service_can_provider --net=host -e LOG_LEVEL=INFO dk_service_can_provider:arm64

# Remove (optional)
docker image rm -f dk_service_can_provider:arm64
```

Test the CAN connection
```shell
cd ~/sdvdemo/tool
# LowBeam ~ On
./cansend can1 3E9#0000000000000000
# LowBeam ~ Off
./cansend can1 3E9#0100000000000000
```



# For MarketPlace (TBD)

## build docker image
local-arch build  
```
docker build -t dk_service_can_provider:latest --file Dockerfile .
```

multi-arch build and push to docker hub  
```
docker buildx create --name dk_service_can_provider_multiarch_build --use
docker buildx build --platform linux/amd64,linux/arm64 -t xxx/dk_service_can_provider:latest --push .
```

## run local built docker container
```
docker kill 671b621fcc2b5c69999ebc50;docker rm 671b621fcc2b5c69999ebc50;docker run -d -it --name 671b621fcc2b5c69999ebc50 --log-opt max-size=10m --log-opt max-file=3 -v /home/$USER/.dk/dk_installedservices/671b621fcc2b5c69999ebc50:/app/runtime --network host  -v /home/$USER/.dk/dk_manager/vssmapping/dbc_default_values.json:/app/vss/dbc_default_values.json:ro  -v /home/$USER/.dk/dk_vssgeneration/vss.json:/app/vss/vss.json:ro dk_service_can_provider:latest
```

## run docker container from docker hub
```
docker pull xxx/dk_service_can_provider:latest
docker kill 671b621fcc2b5c69999ebc50;docker rm 671b621fcc2b5c69999ebc50
docker run -d -it --name 671b621fcc2b5c69999ebc50 --log-opt max-size=10m --log-opt max-file=3 -v /home/$USER/.dk/dk_installedservices/671b621fcc2b5c69999ebc50:/app/runtime --network host  -v /home/$USER/.dk/dk_manager/vssmapping/dbc_default_values.json:/app/vss/dbc_default_values.json:ro  -v /home/$USER/.dk/dk_vssgeneration/vss.json:/app/vss/vss.json:ro xxx/dk_service_can_provider:latest
```
