#!/bin/bash -e

SCRIPTDIR=`dirname $0`
SCRIPTDIR=`cd $SCRIPTDIR && pwd -P`
BASEDIR=${SCRIPTDIR}/..
BASEDIR=`cd ${BASEDIR} && pwd -P`
BUILDDIR=${BASEDIR}/Build
BUILDDIR=`cd ${BUILDDIR} && pwd -P`

BUILDTYPE=Debug
if [ $# -ge 1 ]; then
    BUILDTYPE=$1
fi
ENABLE_CCACHE=ON
if [ $BUILDTYPE != Debug ]; then
    ENABLE_CCACHE=OFF
fi

pushd ${BUILDDIR}

# Build Converter
pushd Converter
/bin/rm -rf build
/bin/mkdir build
pushd build
if type "ninja" > /dev/null; then
    cmake -G Ninja -DCMAKE_BUILD_TYPE=${BUILDTYPE} ..
else
    cmake -DCMAKE_BUILD_TYPE=${BUILDTYPE} ..
fi
cmake --build . --parallel
ctest .
popd > /dev/null # build
popd > /dev/null # Converter

popd > /dev/null # ${BUILDDIR}
