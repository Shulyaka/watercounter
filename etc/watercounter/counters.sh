#!/bin/sh

get_gpio() {
	case "$1" in
		hot)
			NAME=hot_16-046903
			GPIO_CTRL=15
			GPIO_STATE=13
			;;
		cold)
			NAME=cold_15-382321
			GPIO_CTRL=8
			GPIO_STATE=16
			;;
		toilet)
			NAME=toilet_15-382336
			GPIO_CTRL=22
			GPIO_STATE=26
			;;
	esac
}
