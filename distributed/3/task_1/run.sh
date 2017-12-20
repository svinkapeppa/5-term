#! /usr/bin/env bash

OUT_DIR="out"
NUM_REDUCERS=8

hdfs dfs -rm -r -skipTrash ${OUT_DIR}

yarn jar /opt/cloudera/parcels/CDH/lib/hadoop-mapreduce/hadoop-streaming.jar \
    -D mapreduce.job.name="Top3.stage1" \
    -D mapreduce.job.reduces=$NUM_REDUCERS \
    -files mapper.py,reducer.py \
    -mapper mapper.py \
    -reducer reducer.py \
    -input /data/wiki/en_articles \
    -output ${OUT_DIR}/tmp

yarn jar /opt/cloudera/parcels/CDH/lib/hadoop-mapreduce/hadoop-streaming.jar \
    -D mapreduce.job.name="Top3.stage2" \
    -D mapreduce.job.reduces=1 \
    -files sort.py \
    -mapper cat \
    -reducer sort.py \
    -input ${OUT_DIR}/tmp \
    -output ${OUT_DIR}/res

hdfs dfs -cat ${OUT_DIR}/res/part-00000
