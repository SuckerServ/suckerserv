<?php
$serverhost = "127.0.0.1";        // Host of the HopMod server
$serverport = "28788";            // Port of the serverexec socket
$db = array(
                "type" => "mysql",               // Type of database : mysql, sqlite3 WARNING :SQLite3 DOESN'T WORK
                "path" => "stats.sqlite",        // Path to SQLite3 Database
                "host" => "localhost",           // MySQL Database host
                "name" => "suckerserv",          // MySQL Database name
                "user" => "suckerserv",          // User for MySQL Database
                "pass" => "suckerserv"           // Password for MySQL Database
            );
$rows_per_page =  "20";
?>
