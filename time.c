#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#define __DEBUG
#ifdef __DEBUG
	#define DEBUG(format,...) printf("FILE: "__FILE__", LINE: %d: "format"/n", __LINE__, ##__VA_ARGS__)
#else
	#define DEBUG(info)
#endif

#define CLOCKID CLOCK_REALTIME
#define SIG SIGUSR1
#define TIMER_INTERVAL 5
timer_t timerid;

/*
static void handler(int sig, siginfo_t *si, void *uc)
{
    if(si->si_value.sival_ptr != &timerid){
        printf("Stray signal\n");
    } else {
        printf("Caught signal %d from timer\n", sig);
    }
}

void set(struct itimerspec *its, int second, int nsecond){
    its->it_value.tv_sec = second;
    its->it_value.tv_nsec = nsecond;
    its->it_interval.tv_sec = its->it_value.tv_sec;
    its->it_interval.tv_nsec = its->it_value.tv_nsec;
}
*/

//定时器处理函数
void time_handler(int sig, siginfo_t *si, void *uc)
{
    if(si->si_value.sival_ptr != &timerid){
        printf("Stray signal\n");
    } else {
        printf("Caught signal %d from timer\n", sig);
    }
	/*
    if(si->si_value.sival_ptr == &timerid){
		DEBUG("timer handler is running");
    }
	*/
}

//定时器设定时间函数
void time_set(timer_t timerid, int second, int nsecond){
	struct itimerspec its;
    its.it_value.tv_sec = second;
    its.it_value.tv_nsec = nsecond;
    its.it_interval.tv_sec = its.it_value.tv_sec;
    its.it_interval.tv_nsec = its.it_value.tv_nsec;
    timer_settime(timerid, 0, &its, NULL);
}

//定时器初始化
void time_init() {
    struct sigevent sev;
    long long freq_nanosecs;
    sigset_t mask;
    struct sigaction sa;

    DEBUG("Establishing handler for signal %d\n", SIG);
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = time_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIG, &sa, NULL);

    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIG;
    sev.sigev_value.sival_ptr = &timerid;
    timer_create(CLOCKID, &sev, &timerid);
	time_set(timerid, TIMER_INTERVAL, 0);
}

int main(int argc, char *argv[])
{
	time_init();
/*
    struct sigevent sev;
    struct itimerspec its;
    long long freq_nanosecs;
    sigset_t mask;
    struct sigaction sa;

    printf("Establishing handler for signal %d\n", SIG);
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIG, &sa, NULL);

    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIG;
    sev.sigev_value.sival_ptr = &timerid;
    timer_create(CLOCKID, &sev, &timerid);
    // Start the timer 
    set(&its, 5, 0);

    timer_settime(timerid, 0, &its, NULL);
	*/
	/*
    sleep(4);
    set(&its, 5, 0);
    timer_settime(timerid, 0, &its, NULL);
	*/


   while(1) {
	sleep(1);
   }
}

