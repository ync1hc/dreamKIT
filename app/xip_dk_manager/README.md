# dk_app_python_template

> Info: This repo provides a guideline to build image for dk_manager app. dk_manager is the core app to manage the operation of dreamkit system.

## Prerequisites:
Git clone dk-manger repo into src folder before run docker build.  
```
cd src
git clone https://github.com/ppa2hc/dk-manager.git
```

## Build docker image
Local-arch build:  
```
docker build -t dk_manager:latest --file Dockerfile .
```
Run local:  
```
docker stop dk_manager; docker rm dk_manager; docker run -d -it --name dk_manager -v /var/run/docker.sock:/var/run/docker.sock -v /usr/bin/docker:/usr/bin/docker  dk_manager:latest
```

Multi-arch build and push to docker hub:  
```
docker buildx create --name dk_multiarch_build_dk_manager --use
docker buildx use dk_multiarch_build_dk_manager
docker buildx build --platform linux/amd64,linux/arm64 -t phongbosch/dk_manager:latest --push -f Dockerfile .
```

Use Dockerfile.Fast to build faster. This uses prebuilt socketio client.  
```
docker buildx build --platform linux/amd64,linux/arm64 -t phongbosch/dk_manager:latest --push -f Dockerfile.Fast .
```

Run from docker hub
```
docker pull phongbosch/dk_manager:latest
docker stop dk_manager; docker rm dk_manager; docker run -d -it --name dk_manager -v /var/run/docker.sock:/var/run/docker.sock -v /usr/bin/docker:/usr/bin/docker  phongbosch/dk_manager:latest
```
