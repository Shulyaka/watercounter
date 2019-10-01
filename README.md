# watercounter

A set of tools to manage water impulse counter

Features:

* Collect and store readings over gpio
* Chatter filter
* Collectd integration
* Home Assistant integration
* Web interface
* Command line interface
* RESTful interface
* Control electrical valve actuators over gpio

Hardware setup:
I use an `unwired one` board with openwrt-based os.
counter: Just connect your counter with a wire to a free gpio port.
valve: A valve needs 2 gpios: one for switching power on or off and another for controlling the direction (open or close). Those gpios should be connected to a relay controlling the actual valve.
An example configuration is provided for 3 counters: "hot", "cold" and "toilet". Example GPIO used:

| Name   | Input  | Valve power | Valve direction |
|--------|--------|-------------|-----------------|
| hot    | GPIO0  | GPIO15      | GPIO13          |
| cold   | GPIO24 | GPIO17      | GPIO16          |
| toilet | GPIO1  | GPIO22      | GPIO26          |

Installation:
1. Compile for your target arch and install the main `watercouner` binary
2. Make sure the binary is always running. Optionally do it with collectd (see below). If you are not using collectd, I please set lesswrites to 0 in `main.c`.
3. Copy `bin/counter` and `bin/valve` CLI scripts to your device and make them executable
4. Copy `/etc/watercounter/counters.sh` and set the parameters with your GPIO pins
5. Copy the contents of www/ directory to the root directory of your web server

Configuration:
1. Create a configuration file with the following format: `/etc/watercounter/counter{GPIO}_{CLASS}_{NAME}`
GPIO is the gpio number, CLASS is either "hot" or "cold" (only affects a background picture) and the NAME is any name you want
2. Optionally create symbolic links to `/bin/counter` with the appropriate counter names, i.e. cold -> counter, hot -> counter, etc

Collectd integration:
1. Add the following to your /etc/collectd.conf:
```
LoadPlugin exec
<Plugin exec>
        Exec "nobody:nogroup" "/usr/bin/watercounter"
</Plugin>
```
2. Make sure `nobody` user can read __and__ write `/etc/watercounter/counter_*`:
```
chown nobody:nogroup /etc/watercounter/counter*
```
3. Make sure `nobody` user can read and write /sys/kernel/debug. This is required to access gpios. For example, add the following line to the `/etc/init.d/collectd` script in the `start()` function:
```
chmod go+rwx /sys/kernel/debug
```

Home Assistant integration:
Use the following as an example for your configuration.yaml:
```
sensor:
  - platform: rest
    resource: http://watercounter/cgi-bin/wcounter.sh
    name: wcounter
    value_template: 'OK'
    json_attributes: counters
  - platform: template
    sensors:
      counter_0:
        friendly_name_template: "{{states.sensor.wcounter.attributes.counters[0].name.capitalize()}} counter"
        entity_id: sensor.wcounter
        unit_of_measurement: 'm³'
        value_template: "{{'%.3f'|format(states.sensor.wcounter.attributes.counters[0].value/100)}}"
        attribute_templates:
          serial: "{{states.sensor.wcounter.attributes.counters[0].serial}}"
          state: "{{states.sensor.wcounter.attributes.counters[0].state*100/15|int}} %"
          lastaction: "{{states.sensor.wcounter.attributes.counters[0].lastaction}}"
      counter_1:
        friendly_name_template: "{{states.sensor.wcounter.attributes.counters[1].name.capitalize()}} counter"
        entity_id: sensor.wcounter
        unit_of_measurement: 'm³'
        value_template: "{{'%.3f'|format(states.sensor.wcounter.attributes.counters[1].value/100)}}"
        attribute_templates:
          serial: "{{states.sensor.wcounter.attributes.counters[1].serial}}"
          state: "{{states.sensor.wcounter.attributes.counters[1].state*100/15|int}} %"
          lastaction: "{{states.sensor.wcounter.attributes.counters[1].lastaction}}"
      counter_2:
        friendly_name_template: "{{states.sensor.wcounter.attributes.counters[2].name.capitalize()}} counter"
        entity_id: sensor.wcounter
        unit_of_measurement: 'm³'
        value_template: "{{'%.3f'|format(states.sensor.wcounter.attributes.counters[2].value/100)}}"
        attribute_templates:
          serial: "{{states.sensor.wcounter.attributes.counters[2].serial}}"
          state: "{{states.sensor.wcounter.attributes.counters[2].state*100/15|int}} %"
          lastaction: "{{states.sensor.wcounter.attributes.counters[2].lastaction}}"

switch:
  - platform: rest
    resource: http://watercounter/cgi-bin/wcontrol.sh?valve=hot
    name: Hot water
    body_on: '{"function": "open"}'
    body_off: '{"function": "close"}'
    is_on_template: '{{ value_json.lastaction == "open" }}'
  - platform: rest
    resource: http://watercounter/cgi-bin/wcontrol.sh?valve=cold
    name: Cold water
    body_on: '{"function": "open"}'
    body_off: '{"function": "close"}'
    is_on_template: '{{ value_json.lastaction == "open" }}'
  - platform: rest
    resource: http://watercounter/cgi-bin/wcontrol.sh?valve=toilet
    name: Toilet water
    body_on: '{"function": "open"}'
    body_off: '{"function": "close"}'
    is_on_template: '{{ value_json.lastaction == "open" }}'
```
