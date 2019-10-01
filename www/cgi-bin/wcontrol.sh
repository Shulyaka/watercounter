#!/bin/sh

echo "Status: 200 OK"
echo "Content-Type: application/json"
echo "Cache-Control: no-cache"
echo "Expires: 0"
echo ""

QUERY_STRING="${QUERY_STRING//&/ }"
DATA="$(cat|sed -e 's/.*{//' -e 's/}.*//' -e 's/\"//g' -e 's/ *: */=/g' -e 's/ *, */ /g')"

FUNCTION="lastaction"

for ARG in $QUERY_STRING $DATA; do
	PAR="${ARG%%=*}"  #What the f#*=k is %%=*?
	VAL="${ARG#*=}"
	case "$PAR" in
		valve)
			VALVE="$VAL"
			;;
		function)
			FUNCTION="$VAL"
			;;
		value)
			VALUE="$VAL"
			;;
		_)
			;;
		*)
			echo "{\"success\":0,\"error\":\"Unknown parameter $PAR\"}"
			exit
			;;
	esac
done

case "$FUNCTION" in
	open|close|abort|lastaction) ;;
	set)
		FUNCTION="$FUNCTION $VALUE"
		;;
	*)
		echo "{\"success\":0,\"error\":\"Unknown function\"}"
		exit
		;;
esac

for COUNTER in /etc/watercounter/counter*; do
	COUNTER="$(basename "$COUNTER")"
	test "${COUNTER%%-opkg}" == "$COUNTER" || continue
	COUNTER="${COUNTER#counter*_}"

	NAME="${COUNTER%_*}"
	test "$NAME" == "$VALVE" || continue

	LASTACTION=$($VALVE $FUNCTION)

	echo "{\"success\":1,\"lastaction\":\"$LASTACTION\"}"
	exit
done

echo "{\"success\":0,\"error\":\"Valve not found\"}"
