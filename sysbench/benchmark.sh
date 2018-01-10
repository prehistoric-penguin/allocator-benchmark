#!/bin/bash

JEMALLOC=`pwd`/jemalloc/lib/libjemalloc.so
TCMALLOC=`pwd`/gperftools/.libs/libtcmalloc.so

BENCH_DB=sysbench
BENCH_USER=sysbench 
BENCH_PWD=password

LOG_RO=ro.log
LOG_PS=ps.log
TIME_LIMIT=20

TABLE_LINES=10000000

function warm_up() {
    mysql -u$BENCH_USER -p$BENCH_PWD -e "use $BENCH_DB;select avg(id) from sbtest1 FORCE KEY (PRIMARY)"
}

function read_only() {
    sysbench --db-driver=mysql --table-size=$TABLE_LINES --mysql-db=sysbench \
    --percentile=99 --time=$1 \
    --mysql-user=$BENCH_USER --mysql-password=$BENCH_PWD --threads=$2 \
    oltp_read_only run >> $LOG_RO
}

function point_select() {
        sysbench --db-driver=mysql --table-size=$TABLE_LINES --mysql-db=sysbench \
        --percentile=99 \
        --time=$1 --mysql-user=$BENCH_USER --mysql-password=$BENCH_PWD  \
        --threads=$2 \
        oltp_point_select run >> $LOG_PS
}

function prepare_database() {
    echo "Input your root password for mysql"
    read ROOTPWD

    mysql -u root -p$ROOTPWD -e "DROP DATABASE IF EXISTS $BENCH_DB;CREATE DATABASE $BENCH_DB;"
    mysql -u root -p$ROOTPWD -e "CREATE USER IF NOT EXISTS '$BENCH_USER'@'localhost' IDENTIFIED BY '$BENCH_PWD';"
    mysql -u root -p$ROOTPWD -e "GRANT ALL PRIVILEGES ON *.* TO '$BENCH_USER'@'localhost' IDENTIFIED  BY '$BENCH_PWD';"

    sysbench --db-driver=mysql --table-size=$TABLE_LINES --mysql-db=$BENCH_DB \
    --mysql-user=$BENCH_USER --mysql-password=$BENCH_PWD oltp_read_only prepare
}

function increase_file_limit() {
    sudo cp /lib/systemd/system/mysql.service /etc/systemd/system/
    echo -e "LimitNOFILE=infinity\nLimitMEMLOCK=infinity" | sudo tee -a \
    /etc/systemd/system/mysql.service > /dev/null
    sudo systemctl daemon-reload
}

function config_database() {
    increase_file_limit

    echo -e "innodb_buffer_pool_size=3G\nmax_connections=2000"  | sudo tee -a \
    /etc/mysql/mysql.conf.d/mysqld.cnf > /dev/null
    sudo bash -c 'ulimit -u 102400 -n 102400'
    sudo /etc/init.d/mysql restart
}

function test_allocator() {
    alloc=$1
    for threads in 8 64 128 256 512 
    do 
        echo "Current allocator:$alloc" | tee -a  $LOG_RO $LOG_PS > /dev/null

        LD_PRELOAD=$alloc sudo /etc/init.d/mysql restart >/dev/null 2>&1
        
        warm_up  >/dev/null 2>&1

        read_only $TIME_LIMIT $threads
        point_select $TIME_LIMIT $threads 

        ps -eopid,fname,rss,vsz,user,command | \
        grep -e "RSS" -e "mysql" | grep -v "grep"  | tee -a $LOG_RO $LOG_PS > /dev/null
        echo "-----------------------------" | tee -a $LOG_RO $LOG_PS > /dev/null
       
    done
}

# DEFAULT is a placeholder for glibc allocator
function benchmark() {
    echo "Benchmark start at:" `date`
    for allocator in $JEMALLOC $TCMALLOC DEFAULT
    do
        test_allocator $allocator
    done

    echo "Benchmark done at:" `date`
}

function main() {
    case "$1" in
        first_time)
            echo "First time to run, prepare database at first"
            config_database
            prepare_database
            benchmark
            ;;
        *)
            benchmark
            ;;
    esac
}

main $1
