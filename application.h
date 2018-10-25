#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "dataLink.h"

struct application{
  int seriesPortFd;
  int status;
};

enum status{
  TRANSMITTER = 0,
  RECEIVER = 0
};
