module("luci.statistics.rrdtool.definitions.exec", package.seeall)

function rrdargs(graph, plugin, plugin_instance)
	if "watercounter" == plugin_instance and "exec" == plugin then
		local gauge = {
			title = "%H: Absolute counter values",
			vlabel = "cubic metres",
			alt_autoscale = true,
			number_format = "%5.3lf",
			data = {
				types = { "gauge" },
				options = {
					gauge = {
						title = "%di",
						overlay = true,
						flip = false,
						total = false,
						noarea = true,
					}
				}
			}
		}
		local counter = {
			title = "%H: Water consumption speed",
			vlabel = "cubic metres per second",
			--totals_format = "%5.0lf%s",
			data = {
				types = { "counter" },
				options = {
					counter = {
						title  = "%di",
						total = true,
						--color  = "ff0000",
						--transform_rpn = "1000,*"
					}
				}
			}
		}
		return { gauge, counter }
	end
end
