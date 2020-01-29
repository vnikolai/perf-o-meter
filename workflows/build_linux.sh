#!/bin/sh

src=$(dirname $(realpath $0))/..

build=$src/build
echo $build

rm -rdf $build
mkdir $build

cd $build
pwd

cmake $src
make
