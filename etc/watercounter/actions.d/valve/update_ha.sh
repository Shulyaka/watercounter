#!/bin/sh

HASS="http://hassio.local:8123"
TOKEN=""

NAME=$1
ACTION=$2

STATE="on"
test "$ACTION" = "close" && STATE="off"

curl -X POST -s -S -o /dev/null \
	-H "Authorization: Bearer $TOKEN" \
	-H "Content-Type: application/json" \
	-d "{\"state\": \"$STATE\"}" \
	$HASS/api/states/switch.${NAME%_*}_water
