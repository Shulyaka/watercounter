#include <stdio.h>
#include <err.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

#define SIG_GPIO_IRQ	42

#define GPIO_CHIP	0
#define NUMCOUNTER	3

int counter[NUMCOUNTER]={0, 1, 24};

void irq_handler(int n, siginfo_t *info, void *unused)
{
	printf("Received value 0x%X\n", info->si_int);
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

	sig.sa_sigaction=irq_handler;
	sig.sa_flags = SA_SIGINFO;
	if(sigaction(SIG_GPIO_IRQ, &sig, NULL)<0)
		err(1, "Unable to set interrupt handler");

	printf("PID is %d\n", getpid());

	if(!(file=fopen("/sys/kernel/debug/gpio-irq", "w")))
		err(1, "Unable to open /sys/kernel/debug/gpio-irq");

	for(i=0; i<NUMCOUNTER; i++)
	{
		counter[i]+=gpio_base;
		if(fprintf(file, "+ %d %d", counter[i], getpid()) <= 0)
			err(1, "Unable to register irq handler for gpio %d", counter[i]);
		fflush(file);
	}

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
		if(fprintf(file, "- %d %d", counter[i], getpid()) <= 0)
			err(1, "Unable to clear irq handler for gpio %d", counter[i]);
		fflush(file);
	}

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
		if(fprintf(file, "%d", counter[i]) <= 0)
			err(1, "Unable to unexport gpio %d", counter[i]);
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
		{
			printf("Received SIGINT\n");
			break;
		}
	}

	gpio_free();

	printf("\nNormal exit\n");

	return 0;
}
