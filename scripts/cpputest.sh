#!/bin/sh
TEST_FOLDER="test/cpputest"
TEST_BUILD_FOLDER="build"
TEST_BINARY="native_test"
TEST_OPTIONS="-v"

while :; do
    case "$1" in
    --asan)
        CXXFLAGS="-g -O0 -fsanitize=address"
        LDFLAGS="-fsanitize=address"
        shift
        ;;
    --ci)
        TEST_OPTIONS="-ojunit"
        shift
        ;;
    *)
        break
        ;;
    esac
done

echo $TEST_OPTIONS
mkdir -p $TEST_FOLDER/$TEST_BUILD_FOLDER
rm -rf $TEST_FOLDER/$TEST_BUILD_FOLDER/*
cd $TEST_FOLDER/$TEST_BUILD_FOLDER
CXXFLAGS=$CXXFLAGS LDFLAGS=$LDFLAGS cmake .. && make
./$TEST_BINARY $TEST_OPTIONS || exit 1

if [ "$TEST_OPTIONS" = "-ojunit" ]; then
    mkdir ../results
    mv *.xml ../results
fi

