ADD jar /opt/cloudera/parcels/CDH/lib/hive/lib/hive-serde.jar;
ADD jar /opt/cloudera/parcels/CDH/lib/hive/lib/hive-contrib.jar;
USE mcs2017127;

DROP TABLE IF EXISTS LogsTemp;
CREATE EXTERNAL TABLE LogsTemp (
    ip STRING,
    datetime INT,
    request STRING,
    size INT,
    code INT,
    info STRING
)
ROW FORMAT SERDE 'org.apache.hadoop.hive.serde2.RegexSerDe'
WITH SERDEPROPERTIES (
    "input.regex" = '^(\\S*)\\t\\t\\t(\\d{8})\\S*\\t(\\S*)\\t(\\S*)[ \\t](\\S*)[ \\t](\\S*)[ \\t].*$'
)
STORED AS TEXTFILE
LOCATION '/data/user_logs/user_logs_M';

SET hive.exec.dynamic.partition.mode=nonstrict;
SET hive.exec.max.dynamic.partitions.pernode=300;
SET hive.exec.max.dynamic.partitions=2100;

DROP TABLE IF EXISTS Logs;
CREATE EXTERNAL TABLE Logs (
    ip STRING,
    request STRING,
    size INT,
    code INT,
    info STRING
)
PARTITIONED BY (datetime INT)
STORED AS TEXTFILE;
INSERT OVERWRITE TABLE Logs PARTITION (datetime)
SELECT ip, request, size, code, info, datetime FROM LogsTemp;

DROP TABLE IF EXISTS Users;
CREATE EXTERNAL TABLE Users (
    ip STRING,
    agent STRING,
    sex STRING,
    age INT
)
ROW FORMAT SERDE 'org.apache.hadoop.hive.serde2.RegexSerDe'
WITH SERDEPROPERTIES (
    "input.regex" = '^(\\S*)\\t(\\S*)\\t(\\S*)\\t(\\S*).*$'
)
STORED AS TEXTFILE
LOCATION '/data/user_logs/user_data_M';

DROP TABLE IF EXISTS IPRegions;
CREATE EXTERNAL TABLE IPRegions (
    ip STRING,
    region STRING
)
ROW FORMAT SERDE 'org.apache.hadoop.hive.serde2.RegexSerDe'
WITH SERDEPROPERTIES (
    "input.regex" = '^(\\S*)\\t(\\S*).*$'
)
LOCATION '/data/user_logs/ip_data_M';

SELECT * FROM Logs LIMIT 10;
