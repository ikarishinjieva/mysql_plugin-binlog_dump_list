mysql_plugin-binlog_dump_list
=============================

Mysql plugin, select all binlog dump threads with its slave port (NOT peer port in "show processlist").

You can identify binlog dump threads from different mysql slaves of same IP.

Sample
----

    mysql> select * from information_schema.binlog_dump_list;
    +----+----------------+------+------+-----------------------------------------------------------------------+
    | ID | HOST           | PORT | USER | STATUS                                                                |
    +----+----------------+------+------+-----------------------------------------------------------------------+
    |  2 | 192.168.56.202 | 3306 | repl | Master has sent all binlog to slave; waiting for binlog to be updated |
    |  3 | 192.168.56.202 | 3406 | repl | Master has sent all binlog to slave; waiting for binlog to be updated |
    +----+----------------+------+------+-----------------------------------------------------------------------+
    2 rows in set (0.00 sec)


Installation
----
Download the .so from deploy folder

**CAUTION: Since the plugin use mysqld internal variables, so it could not be used by mysqld of different version**
    
Install plugin
    
    mysql> install plugin binlog_dump_list SONAME 'binlog_dump_list.so';
    Query OK, 0 rows affected (0.00 sec)
    
Cannot find plugin for some mysqld version?
----
You can 

1. Open a issue is warmly welcome

2. Compile one

Compile
----
####1. Prepare mysql source, make and make install
####2. Edit CMakeLists.txt, set MYSQL_PATH and MYSQL_SOURCE_PATH

    #SET PATH HERE
    set(MYSQL_PATH "/opt/mysql")
    set(MYSQL_SOURCES_PATH "/vagrant/mysql-5.5.33")
    
####3. Run Cmake
By default, `cmake .` will use MYSQL_PATH/bin/mysql_config to get current mysql compilation options.

You can use `cmake -D FOR_OFFICIAL_MYSQL=true .` to use options which match official mysql. (Tested on mysql 5.5.33 & mysql 5.5.35)

For mysql debug version, use `cmake -D WITH_DEBUG=true .` (Tested on mysql 5.5.33 & mysql 5.5.35)

####4. Run make

####5. Copy plugin .so file

use `make install` to copy plugin .so file to mysql plugin dir

OR, manually copy it to plugin_dir

    mysql> show variables like '%plugin%';
    +---------------+----------------------------------------------+
    | Variable_name | Value                                        |
    +---------------+----------------------------------------------+
    | plugin_dir    | /opt/mysql/lib/plugin/                       |
    +---------------+----------------------------------------------+
    1 row in set (0.00 sec)
    
####6. Install
