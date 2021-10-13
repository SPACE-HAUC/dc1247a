#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include "i2cbus/i2cbus.h"

volatile sig_atomic_t done = 0;
void sighandler(int sig)
{
    done = 1;
}

int main(int argc, char *argv[])
{
    if ((argc < 2) || (argc > 3))
    {
        printf("\nUsage: %s <I2C Bus> [Device Address, default 0x45]\n\n", argv[0]);
        exit(0);
    }
    int bus = atoi(argv[1]);
    int addr = argc == 3 ? strtol(argv[2], NULL, 16) : 0x45;
    printf("Opening bus: %d, device: 0x%x\n", bus, addr);
    i2cbus dev[1];
    if (i2cbus_open(dev, bus, addr) < 0)
    {
        printf("Error opening device/bus, exiting.\n");
        exit(0);
    }
    signal(SIGINT, sighandler);
    sleep(2);
// Step 1: Reset chip
    printf("Resetting chip... ");
    fflush(stdout);
    uint8_t cmd[2];
    cmd[0] = 0x4; // control register
    cmd[1] = 0b00100000; // reset command
    while((i2cbus_write(dev, &cmd, sizeof(cmd)) < 0) && (!done))
        sleep(1);
    if (done)
        goto cleanup;
    printf("Done\n");
    fflush(stdout);
// Step 2: Enable register input
    printf("Enabling register control... ");
    fflush(stdout);
    cmd[0] = 0x4; // control reg
    cmd[1] = 0x7; // update all voltages from register
    while((i2cbus_write(dev, &cmd, sizeof(cmd)) < 0) && (!done))
        sleep(1);
    if (done)
        goto cleanup;
    printf("Done\n");
    fflush(stdout);
// Step 3: Set VNEG to -8V
    printf("Setting VNEG to -8.0 V... ");
    fflush(stdout);
    cmd[0] = 0x1; // vneg reg
    cmd[1] = 0x88; // set voltage
    while((i2cbus_write(dev, &cmd, sizeof(cmd)) < 0) && (!done))
        sleep(1);
    if (done)
        goto cleanup;
    printf("Done\n");
    fflush(stdout);
// Step 4: Enable output
    printf("Enabling output... ");
    fflush(stdout);
    cmd[0] = 0x2; // output reg
    cmd[1] = 0x3; // enable all outputs
    while((i2cbus_write(dev, &cmd, sizeof(cmd)) < 0) && (!done))
        sleep(1);
    if (done)
        goto cleanup;
    printf("Done\n");
    fflush(stdout);
// Step 5: Parse inputs
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
        cmd[0] = 0x1;
        cmd[1] = (uint8_t)((vin + 1.2) / -0.05);
        printf("Voltage: %f V, Command: 0x%x\n", vin, cmd[1]);
        fflush(stdout);
        while((i2cbus_write(dev, &cmd, sizeof(cmd)) < 0) && (!done))
            usleep(100000);
    }
cleanup:
    i2cbus_close(dev);
    return 0;
}