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

// https://github.com/unwireddevices/gpio-irq-handler

#define SIG_GPIO_IRQ	42

#define GPIO_CHIP	0
#define MAXCOUNTER	20

#define THRESHOLDLOHI	3 //The threshold is the minimum time in seconds that could physically pass between the two states
#define THRESHOLDHILO	1 //Everything below that limit is considered as chatter

#define CONFIGDIR "/etc/watercounter"

const int debug=1;
const int lesswrites=1;

struct counter
{
	int gpio;
	unsigned long value; //litres
	time_t timestamp; //last state change
	char filename[PATH_MAX];
	char name[NAME_MAX];
	int thresholdbypass; //whether we have detected a chatter after last value update, so we should ignore the following state change and ignore the threshold as we had already waited enough
	time_t lastsave; //last save of the value
} counter[MAXCOUNTER];

int numcounter=0;
int gpio_base;

void run_scripts(int i)
{
	static char cmd[256+PATH_MAX]={'\0'};
	snprintf(cmd, sizeof(cmd)-1, "test -d /etc/watercounter/actions.d/counter/ && for script in /etc/watercounter/actions.d/counter/*; do $script %s %lu ; done", counter[i].name, counter[i].value);
	if(!fork()) //child
	{
		exit(system(cmd));
	}
}

void counter_save(int i, time_t curtime, int writefile)
{
	FILE *file;
	static char *hostname=NULL;

	if(!hostname)
	{
		hostname=getenv("COLLECTD_HOSTNAME");
		if(!hostname)
			hostname==getenv("HOSTNAME");
		if(!hostname)
			hostname="localhost";
	}

	if(debug)
	{
		fprintf(stderr, "counter: %s, value: %lu\n", counter[i].name, counter[i].value);
		fflush(stderr);
	}

	if(writefile)
	{
		if(!(file=fopen(counter[i].filename, "w")))
			err(1, "open %s", counter[i].filename);

		if(fprintf(file, "%lu\n", counter[i].value)<=0)
			err(1, "write %s", counter[i].filename);

		fclose(file);
	}

	printf("PUTVAL %s/exec-watercounter/counter-%s interval=%ld %ld:%lu\n", hostname, counter[i].name, curtime-counter[i].lastsave, curtime, counter[i].value);
	printf("PUTVAL %s/exec-watercounter/gauge-%s interval=%ld %ld:%.3f\n", hostname, counter[i].name, curtime-counter[i].lastsave, curtime, 0.001*counter[i].value);
	fflush(stdout);

	run_scripts(i);

	counter[i].lastsave=curtime;
}

void counter_free(void)
{
	time_t curtime=time(NULL);
	int i;

	if(lesswrites)
		for(i=0; i<numcounter; i++)
			counter_save(i, curtime, 1);
}

void counter_update(int gpio, int val)
{
	int i;
	time_t curtime=time(NULL);
	unsigned char lastdigit;

	if(debug)
		fprintf(stderr, "gpio: %d, state: %d", gpio, val);

	for(i=0; i<numcounter && counter[i].gpio!=gpio; i++);

	if(i==numcounter)
	{
		fprintf(stderr, "\nWarning: Unknown gpio pin %d\n", gpio);
		fflush(stderr);
		return;
	}

	if(debug)
	{
		fprintf(stderr, " time: %ld, bypass: %d\n", curtime-counter[i].timestamp, counter[i].thresholdbypass);
		fflush(stderr);
	}

	switch(val)
	{
		case 1: //lo-hi change
			if(curtime-counter[i].timestamp>=THRESHOLDLOHI || counter[i].thresholdbypass)
			{
				if(counter[i].thresholdbypass==0)
				{
					lastdigit=counter[i].value-counter[i].value/10*10;
					if(lastdigit < 7) //allow unaligned initial value
					{
						counter[i].value+=7-lastdigit;
						counter_save(i, curtime, !lesswrites);
					}
				}
				counter[i].thresholdbypass=0;
			}
			else	//ignoring event because of rate limit, the water could not flow that fast; it is a way to filter out the chatter
				counter[i].thresholdbypass=1;		//ignore the threshold next time as we had already waited enough previously
			break;
		case 0: //hi-lo change
			if(curtime-counter[i].timestamp>=THRESHOLDHILO || counter[i].thresholdbypass)
			{
				if(counter[i].thresholdbypass==0)
				{
					lastdigit=counter[i].value-counter[i].value/10*10;
					if(lastdigit >= 7)
					{
						counter[i].value+=10-lastdigit;
						counter_save(i, curtime, !lesswrites);
					}
				}
				counter[i].thresholdbypass=0;
			}
			else
				counter[i].thresholdbypass=1;
			break;
		case 2048: //commit update
			counter_save(i, curtime, !lesswrites);
			break;
		default:
			counter[i].value+=val-2048;
			counter[i].thresholdbypass=0;
	}

	counter[i].timestamp=curtime;
}

void irq_handler(int n, siginfo_t *info, void *unused)
{
	counter_update(info->si_int >> 24, info->si_int & 0x0FFF);
}

void gpio_init(void)
{
	FILE *file;
	char tmpstr[256];
	int i;
	struct sigaction sig;

	if(!numcounter)
		err(1, "Counters not initialized");

	sprintf(tmpstr, "/sys/class/gpio/gpiochip%d/base", GPIO_CHIP);

	if(!(file=fopen(tmpstr, "r")))
		err(1, "Unable to open %s", tmpstr);

	if(fscanf(file, "%d", &gpio_base)!=1)
		err(1, "Unable to read %s", tmpstr);

	fclose(file);


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

	fprintf(file, "?\n");
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
	fprintf(file, "?\n");

	fclose(file);

	sig.sa_handler=SIG_DFL;
	sig.sa_flags = SA_SIGINFO | SA_NODEFER;
	if(sigaction(SIG_GPIO_IRQ, &sig, NULL)<0)
		err(1, "Unable to clear interrupt handler");
}

// The configuration file for every counter should be named with the following format:
// /etc/watercounter/counter${GPIO}_${NAME}
// where ${GPIO} is the gpio pin number and the ${NAME} is a human-readable name (may not end on "-opkg"). The name is optional.
// If you are running the program as a collectd exec plugin, make sure the files are readable and writable by the user you've configured in the collectd settings for the plugin.
int namefilter(const struct dirent *de)
{
	if(strlen(de->d_name)<8 || strncmp(de->d_name, "counter", 7) || strstr(de->d_name+7, "-opkg"))
		return 0;
	return de->d_name[7]>='0' && de->d_name[7]<='9' && (de->d_type==DT_LNK || de->d_type==DT_REG || de->d_type==DT_UNKNOWN);
}

void counter_init(const char *dirp)
{
	struct dirent **namelist;
	struct stat filestat;
	FILE *file;
	int i;
	time_t curtime=time(NULL);
	unsigned char lastdigit;
	char *delim;

	if((numcounter=scandir(dirp, &namelist, namefilter, alphasort))<0)
		err(1, "scandir %s", dirp);

	if(numcounter==0)
	{
		fprintf(stderr, "No counters configured\n");
		exit(1);
	}
	else if(numcounter > MAXCOUNTER)
	{
		fprintf(stderr, "Counter limit exceeded\n");
		exit(1);
	}

	for(i=0; i<numcounter; i++)
	{
		sprintf(counter[i].filename, "%s/%s", dirp, namelist[i]->d_name);

		counter[i].gpio=atoi(namelist[i]->d_name+7);

		if((delim=strchr(namelist[i]->d_name+8, '_')))
			strcpy(counter[i].name, delim+1);

		if(!delim || !counter[i].name[0])
			sprintf(counter[i].name, "%d", counter[i].gpio);

		if(stat(counter[i].filename, &filestat)<0)
			err(1, "stat %s", counter[i].filename);

		counter[i].timestamp=filestat.st_mtim.tv_sec;
		if(counter[i].timestamp>curtime)
		{
			fprintf(stderr, "Warning: %s: file modification time is in future\n");
			fflush(stderr);
			counter[i].timestamp=curtime;
		}

		counter[i].lastsave=counter[i].timestamp;
		counter[i].thresholdbypass=0;

		if(!(file=fopen(counter[i].filename, "r")))
			err(1, "open %s", counter[i].filename);

		if(fscanf(file, "%lu", &counter[i].value)!=1)
			err(1, "read %s", counter[i].filename);

		fclose(file);

		free(namelist[i]);
	}

	free(namelist);
}

void counter_print(void)
{
	int i;

	fprintf(stderr, "Current counters:\n");

	for(i=0; i<numcounter; i++)
		fprintf(stderr, "Index: %d, gpio: %d, name: %s, value: %lu, bypass: %d, timestamp: %ld, filename: %s\n", i, counter[i].gpio, counter[i].name, counter[i].value, counter[i].thresholdbypass, counter[i].timestamp, counter[i].filename);
	fflush(stderr);
}

int remote_update(pid_t pid, unsigned int gpio, long valuediff, int commit)
{
	int ret;
	int remainder=0;

	if(valuediff>2047)
	{
		remainder=valuediff-2047;
		valuediff=2047;
	}
	else if(valuediff<-2046)
	{
		remainder=valuediff+2046;
		valuediff=-2046;
	}

	union sigval sv={.sival_int=gpio<<24|(valuediff+2048)};

	ret=sigqueue(pid, SIG_GPIO_IRQ, sv);

	if(remainder)
	{
		ret=remote_update(pid, gpio, remainder, 0);
	}

	if(commit)
	{
		sv.sival_int=gpio<<24|2048;
		ret=sigqueue(pid, SIG_GPIO_IRQ, sv);
	}

	return ret;
}

int main(int argc, char **argv)
{
	if(argc==4)
		return remote_update((pid_t)atoi(argv[1]), atoi(argv[2]), atol(argv[3]), 1);

	sigset_t sigset;
	siginfo_t siginfo={0};

	counter_init(argc>1?argv[1]:CONFIGDIR);
	if(debug)
		counter_print();

	gpio_init();

	signal(SIGCHLD, SIG_IGN);

	sigemptyset(&sigset);
	sigaddset(&sigset, SIGINT);
	sigaddset(&sigset, SIGTERM);
	sigprocmask(SIG_BLOCK, &sigset, NULL);

	while(siginfo.si_signo != SIGINT && siginfo.si_signo != SIGTERM)
		sigwaitinfo(&sigset, &siginfo);

	gpio_free();

	if(debug)
		counter_print();

	counter_free();

	return 0;
}

