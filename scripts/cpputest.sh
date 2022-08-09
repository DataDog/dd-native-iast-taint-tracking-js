#!/bin/sh
TEST_FOLDER="test/cpputest"
TEST_BUILD_FOLDER="build"
TEST_BINARY="native_test"

if [ "$1" = "ci" ]; then
    TEST_OPTIONS="-ojunit"
else
    TEST_OPTIONS="-v"
fi

echo $TEST_OPTIONS
mkdir -p $TEST_FOLDER/$TEST_BUILD_FOLDER
cd $TEST_FOLDER/$TEST_BUILD_FOLDER
cmake ..
make && ./$TEST_BINARY $TEST_OPTIONS

if [ "$1" = "ci" ]; then
    mkdir ../results
    mv *.xml ../results
fi

