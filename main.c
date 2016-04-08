#include <stdio.h>
#include <event2/event.h>
#include <err.h>

#define GPIO_CHIP 0
#define NUMCOUNTER 3

int counter[NUMCOUNTER]={0, 1, 24};

void gpio_init(void)
{
	FILE *file;
	char tmpstr[256];
	int gpio_base;
	int i;

	sprintf(tmpstr, "/sys/class/gpio/gpiochip%d/base", GPIO_CHIP);

	if(!(file=fopen(tmpstr, "r")))
		err(1, "Unable to open %s", tmpstr);

	if(fscanf(file, "%d", &gpio_base)!=1)
		err(1, "Unable to read %s", tmpstr);

	fclose(file);

	printf("gpio base is %d\n", gpio_base);

	if(!(file=fopen("/sys/class/gpio/export", "w")))
		err(1, "Unable to open /sys/class/gpio/export");

	for(i=0; i<NUMCOUNTER; i++)
		if(fprintf(file, "%d", gpio_base + counter[i]) <= 0)
			err(1, "Unable to export gpio %d", counter[i]);

	fclose(file);
}
	

int main(void)
{
	struct event_base *evbase=event_base_new();

	gpio_init();


	event_base_free(evbase);		
	return 0;
}
