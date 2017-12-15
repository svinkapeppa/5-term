#! /usr/bin/env bash

OUT_DIR="out"
NUM_REDUCERS=8

hdfs dfs -rm -r -skipTrash ${OUT_DIR}

yarn jar /opt/cloudera/parcels/CDH/lib/hadoop-mapreduce/hadoop-streaming.jar \
    -D mapreduce.job.name="SitePageLifespan.stage1" \
    -D mapreduce.job.reduces=$NUM_REDUCERS \
    -files mapper.py,reducer.py \
    -mapper mapper.py \
    -reducer reducer.py \
    -input /data/user_events \
    -output ${OUT_DIR}/tmp

yarn jar /opt/cloudera/parcels/CDH/lib/hadoop-mapreduce/hadoop-streaming.jar \
    -D mapreduce.job.name="SitePageLifespan.stage2" \
    -D mapreduce.job.reduces=1 \
    -files limit.py \
    -mapper limit.py \
    -reducer limit.py \
    -input ${OUT_DIR}/tmp \
    -output ${OUT_DIR}/res

hdfs dfs -cat ${OUT_DIR}/res/part-00000 | head
