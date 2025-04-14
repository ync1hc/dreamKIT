# dk_service_can_provider

> Information: 
This docker image provides a service to communicate with CAN bus based on CAN signal in .dbc file and vss singal.  

## Presequisites
dreamOS must be installed first. Refer to this repo (https://github.com/ppa2hc/dk_installation) for dreamOS installation.  

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
docker kill 671b621fcc2b5c69999ebc50;docker rm 671b621fcc2b5c69999ebc50;docker run -d -it --name 671b621fcc2b5c69999ebc50 --log-opt max-size=10m --log-opt max-file=3 -v /home/$USER/.dk/dk_installedservices/671b621fcc2b5c69999ebc50:/app/runtime --network host  -v /home/$USER/.dk/dk_manager/vssmapping/dbc_default_values.json:/app/vss/dbc_default_values.json:ro  -v /home/$USER/.dk/dk_vssgeneration/vss.json:/app/vss/vss.json:ro xxx/dk_service_can_provider:latest
```