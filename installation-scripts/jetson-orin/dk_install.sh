#!/bin/bash

echo "------------------------------------------------------------------------------------------------------------------------------------"
echo "Start dreamOS installation !!!!!!!!!!!!!"
echo "------------------------------------------------------------------------------------------------------------------------------------"
# Determine the user who ran the command
if [ -n "$SUDO_USER" ]; then
    # Command was run with sudo
    DK_USER=$SUDO_USER
else
    # Command was not run with sudo, fall back to current user
    DK_USER=$USER
fi

# Get the current directory path
CURRENT_DIR=$(pwd)

# Add current user to sudo group of docker
# Check if the docker group exists
if getent group docker > /dev/null 2>&1; then
    echo "Docker: Docker group exists, proceeding..."
else
    echo "Docker: Docker group does not exist. Creating docker group..."
    sudo groupadd docker
fi
# Add the user to the docker group
if sudo usermod -aG docker "$DK_USER"; then
    echo "Docker: User '$DK_USER' has been added to the docker group."
else
    echo "Docker: Failed to add user '$DK_USER' to the docker group."
    exit 1
fi
# Inform the user that they need to log out and back in for the changes to take effect
echo "Docker: Please log out and log back in for the group changes to take effect."

# Detect the system architecture
ARCH_DETECT=$(uname -m)
# Set ARCH variable based on the detected architecture
if [[ "$ARCH_DETECT" == "x86_64" ]]; then
    ARCH="amd64"
elif [[ "$ARCH_DETECT" == "aarch64" ]]; then
    ARCH="arm64"
else
    ARCH="unknown"
fi

# Get XDG_RUNTIME_DIR for the user (not root)
XDG_RUNTIME_DIR=$(sudo -u "$DK_USER" env | grep XDG_RUNTIME_DIR | cut -d= -f2)
# If empty, manually set it
if [ -z "$XDG_RUNTIME_DIR" ]; then
    XDG_RUNTIME_DIR="/run/user/$(id -u "$DK_USER")"
fi
echo "Detected XDG_RUNTIME_DIR: $XDG_RUNTIME_DIR"

# Set Env Variables
HOME_DIR="/home/$DK_USER"
DOCKER_SHARE_PARAM="-v /var/run/docker.sock:/var/run/docker.sock -v /usr/bin/docker:/usr/bin/docker"
DOCKER_AUDIO_PARAM="--device /dev/snd --group-add audio -e PULSE_SERVER=unix:${XDG_RUNTIME_DIR}/pulse/native -v ${XDG_RUNTIME_DIR}/pulse/native:${XDG_RUNTIME_DIR}/pulse/native -v $HOME_DIR/.config/pulse/cookie:/root/.config/pulse/cookie"
LOG_LIMIT_PARAM="--log-opt max-size=10m --log-opt max-file=3"
DOCKER_HUB_NAMESPACE="ghcr.io/samtranbosch"

echo "Env Variables:"
echo "DK_USER: $DK_USER"
echo "ARCH: $ARCH"
echo "HOME_DIR: $HOME_DIR"
echo "DOCKER_SHARE_PARAM: $DOCKER_SHARE_PARAM"
echo "XDG_RUNTIME_DIR: $XDG_RUNTIME_DIR"
echo "DOCKER_AUDIO_PARAM: $DOCKER_AUDIO_PARAM"
echo "LOG_LIMIT_PARAM: $LOG_LIMIT_PARAM"
echo "DOCKER_HUB_NAMESPACE: $DOCKER_HUB_NAMESPACE"

echo "Create dk directoties ..."
mkdir -p /home/$DK_USER/.dk/dk_swupdate /home/$DK_USER/.dk/dk_swupdate/dk_patch /home/$DK_USER/.dk/dk_swupdate/dk_current /home/$DK_USER/.dk/dk_swupdate/dk_current_patch

echo "Create dk_network ..."
docker network create dk_network

echo "------------------------------------------------------------------------------------------------------------------------------------"
echo "------------------------------------------------------------------------------------------------------------------------------------"
echo "Install required utils"
echo "Installing git ..."
# Check if git is available
if command -v git >/dev/null 2>&1; then
    echo "Git is already installed."
else
    echo "Git is not installed. Installing using apt-get..."

    # Update package lists
    apt-get update

    # Install git without prompting for confirmation
    apt-get install -y git

    # Verify installation
    if command -v git >/dev/null 2>&1; then
        echo "Git has been installed successfully."
    else
        echo "There was an error installing Git."
        exit 1
    fi
fi

echo "------------------------------------------------------------------------------------------------------------------------------------"
echo "------------------------------------------------------------------------------------------------------------------------------------"
echo "Install installation repo"
DK_INSTALLATION_DIR="$HOME_DIR/.dk/dk_swupdate"

echo "------------------------------------------------------------------------------------------------------------------------------------"
echo "------------------------------------------------------------------------------------------------------------------------------------"
echo "Install kuksa-client ..."
docker pull ghcr.io/eclipse/kuksa.val/kuksa-client:0.4.2

echo "------------------------------------------------------------------------------------------------------------------------------------"
echo "------------------------------------------------------------------------------------------------------------------------------------"
echo "Install sdv-runtime ..."
docker pull ghcr.io/eclipse-autowrx/sdv-runtime:latest
docker stop sdv-runtime; docker rm sdv-runtime; docker run -d -it --name sdv-runtime --restart unless-stopped -e USER=$DK_USER -e RUNTIME_NAME="DreamKIT_BGSV" -p 55555:55555 -e ARCH=$ARCH ghcr.io/eclipse-autowrx/sdv-runtime:latest

echo "------------------------------------------------------------------------------------------------------------------------------------"
echo "------------------------------------------------------------------------------------------------------------------------------------"
echo "Install dk_manager ..."
docker pull $DOCKER_HUB_NAMESPACE/dk_manager:latest
docker stop dk_manager; docker rm dk_manager; docker run -d -it --name dk_manager $LOG_LIMIT_PARAM $DOCKER_SHARE_PARAM  -v $HOME_DIR/.dk:/app/.dk --restart unless-stopped -e USER=$DK_USER -e DOCKER_HUB_NAMESPACE=$DOCKER_HUB_NAMESPACE -e ARCH=$ARCH $DOCKER_HUB_NAMESPACE/dk_manager:latest

echo "------------------------------------------------------------------------------------------------------------------------------------"
echo "------------------------------------------------------------------------------------------------------------------------------------"
DOCKER_HUB_NAMESPACE="phongbosch"
echo "Install vss_generation ..."
docker pull $DOCKER_HUB_NAMESPACE/dk_vssgeneration_image:vss4.0
docker rm vssgen;docker run -it --name vssgen -v $HOME_DIR/.dk/dk_vssgeneration/:/app/dk_vssgeneration -v $HOME_DIR/.dk/dk_manager/vssmapping/vssmapping_overlay.vspec:/app/.dk/dk_manager/vssmapping/vssmapping_overlay.vspec:ro $LOG_LIMIT_PARAM $DOCKER_HUB_NAMESPACE/dk_vssgeneration_image:vss4.0

DOCKER_HUB_NAMESPACE="ghcr.io/samtranbosch"
echo "------------------------------------------------------------------------------------------------------------------------------------"
echo "------------------------------------------------------------------------------------------------------------------------------------"
echo "Install App/service installation service ... "
docker pull $DOCKER_HUB_NAMESPACE/dk_appinstallservice:latest

echo "------------------------------------------------------------------------------------------------------------------------------------"
echo "------------------------------------------------------------------------------------------------------------------------------------"
# Install dk_ivi
dk_ivi_value=""
# Loop through all input arguments
for arg in "$@"; do
    # Check if the argument starts with dk_ivi=
    if [[ "$arg" == dk_ivi=* ]]; then
        # Extract the value after the equal sign
        dk_ivi_value="${arg#*=}"
    fi
done
if [[ "$dk_ivi_value" == "true" ]]; then
    echo "enable xhost local"
	$CURRENT_DIR/scripts/dk_enable_xhost.sh
    echo "Instal dk_ivi ..."
    docker pull $DOCKER_HUB_NAMESPACE/dk_ivi:latest

    echo "Checking for NVIDIA Target Board..."
    if [ -f "/etc/nv_tegra_release" ]; then
        echo "NVIDIA Jetson board detected."
        docker stop dk_ivi; docker rm dk_ivi ; docker run -d -it --name dk_ivi --network host -v /tmp/.X11-unix:/tmp/.X11-unix -e DISPLAY=$DISPLAY -e XDG_RUNTIME_DIR=$XDG_RUNTIME_DIR -e QT_QUICK_BACKEND=software --restart unless-stopped $LOG_LIMIT_PARAM $DOCKER_SHARE_PARAM -v $HOME_DIR/.dk:/app/.dk -e DKCODE=dreamKIT -e DK_USER=$DK_USER -e DK_DOCKER_HUB_NAMESPACE=$DOCKER_HUB_NAMESPACE -e DK_ARCH=$ARCH -e DK_CONTAINER_ROOT="/app/.dk/" $DOCKER_HUB_NAMESPACE/dk_ivi:latest
    else
        echo "Not NVIDIA board."
        docker stop dk_ivi; docker rm dk_ivi ; docker run -d -it --name dk_ivi --network host -v /tmp/.X11-unix:/tmp/.X11-unix -e DISPLAY=$DISPLAY -e XDG_RUNTIME_DIR=$XDG_RUNTIME_DIR --device /dev/dri:/dev/dri --restart unless-stopped $LOG_LIMIT_PARAM $DOCKER_SHARE_PARAM -v $HOME_DIR/.dk:/app/.dk -e DKCODE=dreamKIT -e DK_USER=$DK_USER -e DK_DOCKER_HUB_NAMESPACE=$DOCKER_HUB_NAMESPACE -e DK_ARCH=$ARCH -e DK_CONTAINER_ROOT="/app/.dk/" $DOCKER_HUB_NAMESPACE/dk_ivi:latest
    fi
else
    echo "To Install dk_ivi, run './dk_install dk_ivi=true'"
fi


echo "------------------------------------------------------------------------------------------------------------------------------------"
echo "------------------------------------------------------------------------------------------------------------------------------------"
echo "Store environment variables"

# Define the output file (you can change the path as needed)
mkdir -p /home/.dk/dk_swupdate
DK_ENV_FILE="/home/.dk/dk_swupdate/dk_swupdate_env.sh"
> $DK_ENV_FILE

# Write the actual values of the variables to the output file
cat <<EOF > "${DK_ENV_FILE}"
#!/bin/bash

DK_USER="${DK_USER}"
ARCH="${ARCH}"
HOME_DIR="${HOME_DIR}"
DOCKER_SHARE_PARAM="${DOCKER_SHARE_PARAM}"
XDG_RUNTIME_DIR="${XDG_RUNTIME_DIR}"
DOCKER_AUDIO_PARAM="${DOCKER_AUDIO_PARAM}"
LOG_LIMIT_PARAM="${LOG_LIMIT_PARAM}"
DOCKER_HUB_NAMESPACE="${DOCKER_HUB_NAMESPACE}"
dk_ivi_value="${dk_ivi_value}"
EOF

# make the output file executable
chmod +x "${DK_ENV_FILE}"
cp $DK_ENV_FILE "${HOME_DIR}/.dk/dk_swupdate/dk_swupdate_env.sh"
chmod +x "$CURRENT_DIR/scripts/dk_kuksa_client.sh"
cp $CURRENT_DIR/scripts/dk_kuksa_client.sh /home/.dk/dk_swupdate/
chmod +x "$CURRENT_DIR/scripts/dk_xiphost.sh"
cp $CURRENT_DIR/scripts/dk_xiphost.sh /home/.dk/dk_swupdate/
$CURRENT_DIR/scripts/create_dk_xiphost_service.sh

echo "Environment variables with actual values have been saved to ${DK_ENV_FILE}"

echo "------------------------------------------------------------------------------------------------------------------------------------"
echo "------------------------------------------------------------------------------------------------------------------------------------"
echo "Create sw history file for the first time"

DK_SWHISTORY_FILE="/home/$DK_USER/.dk/dk_swupdate/dk_swhistory.json"

> $DK_SWHISTORY_FILE

# Get current UTC timestamp in ISO 8601 format (e.g. 2023-10-01T10:15:30Z)
timestamp=$(date -u +"%Y-%m-%dT%H:%M:%SZ")

# Create dk_swhistory.json with the JSON content and the actual timestamp
cat <<EOF > $DK_SWHISTORY_FILE
{
  "MetaData": {
      "timestamp": "${timestamp}",
      "description": "Initial software.",
      "currentVersion": "0.0.0",
      "patch": ""
  },
  "SwUpdateHistory": [
    {
      "id": 1,
      "timestamp": "${timestamp}",
      "description": "Initial software.",
      "version": "0.0.0",
      "patch": ""
    }
  ],
  "LastFailure": {
      "timestamp": "",
      "description": "",
      "version": "",
      "patch": ""
  }
}
EOF

echo "$DK_SWHISTORY_FILE created with timestamp ${timestamp}"

echo "------------------------------------------------------------------------------------------------------------------------------------"
echo "------------------------------------------------------------------------------------------------------------------------------------"
echo "Remove Only Dangling Images (No Tags)"
docker image prune -f

echo "------------------------------------------------------------------------------------------------------------------------------------"
echo "------------------------------------------------------------------------------------------------------------------------------------"

echo "------------------------------------------------------------------------------------------------------------------------------------"
echo "Please reboot your system for dreamSW to take effect !!!!!!!!!!!!!"
echo "------------------------------------------------------------------------------------------------------------------------------------"
