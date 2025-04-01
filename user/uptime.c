#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{

  if (argc != 1)
  {
      fprintf(2, "Usage: uptime\n");
      exit(1);
  }

  fprintf(1, "uptime: %d\n", uptime());
  
  exit(0);
}
