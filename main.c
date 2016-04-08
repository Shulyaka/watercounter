#include <stdio.h>
#include <err.h>
#include <sys/types.h>
#include <unistd.h>

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

/*
	if(!(file=fopen("/sys/class/gpio/export", "w")))
		err(1, "Unable to open /sys/class/gpio/export");

	for(i=0; i<NUMCOUNTER; i++)
	{
		counter[i]+=gpio_base;
		if(fprintf(file, "%d", counter[i]) <= 0)
			err(1, "Unable to export gpio %d", counter[i]);
		fflush(file);
	}

	fclose(file);

	for(i=0; i<NUMCOUNTER; i++)
	{
		sprintf(tmpstr, "/sys/class/gpio/gpio%d/direction", counter[i]);
		if(!(file=fopen(tmpstr, "w")) || fprintf(file, "in")<=0)
			err(1, "Unable to set gpio %d direction", counter[i]);
		fclose(file);

		sprintf(tmpstr, "/sys/class/gpio/gpio%d/active_low", counter[i]);
		if(!(file=fopen(tmpstr, "w")) || fprintf(file, "0")<=0)
			err(1, "Unable to set gpio %d active_low setting", counter[i]);
		fclose(file);

		sprintf(tmpstr, "/sys/class/gpio/gpio%d/edge", counter[i]);
		if(!(file=fopen(tmpstr, "w")) || fprintf(file, "both")<=0)
			err(1, "Unable to set gpio %d edge setting", counter[i]);
		fclose(file);
	}
*/
	printf("PID is %d\n", getpid());

	if(!(file=fopen("/sys/kernel/debug/gpio-irq", "w")))
		err(1, "Unable to open /sys/kernel/debug/gpio-irq");

	for(i=0; i<NUMCOUNTER; i++)
	{
		counter[i]+=gpio_base;
		if(fprintf(file, "+ %d %d", counter[i], getpid()) <= 0)
			err(1, "Unable to register handler for gpio %d", counter[i]);
		fflush(file);
	}

	fclose(file);

}

void gpio_free(void)
{
	FILE *file;
	char tmpstr[256];
	int i;

	if(!(file=fopen("/sys/kernel/debug/gpio-irq", "w")))
		err(1, "Unable to open /sys/kernel/debug/gpio-irq");

	for(i=0; i<NUMCOUNTER; i++)
	{
		if(fprintf(file, "- %d %d", counter[i], getpid()) <= 0)
			err(1, "Unable to register handler for gpio %d", counter[i]);
		fflush(file);
	}

	fclose(file);
/*
	if(!(file=fopen("/sys/class/gpio/unexport", "w")))
		err(1, "Unable to open /sys/class/gpio/unexport");

	for(i=0; i<NUMCOUNTER; i++)
	{
		if(fprintf(file, "%d", counter[i]) <= 0)
			err(1, "Unable to unexport gpio %d", counter[i]);
		fflush(file);
	}

	fclose(file);
*/
}

int main(void)
{
	gpio_init();

	sleep(5);

	gpio_free();

	return 0;
}
