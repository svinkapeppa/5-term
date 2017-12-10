ADD jar /opt/cloudera/parcels/CDH/lib/hive/lib/hive-serde.jar;
ADD jar /opt/cloudera/parcels/CDH/lib/hive/lib/hive-contrib.jar;
USE mcs2017127;

SET mapreduce.job.reduces=10;
SET hive.input.format=org.apache.hadoop.hive.ql.io.HiveInputFormat;
SET mapreduce.job.maps=10;

SELECT code, SUM(male), SUM(female)
FROM (SELECT code,
             IF(sex = 'male', count(*), 0) male,
             IF(sex = 'female', count(*), 0) female 
      FROM Logs JOIN Users ON (Logs.ip = Users.ip) 
      GROUP BY code, sex) TEMP
GROUP BY code;

