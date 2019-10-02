#!/bin/sh

HASS="http://hassio.local:8123"
TOKEN=""

NAME=$1
STATE=$2

curl -X POST -s -S -o /dev/null \
	-H "Authorization: Bearer $TOKEN" \
	-H "Content-Type: application/json" \
	-d '{"entity_id": "sensor.wcounter"}' \
	$HASS/api/services/homeassistant/update_entity
