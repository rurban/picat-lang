#!/bin/sh
# dos2unix and chmod -x

find . -type f -exec dos2unix '{}' \;
find . -type f -exec chmod -x '{}' \;
