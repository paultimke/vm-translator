#!/bin/bash

function buildDebug() {
    CURR_DIR=$(pwd)
    cmake -S . -B _build/Debug -DCMAKE_BUILD_TYPE=Debug || return $?
    cmake --build _build/Debug || return $?
    cd _build/Debug && make install || return $?
    mv compile_commands.json ${CURR_DIR}
    cd ${CURR_DIR}
}

if [[ $# == 0 ]]; then
    buildDebug
elif [[ $# == 1 ]]; then
    if [[ $1 == "clean" ]]; then
        rm -rf _build
        rm ./vm-translator

        # Delete google test generated directories
        rm -rf lib
        rm -rf include
    elif [[ $1 == "release" ]]; then
        cmake -S . -B _build/Release -DCMAKE_BUILD_TYPE=Release || exit $?
        cmake --build _build/Release || exit $?
        cd _build/Release && make install || exit $?
        cd ..
    elif [[ $1 == "test" ]]; then
        buildDebug || exit $?
        ./_build/Debug/test/vm-translator-tests || exit $?
    fi
else
    echo "run as ./build.sh or ./build.sh <arg>"
fi
