#!/bin/sh

echo "Status: 200 OK"
echo "Content-Type: application/json"
echo "Cache-Control: no-cache"
echo "Expires: 0"
echo ""
echo -n "{\"counters\":["
NOTFIRST=""
counter list | while read NAME SERIAL VALUE STATE LASTACTION; do
	test -n "$NOTFIRST" && echo -n ","
	NOTFIRST=Y

	echo -n "{\"name\":\"$NAME\","
	echo -n "\"serial\":\"$SERIAL\","
	echo -n "\"value\":${VALUE:-null},"
	echo -n "\"state\":${STATE:-null},"
	echo -n "\"lastaction\":\"${LASTACTION:-open}\"}"
done
echo "]}"

