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
check_bin python
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
check_lib libbrotli-dev
check_lib libzstd-dev

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
#echo -awsKey=$AWS_KEY -awsSecret=$AWS_SECRET
#$ONION library -upload -library=scripts/zlib.onion -awsKey=$AWS_KEY -awsSecret=$AWS_SECRET
#$ONION library -upload -library=scripts/lz4.onion -awsKey=$AWS_KEY -awsSecret=$AWS_SECRET
$ONION library -upload -library=scripts/freetype.onion -awsKey=$AWS_KEY -awsSecret=$AWS_SECRET
#$ONION library -upload -library=scripts/freeimage.onion -awsKey=$AWS_KEY -awsSecret=$AWS_SECRET
#$ONION library -upload -library=scripts/squish.onion -awsKey=$AWS_KEY -awsSecret=$AWS_SECRET
#$ONION library -upload -library=scripts/dxc.onion -awsKey=$AWS_KEY -awsSecret=$AWS_SECRET
#$ONION library -upload -library=scripts/mbedtls.onion -awsKey=$AWS_KEY -awsSecret=$AWS_SECRET
#$ONION library -upload -library=scripts/ofbx.onion -awsKey=$AWS_KEY -awsSecret=$AWS_SECRET
#$ONION library -upload -library=scripts/lua.onion -awsKey=$AWS_KEY -awsSecret=$AWS_SECRET
#$ONION library -upload -library=scripts/gtest.onion -awsKey=$AWS_KEY -awsSecret=$AWS_SECRET
#$ONION library -upload -library=scripts/imgui.onion -awsKey=$AWS_KEY -awsSecret=$AWS_SECRET
#$ONION library -upload -library=scripts/embree.onion -awsKey=$AWS_KEY -awsSecret=$AWS_SECRET
#$ONION library -upload -library=scripts/physx.onion -awsKey=$AWS_KEY -awsSecret=$AWS_SECRET
#$ONION library -upload -library=scripts/openal.onion -awsKey=$AWS_KEY -awsSecret=$AWS_SECRET
#$ONION library -upload -library=scripts/llvm.onion -awsKey=$AWS_KEY -awsSecret=$AWS_SECRET
#$ONION library -upload -library=scripts/curl.onion -awsKey=$AWS_KEY -awsSecret=$AWS_SECRET
#$ONION library -upload -library=scripts/bullet.onion -awsKey=$AWS_KEY -awsSecret=$AWS_SECRET
#$ONION library -upload -library=scripts/recast.onion -awsKey=$AWS_KEY -awsSecret=$AWS_SECRET

