# Allocator benchmark tests from [tcmalloc](https://github.com/gperftools/gperftools/blob/master/benchmark/malloc_bench.cc)

## Prerequisites
* Google benchmark
* GCC/Clang support c++11
* Cmake

## Running

The script will build the source and start benchmark automaticlly
```sh
./benchmark.sh
```

To generate the line chart, you need python2 or python3 installed, with the
additional package:

* matplotlib
* numpy

```sh
cd benchmark_result
python ./generate_line_chart.py
```
