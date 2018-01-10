#!/bin/bash

cd gperftools && ./autogen.sh && ./configure && make -j4
cd ..

cd jemalloc && ./autogen.sh && ./configure && make -j4
cd ..

echo "Building done."
