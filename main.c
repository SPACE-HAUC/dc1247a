#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
 
// Opens the specified I2C device.  Returns a non-negative file descriptor
// on success, or -1 on failure.
int open_i2c_device(const char * device)
{
  int fd = open(device, O_RDWR);
  if (fd == -1)
  {
    perror(device);
    return -1;
  }
  return fd;
}
 
// Sets the target, returning 0 on success and -1 on failure.
//
// For more information about what this command does, see the "Set Target"
// command in the "Command reference" section of the Jrk G2 user's guide.
int write_byte(int fd, uint8_t address, uint8_t reg, uint8_t cmd)
{
  uint8_t command[] = {reg, cmd};
  struct i2c_msg message = { address, 0, sizeof(command), command };
  struct i2c_rdwr_ioctl_data ioctl_data = { &message, 1 };
  int result = ioctl(fd, I2C_RDWR, &ioctl_data);
  if (result != 1)
  {
    perror("failed to set target");
    return -1;
  }
  return 1;
}
 
// Gets one or more variables from the Jrk (without clearing them).
// Returns 0 for success, -1 for failure.
int read_byte(int fd, uint8_t address, uint8_t reg, uint8_t *val)
{
  uint8_t command[] = { reg };
  struct i2c_msg messages[] = {
    { address, 0, sizeof(command), command },
    { address, I2C_M_RD, sizeof(uint8_t), val },
  };
  struct i2c_rdwr_ioctl_data ioctl_data = { messages, 2 };
  int result = ioctl(fd, I2C_RDWR, &ioctl_data);
  if (result != 2)
  {
    perror("failed to get variables");
    return -1;
  }
  return 1;
}

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
    int addr = argc == 3 ? atoi(argv[2]) : 0x45;
    printf("Opening bus: %d, device: 0x%x\n", bus, addr);
    char devname[128];
    snprintf(devname, sizeof(devname), "/dev/i2c-%d", bus);
    int dev = open_i2c_device(devname);
    if (dev < 0)
    {
        printf("Error opening device/bus, exiting.\n");
        exit(0);
    }
    printf("Resetting chip %d...\n", dev);
    fflush(stdout);
    uint8_t reg = 0x4;
    uint8_t cmd = 0b00100000;
    while((write_byte(dev, addr, reg, cmd) < 0) && (!done))
        sleep(1);
    if (done)
        exit(0);
    sleep(1);
    while((read_byte(dev, addr, reg, &cmd) < 0) && (!done))
        sleep(1);
    printf("Done: 0x%x\n", cmd);
    if (done)
        exit(0);
    
    return 0;
}