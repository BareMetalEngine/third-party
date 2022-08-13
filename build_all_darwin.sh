# !/bin/bash

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
    #pip3 show $1 > /dev/null
    args="import $1"
    python3 -c \"$args\"
    echo python3 -c \"$args\"
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
check_bin pip3
check_bin perl
check_bin ninja
check_bin cmake
check_bin xterm
check_bin zip

# check Python packages
check_pip jinja2

# check if required libraries are installed
#check_lib libharfbuzz-dev
#check_lib libx11-dev
#check_lib libxxf86vm-dev
#check_lib libglu1-mesa-dev
#check_lib freeglut3-dev
#check_lib libcg
#check_lib libcggl

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

# fail on any error in building
set -e
set -o pipefail

# determine if AWS keys are there
export SUBMIT_PARAMS="-upload -awsKey=$AWS_KEY -awsSecret=$AWS_SECRET"

# compile the libs
$ONION library -library=scripts/squish.onion $SUBMIT_PARAMS
exit 0
$ONION library -library=scripts/zlib.onion $SUBMIT_PARAMS
$ONION library -library=scripts/zstd.onion $SUBMIT_PARAMS
$ONION library -library=scripts/lz4.onion $SUBMIT_PARAMS
$ONION library -library=scripts/png.onion $SUBMIT_PARAMS
$ONION library -library=scripts/harfbuzz.onion $SUBMIT_PARAMS
$ONION library -library=scripts/bz2.onion $SUBMIT_PARAMS
$ONION library -library=scripts/brotli.onion $SUBMIT_PARAMS
$ONION library -library=scripts/freetype.onion $SUBMIT_PARAMS
$ONION library -library=scripts/freeimage.onion $SUBMIT_PARAMS
exit 0
$ONION library -library=scripts/squish.onion $SUBMIT_PARAMS
$ONION library -library=scripts/dxc.onion $SUBMIT_PARAMS
$ONION library -library=scripts/mbedtls.onion $SUBMIT_PARAMS
$ONION library -library=scripts/ofbx.onion $SUBMIT_PARAMS
$ONION library -library=scripts/lua.onion $SUBMIT_PARAMS
$ONION library -library=scripts/gtest.onion $SUBMIT_PARAMS
$ONION library -library=scripts/imgui.onion $SUBMIT_PARAMS
$ONION library -library=scripts/embree.onion $SUBMIT_PARAMS
$ONION library -library=scripts/physx.onion $SUBMIT_PARAMS
$ONION library -library=scripts/openal.onion $SUBMIT_PARAMS
$ONION library -library=scripts/llvm.onion $SUBMIT_PARAMS
$ONION library -library=scripts/curl.onion $SUBMIT_PARAMS
$ONION library -library=scripts/bullet.onion $SUBMIT_PARAMS
$ONION library -library=scripts/recast.onion $SUBMIT_PARAMS
