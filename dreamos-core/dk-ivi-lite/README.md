# dk_app_python_template

> Info: This repo provides a guideline to build image for dk_manager app. dk_manager is the core app to manage the operation of dreamkit system.

## Prerequisites:
Install dk_manager

## Build docker image
Local-arch build:  
```
docker build -t dk_ivi:latest --file Dockerfile .
```
Run local:  
```
docker stop dk_ivi; docker rm dk_ivi ; docker run -d -it --name dk_ivi -v /tmp/.X11-unix:/tmp/.X11-unix -e DISPLAY=:0 --device /dev/dri:/dev/dri --restart unless-stopped dk_ivi:latest
```

Multi-arch build and push to docker hub:  
```
docker buildx create --name dk_ivi_multiarch_build --use
docker buildx build --platform linux/amd64,linux/arm64 -t boschvn/dk_ivi:latest --push -f Dockerfile .
```

Run from docker hub
```
docker pull boschvn/dk_ivi:latest
docker stop dk_ivi; docker rm dk_ivi ; docker run -d -it --name dk_ivi -v /tmp/.X11-unix:/tmp/.X11-unix -e DISPLAY=:0 --device /dev/dri:/dev/dri --restart unless-stopped boschvn/dk_ivi:latest
```

## Notes:
This image is tested well on Raspi5 and Micorsoft Surface devices.  
In some systems like Nvidia Ubuntu, by default they don't have /dev/dri. Nvidia system might use different way to render display with hardward acceleration.  
Below is in testing mode using nvidia-docker with option "--gpus all". if it doesn't work by default (like in Jetson Nano), the option "-e QT_QUICK_BACKEND=software" should be used.  
Example on Jetson nano:  
```
nvidia-docker run -d -it --name dk_ivi -v /tmp/.X11-unix:/tmp/.X11-unix -e DISPLAY=:0 --gpus all -e QT_QUICK_BACKEND=software --device /dev/dri:/dev/dri boschvn/dk_ivi:latest
```
Example on Jetson Orin AGX:  can run the ivi but there is a flickering issue on the display.  
```
nvidia-docker run -d -it --name dk_ivi -v /tmp/.X11-unix:/tmp/.X11-unix -e DISPLAY=:0 --gpus all --device /dev/dri:/dev/dri boschvn/dk_ivi:latest
```

```
// export to run natively 
export DKCODE="dreamKIT"
export DK_USER=$SUDO_USER
export DK_DOCKER_HUB_NAMESPACE="boschvn"
export DK_ARCH="amd64"
export DK_CONTAINER_ROOT=~/.dk/

// export to run with docker container
export DKCODE="dreamKIT"
export DK_USER=$SUDO_USER
export DK_DOCKER_HUB_NAMESPACE="boschvn"
export DK_ARCH="amd64"
export DK_CONTAINER_ROOT="/app/.dk/"
docker stop dk_ivi; docker rm dk_ivi ; docker run -d -it --name dk_ivi -v /tmp/.X11-unix:/tmp/.X11-unix -e DISPLAY=:0 --device /dev/dri:/dev/dri --restart unless-stopped -v ~/.dk:/app/.dk -e DKCODE=dreamKIT -e DK_USER=$DK_USER -e DK_DOCKER_HUB_NAMESPACE=$DK_DOCKER_HUB_NAMESPACE -e DK_ARCH=$DK_ARCH -v /var/run/docker.sock:/var/run/docker.sock -v /usr/bin/docker:/usr/bin/docker -e DK_CONTAINER_ROOT=$DK_CONTAINER_ROOT dk_ivi:latest
```

