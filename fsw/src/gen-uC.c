/**
 * @file
 *
 * @brief Generic uC Driver Implementation
 *
 * @ingroup I2CMicroController
 */

#include "gen-uC.h"

static const char bus_path[] = "/dev/i2c-2";

static int uC_ioctl(i2c_dev *dev, ioctl_command_t command, void *arg);

int uC_set_bytes(uint16_t chip_address, uint8_t **val, int numBytes){

  int fd;
  int rv;

  if(chip_address == 0){
    chip_address = (uint16_t) UC_ADDRESS;
  }

  uint8_t writebuff[numBytes];

  for(int i = 0; i<numBytes; i++){
    writebuff[i] = (*val)[i];
  }

  i2c_msg msgs[] = {{
    .addr = chip_address,
    .flags = 0,
    .buf = writebuff,
    .len = numBytes,
  }};
  struct i2c_rdwr_ioctl_data payload = {
    .msgs = msgs,
    .nmsgs = sizeof(msgs)/sizeof(msgs[0]),
  };

  fd = open(&bus_path[0], O_RDWR);
  if (fd < 0) {
    printf("Couldn't open bus...\n");
    return 1;
  }

  rv = ioctl(fd, I2C_RDWR, &payload);
  if (rv < 0) {
    perror("ioctl failed");
  }
  close(fd);

  return rv;
}

int i2c_dev_register_uC(const char *bus_path, const char *dev_path){
  i2c_dev *dev;

  dev = i2c_dev_alloc_and_init(sizeof(*dev), bus_path, UC_ADDRESS);
  if (dev == NULL) {
    return -1;
  }

  dev->ioctl = uC_ioctl;

  return i2c_dev_register(dev, dev_path);
}

static int uC_ioctl(i2c_dev *dev, ioctl_command_t command, void *arg){
  int err;

  // Variables for the Send test
  int numBytes = 3;
  uint8_t *val;

  switch (command) {
    case UC_SEND_TEST:

      val = NULL;
      val = malloc(numBytes * sizeof(uint8_t));

      val[0] = 0x03;
      val[1] = 0x06;
      val[2] = 0x09;

      err = uC_set_bytes(UC_ADDRESS, &val, numBytes); //Send 0x03, 0x06 and 0x09 to the uC default address
      break;

    default:
      err = -ENOTTY;
      break;
  }

  return err;
}

int uC_send_test(int fd){
  return ioctl(fd, UC_SEND_TEST, NULL);
}
