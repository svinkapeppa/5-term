ADD jar /opt/cloudera/parcels/CDH/lib/hive/lib/hive-contrib.jar;
ADD FILE /home/mcs2017/mcs2017127/hw2/years.py;
USE mcs2017127;

SET mapreduce.job.reduces=10;
SET hive.input.format=org.apache.hadoop.hive.ql.io.HiveInputFormat;
SET mapreduce.job.maps=10;

SELECT TRANSFORM(age)
USING 'python3 years.py'
AS age
FROM Users
LIMIT 10;

