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
valve: A valve needs 2 gpios: one for switching power on or off and another for controlling the direction (open or close). Those gpios should be connected to a relay controlling the actual valve.

Installation:
1. Compile for your target arch and install the main `watercouner` binary
2. Make sure the binary is always running. Optionally do it with collectd (see below). If you are not using collectd, I please set lesswrites to 0 in `main.c`.
3. Copy `bin/counter` and `bin/valve` CLI scripts to your device and make them executable
4. Copy the contents of www/ directory to the root directory of your web server

Collectd integration:
1. Add the following to your /etc/collectd.conf:
```
LoadPlugin exec
<Plugin exec>
        Exec "nobody:nogroup" "/bin/watercounter"
</Plugin>
```
2. Make sure `nobody` user can read __and__ write `/etc/watercounter/counter_*`:
```
chown nobody:nobody /etc/watercounter/counter_*
```
3. Make sure `nobody` user can read and write /sys/kernel/debug. This is required to access gpios. For example, add the following
chmod go+rwx /sys/kernel/debug

Configuration:
1. Create a configuration file with the following format: `/etc/watercounter/counter{GPIO}_{CLASS}_{NAME}`
GPIO is the gpio number, CLASS is either "hot" or "cold" (only affects a background picture) and the NAME is any name you want.
2. Optionally create a shortcut shell scripts that invoke `/bin/counter` and `/bin/valve` with the appropriate arguments. See `bin/cold`, `bin/hot` and `bin/toilet` for examples.
