-- Server name
server.servername = "piernov.org"

-- Server's IP address
--server.serverip = "0.0.0.0"

-- Server's port
server.serverport = 28787

-- use text file as db
--server.use_text_database("./log/auths.txt")

-- or use a mysql db
--server.use_mysql_database("hostname", "port", "database", "username", "password", "install_db")
server.use_mysql_database("localhost", "3306", "database", "username", "password")

-- or use a sqlite3 db
--server.use_sqlite_database (./db")

server.debug = 1

server.web_admins = {}
