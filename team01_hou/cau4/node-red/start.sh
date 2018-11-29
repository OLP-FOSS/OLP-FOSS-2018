#!/bin/sh

#systemctl start mysql
#systemctl start mosquitto

OLDPWD="$PWD"
cd './prog/mosquitto'
LD_LIBRARY_PATH=. ./mosquitto -d -c ./mosquitto.conf
cd $OLDPWD
OLDPWD="$PWD"
cd './prog/influxdb'
LD_LIBRARY_PATH=. ./influxd run -config ./influxdb.conf &
cd $OLDPWD
./node_modules/.bin/node-red -u .

