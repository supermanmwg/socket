#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>

#define CLOCKID CLOCK_REALTIME
#define SIG SIGUSR1
timer_t timerid;


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

int main(int argc, char *argv[])
{
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
    /* Start the timer */
    set(&its, 5, 0);

    timer_settime(timerid, 0, &its, NULL);
    sleep(4);
    set(&its, 5, 0);
    timer_settime(timerid, 0, &its, NULL);


   while(1) {
	sleep(1);
   }
}

