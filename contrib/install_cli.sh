 #!/usr/bin/env bash

 # Execute this file to install the telestai cli tools into your path on OS X

 CURRENT_LOC="$( cd "$(dirname "$0")" ; pwd -P )"
 LOCATION=${CURRENT_LOC%Telestai-Qt.app*}

 # Ensure that the directory to symlink to exists
 sudo mkdir -p /usr/local/bin

 # Create symlinks to the cli tools
 sudo ln -s ${LOCATION}/Telestai-Qt.app/Contents/MacOS/telestaid /usr/local/bin/telestaid
 sudo ln -s ${LOCATION}/Telestai-Qt.app/Contents/MacOS/telestai-cli /usr/local/bin/telestai-cli
