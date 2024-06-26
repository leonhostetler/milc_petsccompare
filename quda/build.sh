#! /bin/bash

# Build quda
BRANCH=develop
ARCH=sm_70 # V100 sm_70; A100 sm_80
export CXX=g++-8
export CC=gcc-8

if [ -d quda ]
then
  pushd quda
  git pull
  git checkout ${BRANCH}
  popd
else
  git clone -b ${BRANCH} https://github.com/lattice/quda
fi

QUDA_HOME=`pwd`/quda

mkdir -p build
pushd build

QUDA_BUILD=`pwd`
#MULTIGRID_FLAG="-DQUDA_MULTIGRID=ON"
MULTIGRID_FLAG=""

cmake \
    -DCMAKE_BUILD_TYPE=RELEASE \
    -DQUDA_BUILD_SHAREDLIB=ON \
    -DQUDA_CONTRACT=ON \
    -DQUDA_COVDEV=ON \
    -DQUDA_DIRAC_DEFAULT_OFF=ON \
    -DQUDA_DIRAC_STAGGERED=ON \
    -DQUDA_DOWNLOAD_USQCD=ON -DQUDA_QIO=ON -DQUDA_QMP=ON \
    -DQUDA_FORCE_GAUGE=ON \
    -DQUDA_FORCE_HISQ=ON \
    -DQUDA_GPU_ARCH=${ARCH} \
    -DQUDA_TEX=OFF \
    ${MULTIGRID_FLAG} \
    ${QUDA_HOME}

make -k -j16 install

popd


