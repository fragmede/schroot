#!/bin/sh
# This script ensures that the test data is owned by root.

rm -rf test/testdata
mkdir test/testdata
cp -r ${PROJECT_SOURCE_DIR}/test/*.ex* test/testdata
mkdir test/testdata/run-parts.ex2
chown -R root:root test/testdata
