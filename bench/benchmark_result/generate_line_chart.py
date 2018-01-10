#!/bin/python
import json
import os
import logging
import collections
from os import listdir
from os.path import isfile, join
import matplotlib.pyplot as plt
import numpy as np
import matplotlib.backends.backend_pdf

FORMAT = "%(message)s"
logging.basicConfig(format=FORMAT, level=logging.FATAL)
work_path="./"

class Row:
    def __init__(self, label, row_data):
        self.label = label
        self.row_data = row_data

    def __str__(self):
        return "{0: <15}\t{1}".format(self.label, self.row_data)

class Table:
    def __init__(self, title, xticks):
        self.title = title
        self.xticks = xticks
        self.table = []

    def add_row(self, r):
        if len(self.xticks) != len(r.row_data):
            logging.fatal("error in data, not all case in same threads range:%s, %s",
                         self.xticks, r)
        self.table.append(r)

    def draw(self, pdf_file):
        fig = plt.figure()
        for row in self.table:
            x = np.array(range(len(self.xticks)))
            y = np.array(row.row_data)

            plt.xticks(x, self.xticks)
            plt.plot(x, y, label = row.label)
        pass
        plt.xlabel('Threads Num')
        plt.ylabel('CPU Time(ns)')
        bm_name = self.title.replace("BM_", "benchmark:")
        if "/" in bm_name:
            name, par = bm_name.split("/")
            bm_name = name + " with parameter:" + par
        plt.title(bm_name)
        plt.grid(True, axis='y')
        plt.legend()

        pdf_file.savefig(fig)

    def __str__(self):
        return "title:{0}, xticks:{1}, content:\n{2}".format(
                self.title, self.xticks, "\n".join(list(map(str, self.table))))

def parse(f, tables):
    j = json.load(open(join(work_path, f)))
    assert j

    bench_key = "benchmarks"
    bench = j[bench_key]

    previous_bench = ""
    xticks = []
    row_data = []

    def insert_row_or_init(row):
        if previous_bench not in tables:
            t = Table(previous_bench, xticks)
            t.add_row(row)
            tables[previous_bench] = t
        else:
            tables[previous_bench].add_row(row)

    for b in bench:
        name = b["name"]
        bench_name, thr = name.split("/threads:")
        if previous_bench != "" and previous_bench != bench_name:
            logging.debug("add new row:%s", row_data)
            row = Row(os.path.splitext(f)[0].replace("-benchmark", ""), row_data)
            insert_row_or_init(row)

            xticks = []
            row_data = []

        previous_bench = bench_name
        xticks.append(thr)
        row_data.append(b["cpu_time"])

    row = Row(os.path.splitext(f)[0].replace("-benchmark", ""), row_data)
    insert_row_or_init(row)


def extract():
    alljson = [f for f in listdir(work_path) if isfile(join(work_path, f)) and
            f.endswith(".json")]

    tables = collections.OrderedDict()
    for fname in alljson:
        parse(fname, tables)

    plt.rcParams.update({'figure.max_open_warning': 0})
    pdf = matplotlib.backends.backend_pdf.PdfPages("benchmark.pdf")
    for table in tables.values():
        logging.debug("dumping table:%s", table)
        table.draw(pdf)
    pdf.close()


def main():
    extract()

if __name__ == "__main__":
    main()
