# Benchmark for allocators

## Fetch source for allocator, and build 

```sh
./build_allocators.sh
```

# Dependency for Redhat
```
yum install autoconf
yum install automake
yum install libtool
```

## Run benchmarks
* sysbench
* tcbench

## Reference
* sysbench:
  https://www.percona.com/blog/2012/07/05/impact-of-memory-allocators-on-mysql-performance/
* tcbench: benchmark cases from gperftools/tcmalloc, adapt with google
  benchmark framework
