# Creates greeting.say_hello(), a stored procedure that shows the message
# GREETING() reads from greeting.conf. Run it after INSTALL SONAME
# 'greeting', for example:
#   mariadb < /usr/share/mariadb/greeting/greeting.sql
#   CALL greeting.say_hello();

CREATE DATABASE IF NOT EXISTS greeting;

CREATE OR REPLACE PROCEDURE greeting.say_hello()
  SQL SECURITY INVOKER
  COMMENT 'Shows the message set in greeting.conf'
  SELECT GREETING() AS greeting;
