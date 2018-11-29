#!/bin/sh

./prog/utils/killall -q -TERM node
./prog/utils/killall -q -TERM node-red
./prog/utils/killall -q -TERM mysqld
./prog/utils/killall -q -TERM mosquitto


