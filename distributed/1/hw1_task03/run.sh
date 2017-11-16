#!/usr/bin/env bash

hdfs fsck $1 | python script.py
