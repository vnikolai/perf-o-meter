#!/bin/sh

src=$(dirname $(realpath $0))/..

build=$src/build
echo $build

rm -rdf $build
mkdir $build
cd $build
pwd

cmake $src -DCMAKE_BUILD_TYPE=Release \
           -DPERFOMETER_BUILD_PRINTER=ON \
           -DPERFOMETER_BUILD_SAMPLES=ON \
           -DPERFOMETER_BUILD_VISUALIZER=ON \
           -DPERFOMETER_BUILD_TESTS=ON \
           -DPERFOMETER_BUILD_BENCHMARKS=ON
make

ctest --output-on-failure
