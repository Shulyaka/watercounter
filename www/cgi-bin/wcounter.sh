#!/bin/sh

echo "Status: 200 OK"
echo "Content-Type: application/json"
echo "Cache-Control: no-cache"
echo "Expires: 0"
echo ""
echo -n "{\"counters\":["
NOTFIRST=""
for COUNTER in /etc/watercounter/counter*; do
	COUNTER="$(basename "$COUNTER")"
	test "${COUNTER%%-opkg}" == "$COUNTER" || continue
	test -n "$NOTFIRST" && echo -n ","
	NOTFIRST=Y
	COUNTER="${COUNTER#counter*_}"

	NAME="${COUNTER%_*}"
	SERIAL="${COUNTER##*_}"
	VALUE="$(counter $COUNTER get)"
	STATE="$(${COUNTER%_*} state)"

	echo -n "{\"name\":\"$NAME\","
	echo -n "\"serial\":\"$SERIAL\","
	echo -n "\"value\":${VALUE:-null},"
	echo -n "\"state\":${STATE:-null}}"
done
echo "]}"

