# !/bin/bash
set -e 
set -o pipefail

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

export TEST=`pwd`/tests/
echo Test directory: "$TEST"

# compile the test project
$ONION configure -module="$TEST" 
$ONION generate -module="$TEST"
$ONION build -module="$TEST"
$ONION test -module="$TEST" 


