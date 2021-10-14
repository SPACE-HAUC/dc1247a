#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>

volatile sig_atomic_t done = 0;
void sighandler(int sig)
{
    done = 1;
}

int main(int argc, char *argv[])
{
    signal(SIGINT, sighandler);
    // Step 1: Reset chip
    printf("Resetting chip... ");
    char status = system("./test_bias.py -r") >> 8;
    printf("Done: %d\n", status);
    fflush(stdout);
    if (status < 0)
        exit(status);
    // Step 2: Parse inputs
    while (!done)
    {
        float vin = 0;
        printf("Enter voltage: ");
        int ret = scanf(" %f", &vin);
        if (ret == EOF)
        {
            break;
        }
        if (vin < -13.95f || vin > -1.2f)
            continue;
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "./test_bias.py -s %.2f", vin);
        status = system(cmd) >> 8;
        if (status < 0)
            printf("Could not set voltage: %d\n", status);
    }
    return 0;
}