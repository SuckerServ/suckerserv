#!/bin/sh
logrotate -f conf/logrotate.conf -s log/logrotate.status
