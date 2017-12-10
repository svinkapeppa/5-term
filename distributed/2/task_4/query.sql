ADD jar /opt/cloudera/parcels/CDH/lib/hive/lib/hive-contrib.jar;
ADD FILE ./names.py;
USE mcs2017127;

SET mapreduce.job.reduces=10;
SET hive.input.format=org.apache.hadoop.hive.ql.io.HiveInputFormat;
SET mapreduce.job.maps=10;

SELECT TRANSFORM(ip, datetime, request, size, code, info)
USING 'python3 names.py'
AS ip,
   datetime,
   request,
   size,
   code,
   info
FROM Logs
LIMIT 10;
