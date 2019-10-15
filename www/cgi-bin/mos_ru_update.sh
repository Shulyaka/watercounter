#!/bin/sh

echo "Status: 200 OK"
echo "Content-Type: text/plain"
echo "Cache-Control: no-cache"
echo "Expires: 0"
echo ""

/root/mos.ru/water.sh set $(cold get | sed 's/...$/.&/') $(toilet get | sed 's/...$/.&/') $(hot get | sed 's/...$/.&/')
