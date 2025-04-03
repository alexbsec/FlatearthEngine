#!/bin/bash

set echo on

mkdir -p bin

pushd engine
./run.sh
popd

pushd testsuite
./run.sh
popd

pushd tests
./run.sh
popd
