#include <stdio.h>
#include <event.h>
#include <err.h>

#define GPIO_CHIP 0

void gpio_init(void)
{
	FILE *file;
	char tmpstr[256];
	int gpio_base;

	sprintf(tmpstr, "/sys/class/gpio/gpiochip%d/base", GPIO_CHIP);

	if(!(file=fopen(tmpstr, "r")))
		err(1, "Unable to read %s", tmpstr);

	fscanf(file, "%d", &gpio_base);

	fclose(file);

	printf("gpio base is %d", gpio_base);
}
	

int main(void)
{
	event_init();

	gpio_init();
		
	return 0;
}
