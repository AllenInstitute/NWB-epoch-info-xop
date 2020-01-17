#include <stdio.h>

/* Helper program to get the libclang API version */

#include <clang-c/Index.h>

int main(int argc, char** argv)
{
  printf("%s", CINDEX_VERSION_STRING);

  return 0;
}
