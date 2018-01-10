#!/bin/bash

WORKDIR=$(pwd)
BUILDPATH=$(pwd)/build

function test_allocator() {
    alloc=$1
    name=$(basename $alloc)
    alloc_name="${name%.*}"

    echo "Current allcator is:"$alloc_name 
    LD_PRELOAD=$alloc $BUILDPATH/bench --benchmark_out_format=json \
        --benchmark_out=$WORKDIR/benchmark_result/$alloc_name.json 
}

function benchmark() {
    echo "Benchmark start at:" $(date)
    for f in $(ls $BUILDPATH/*/*_bench)
    do
        pre=$(basename $f)
        echo "running $pre"
        $f --benchmark_out_format=json --benchmark_out=./benchmark_result/$pre.json; 
    done
    echo "Benchmark done at:" $(date)
}

function build() {
    mkdir $BUILDPATH > /dev/null 2>&1
    rm -rf $BUILDPATH/*
    cd $BUILDPATH && cmake ..
    make
}

if [ "$#" -eq  "0" ]
then
    benchmark
else
    build
    benchmark
fi
