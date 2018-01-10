# allocator-benchmark
Benchmark for tcmalloc, jemalloc

```
The scripts will append two lines into your mysql config file. You should not
run it in a production machine.
```

## Prerequisites
* mysql-server
* sysbench

# Steps

## Install mysql-server and sysbench(For Ubuntu)

```sh
sudo apt-get install mysql-server

curl -s \
https://packagecloud.io/install/repositories/akopytov/sysbench/script.deb.sh | \
sudo bash
sudo apt -y install sysbench
```

## Run benchmark
**NOTE** You need to input your password for mysql user root

Use first_time to set up test tables and config file

```sh
./benchmark.sh first_time
```

```sh
./benchmark.sh 
```

## Benchmark result

Find them in ps.log(oltp_point_select) and ro.log(oltp_readonly)

precission_sysbench_benchmark.xlsx is the result from precission notebook
vistual_machine_sysbench_benchmark.xlsx is the result from out powerful test machine, whos ip is 10.27.18.68
