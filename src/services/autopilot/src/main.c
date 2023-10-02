#include <stdio.h>
#include <syslog.h>
#include <signal.h>
#include <stdbool.h>

#define SERVICE_NAME "AUTO-PILOT-SERVICE"
static volatile bool run = true;

static void handle_sigint(int sig)
{
    syslog(LOG_NOTICE, "Caught signal %d\n", sig);
    run = false;
}

int main(int argc, char** argv)
{
    signal(SIGINT, handle_sigint);
    openlog(SERVICE_NAME, LOG_PID, LOG_DAEMON);
    syslog(LOG_NOTICE, "Start");

    while (run)
    {
        /** Loop */

    }

    syslog(LOG_NOTICE, "Exit");
    closelog();

    return 0;
}
