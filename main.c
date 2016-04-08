#include <stdio.h>
#include <err.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>

#define SIG_GPIO_IRQ	42

#define GPIO_CHIP	0
#define NUMCOUNTER	3

enum state
{
	lo=0,
	hi=1
};

struct counter
{
	int gpio;
	unsigned long value;
	enum state laststate;
	time_t timestamp;
} counter[NUMCOUNTER]={{0}, {1}, {24}};

void counter_update(int gpio, enum state val)
{
	int i;

	printf("gpio: %d, state: %d\n", gpio, val);

	for(i=0; i<NUMCOUNTER && counter[i].gpio!=gpio; i++);

	if(i==NUMCOUNTER)
	{
		printf("Warning: Unknown gpio pin %d\n", gpio);
		return;
	}

	if(val==hi)
		counter[i].value++;

	counter[i].timestamp=time(NULL);
	counter[i].laststate=val;

	printf("counter: %d, value: %lu\n", i, counter[i].value);
}

void irq_handler(int n, siginfo_t *info, void *unused)
{
	counter_update(info->si_int >> 24, info->si_int & 1);
}

void gpio_init(void)
{
	FILE *file;
	char tmpstr[256];
	int gpio_base;
	int i;
	struct sigaction sig;

	sprintf(tmpstr, "/sys/class/gpio/gpiochip%d/base", GPIO_CHIP);

	if(!(file=fopen(tmpstr, "r")))
		err(1, "Unable to open %s", tmpstr);

	if(fscanf(file, "%d", &gpio_base)!=1)
		err(1, "Unable to read %s", tmpstr);

	fclose(file);

/*
	if(!(file=fopen("/sys/class/gpio/export", "w")))
		err(1, "Unable to open /sys/class/gpio/export");

	for(i=0; i<NUMCOUNTER; i++)
	{
		counter[i].gpio+=gpio_base;
		if(fprintf(file, "%d", counter[i].gpio) <= 0)
			err(1, "Unable to export gpio %d", counter[i].gpio);
		fflush(file);
	}

	fclose(file);

	for(i=0; i<NUMCOUNTER; i++)
	{
		sprintf(tmpstr, "/sys/class/gpio/gpio%d/direction", counter[i].gpio);
		if(!(file=fopen(tmpstr, "w")) || fprintf(file, "in")<=0)
			err(1, "Unable to set gpio %d direction", counter[i].gpio);
		fclose(file);

		sprintf(tmpstr, "/sys/class/gpio/gpio%d/active_low", counter[i].gpio);
		if(!(file=fopen(tmpstr, "w")) || fprintf(file, "0")<=0)
			err(1, "Unable to set gpio %d active_low setting", counter[i].gpio);
		fclose(file);

		sprintf(tmpstr, "/sys/class/gpio/gpio%d/edge", counter[i].gpio);
		if(!(file=fopen(tmpstr, "w")) || fprintf(file, "both")<=0)
			err(1, "Unable to set gpio %d edge setting", counter[i].gpio);
		fclose(file);
	}
*/

	sig.sa_sigaction=irq_handler;
	sig.sa_flags = SA_SIGINFO;
	if(sigaction(SIG_GPIO_IRQ, &sig, NULL)<0)
		err(1, "Unable to set interrupt handler");

	if(!(file=fopen("/sys/kernel/debug/gpio-irq", "w")))
		err(1, "Unable to open /sys/kernel/debug/gpio-irq");

	for(i=0; i<NUMCOUNTER; i++)
	{
		counter[i].gpio+=gpio_base;
		if(fprintf(file, "+ %d %d\n", counter[i].gpio, getpid()) <= 0)
			err(1, "Unable to register irq handler for gpio %d", counter[i].gpio);
		fflush(file);
	}

	fprintf(file, "?\n", counter[i].gpio, getpid());
	fclose(file);

}

void gpio_free(void)
{
	FILE *file;
	char tmpstr[256];
	int i;
	struct sigaction sig;

	if(!(file=fopen("/sys/kernel/debug/gpio-irq", "w")))
		err(1, "Unable to open /sys/kernel/debug/gpio-irq");

	for(i=0; i<NUMCOUNTER; i++)
	{
		if(fprintf(file, "- %d %d\n", counter[i].gpio, getpid()) <= 0)
			err(1, "Unable to clear irq handler for gpio %d", counter[i].gpio);
		fflush(file);
	}
	fprintf(file, "?\n", counter[i].gpio, getpid());

	fclose(file);

	sig.sa_handler=SIG_DFL;
	sig.sa_flags = SA_SIGINFO | SA_NODEFER;
	if(sigaction(SIG_GPIO_IRQ, &sig, NULL)<0)
		err(1, "Unable to clear interrupt handler");
/*
	if(!(file=fopen("/sys/class/gpio/unexport", "w")))
		err(1, "Unable to open /sys/class/gpio/unexport");

	for(i=0; i<NUMCOUNTER; i++)
	{
		if(fprintf(file, "%d", counter[i].gpio) <= 0)
			err(1, "Unable to unexport gpio %d", counter[i].gpio);
		fflush(file);
	}

	fclose(file);
*/
}

int main(void)
{
	sigset_t sigset;
	siginfo_t siginfo;

	gpio_init();

	sigemptyset(&sigset);
	sigaddset(&sigset, SIGINT);
	sigprocmask(SIG_BLOCK, &sigset, NULL);

	printf("Waiting for gpio...\n");

	while(1)
	{
		sigwaitinfo(&sigset, &siginfo);

		if(siginfo.si_signo == SIGINT)
			break;
	}

	gpio_free();

	printf("\nNormal exit\n");

	return 0;
}
