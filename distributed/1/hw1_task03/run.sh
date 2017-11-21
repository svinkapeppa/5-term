#! /usr/bin/env bash

hdfs fsck $1 2>/dev/null | python script.py
