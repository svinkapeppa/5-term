ADD jar /opt/cloudera/parcels/CDH/lib/hive/lib/hive-serde.jar;
ADD jar /opt/cloudera/parcels/CDH/lib/hive/lib/hive-contrib.jar;
USE mcs2017127;

SET mapreduce.job.reduces=10;
SET hive.input.format=org.apache.hadoop.hive.ql.io.HiveInputFormat;
SET mapreduce.job.maps=10;

SELECT info, COUNT(*) cnt FROM Logs GROUP BY info ORDER BY cnt DESC
