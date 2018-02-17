# watercounter

A set of tools to manage water impulse counter

Features:

* Collect and store readings over gpio
* Chatter filter
* Collectd integration
* Web interface
* Command line interface
* Control electrical valve actuators over gpio

Hardware setup:
I use an `unwired one` board with openwrt-based os.
counter: Just connect your counter with a wire to a free gpio port.
valve: A valve needs 2 gpios: one for switching power on or off and another for controlling the direction (open or close).

Configuration:
1. Create a configuration file with the following format: /etc/watercounter/counter{GPIO}_{CLASS}_{NAME}
GPIO is the gpio number, class is either "hot" or "cold" (only affects a background picture) and the name is any name you want.

Collectd integration:
1. Add the following to your /etc/collectd.conf:
```
LoadPlugin exec
<Plugin exec>
        Exec "nobody:nogroup" "/usr/bin/watercounter"
</Plugin>
```
2. Make sure `nobody` user can read __and__ write /etc/watercounter/counter_*:
```
```
3. Make sure `nobody` user can read and write /sys/kernel/debug. For example, add the following
chmod go+rwx /sys/kernel/debug
