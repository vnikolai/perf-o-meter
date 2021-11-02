#!/bin/sh

src=$(dirname $(realpath $0))/..

build=$src/build
echo $build

mkdir $build
cd $build
pwd

cmake $src -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON
make

ctest --output-on-failure
