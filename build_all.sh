# !/bin/bash
set -e 
set -o pipefail

# exe checker
function check_bin() {
    if ! hash $1; then
    echo "$1 is not installed" >&2
        exit 1
    else
        echo "$1 is installed"
    fi
}

# python lib checker
function check_pip() {
    pip show $1 > /dev/null
    if [ $? -ne 0 ]
    then
        echo "$1 is not installed" >&2
        exit 1
    else
        echo "$1 is installed"
    fi
}

# package checker
function check_lib() {
    dpkg -s $1 > /dev/null
    if [ $? -ne 0 ]
    then
        echo "$1 is not installed" >&2
        exit 1
    else
        echo "$1 is installed"
    fi
}

###############################################

# check if python is installed
if ! hash python3; then
    echo "Python is not installed"
    exit 1
else    
    # check Python version
    ver=$(python3 -V 2>&1 | sed 's/.* \([0-9]\).\([0-9]\).*/\1\2/')
    if [ "$ver" -lt "30" ]; then
        echo "This script requires python 3.0 or greater"
        exit 1
    else
        echo "Python is installed"
    fi    
fi

# check if required stuff in installed
check_bin clang
check_bin pip
check_bin perl
check_bin ninja
check_bin cmake
check_bin xterm
check_bin zip

# check Python packages
check_pip jinja2

# check if required libraries are installed
check_lib libharfbuzz-dev
check_lib libx11-dev
check_lib libxxf86vm-dev
check_lib libglu1-mesa-dev
check_lib freeglut3-dev
check_lib libcg
check_lib libcggl

# clone the onion
export ONION=../onion_tool/bin/onion
if [ ! -f $ONION ]; then

    if [ ! -d .onion ]; then
        git clone https://github.com/BareMetalEngine/onion .onion
    else
        pushd .onion
        git pull 
        popd
    fi

    ONION=./.onion/onion

    if [ ! -f $ONION ]; then
        echo "Unable to determine location of Onion tool"
        exit 1
    fi
fi

# compile the libs
#$ONION library -library=scripts/zlib.onion
#$ONION library -library=scripts/lz4.onion
#$ONION library -library=scripts/freetype.onion
#$ONION library -library=scripts/freeimage.onion
#$ONION library -library=scripts/squish.onion
#$ONION library -library=scripts/dxc.onion
#$ONION library -library=scripts/mbedtls.onion
#$ONION library -library=scripts/ofbx.onion
#$ONION library -library=scripts/lua.onion
#$ONION library -library=scripts/gtest.onion
#$ONION library -library=scripts/imgui.onion
#$ONION library -library=scripts/embree.onion
#$ONION library -library=scripts/physx.onion
#$ONION library -library=scripts/openal.onion
#$ONION library -library=scripts/llvm.onion
#$ONION library -library=scripts/curl.onion
#$ONION library -library=scripts/bullet.onion
#$ONION library -library=scripts/recast.onion