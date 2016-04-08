#include <stdio.h>
#include <err.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>

#define SIG_GPIO_IRQ	42

#define GPIO_CHIP	0
#define MAXCOUNTER	20

enum state
{
	lo=0,
	hi=1,
	unknown
};

struct counter
{
	int gpio;
	unsigned long value;
	enum state laststate;
	time_t timestamp;
	char filename[256];
} counter[MAXCOUNTER];

int numcounter=0;
int gpio_base;

void counter_update(int gpio, enum state val)
{
	int i;

	printf("gpio: %d, state: %d\n", gpio, val);

	for(i=0; i<numcounter && counter[i].gpio!=gpio; i++);

	if(i==numcounter)
	{
		printf("Warning: Unknown gpio pin %d\n", gpio);
		return;
	}

	if(val==hi)
	{
		counter[i].value++;
		printf("counter: %d, value: %lu\n", i, counter[i].value);
	}

	counter[i].timestamp=time(NULL);
	counter[i].laststate=val;
}

void irq_handler(int n, siginfo_t *info, void *unused)
{
	counter_update(info->si_int >> 24, info->si_int & 1);
}

void gpio_init(void)
{
	FILE *file;
	char tmpstr[256];
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

	for(i=0; i<numcounter; i++)
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

	for(i=0; i<numcounter; i++)
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

int namefilter(const struct dirent *de)
{
	if(strncmp(de->d_name, "counter", 7))
		return 0;
	return de->d_name[7]>='0' && de->d_name[7]<='9' && (de->d_type==DT_LNK || de->d_type==DT_REG || de->d_type==DT_UNKNOWN);
}

void counter_init(const char *dirp)
{
	struct dirent **namelist;
	struct stat filestat;
	FILE *file;
	int i;

	if((numcounter=scandir(dirp, &namelist, namefilter, alphasort))<0)
		err(1, "scandir %s", dirp);

	if(numcounter==0 || numcounter > MAXCOUNTER)
	{
		printf("Invalid counter count\n");
		exit(1);
	}

	for(i=0; i<numcounter; i++)
	{
		sprintf(counter[i].filename, "%s/%s", dirp, namelist[i]->d_name);

		counter[i].gpio=atoi(namelist[i]->d_name+7);

		if(stat(counter[i].filename, &filestat)<0)
			err(1, "stat %s", counter[i].filename);

		counter[i].timestamp=filestat.st_mtim.tv_sec;

		counter[i].laststate=unknown;

		if(!(file=fopen(counter[i].filename, "r")))
			err(1, "open %s", counter[i].filename);

		if(fscanf(file, "%lu", &counter[i].value)!=1)
			err(1, "read %s", counter[i].filename);

		fclose(file);

		free(namelist[i]);
	}

	free(namelist);
}

void counter_free(void)
{
	FILE *file;
	int i;

	for(i=0; i<numcounter; i++)
	{
		if(!(file=fopen(counter[i].filename, "w")))
			err(1, "open %s", counter[i].filename);

		if(fprintf(file, "%lu", counter[i].value)<=0)
			err(1, "write %s", counter[i].filename);

		fclose(file);
	}
}

void counter_print(void)
{
	int i;

	printf("Current counters:\n");

	for(i=0; i<numcounter; i++)
		printf("Number: %d, gpio: %d, value: %lu, state: %d, timestamp: %ld, filename: %s\n", i, counter[i].gpio, counter[i].value, counter[i].laststate, counter[i].timestamp, counter[i].filename);
}

int main(void)
{
	sigset_t sigset;
	siginfo_t siginfo;

	counter_init("/root/watercounters");
	counter_print();

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

	counter_print();
	counter_free();

	printf("\nNormal exit\n");

	return 0;
}
