#include "ssss-split.h"
#include "stdio.h"

int main(int argc, char *argv[])
{
  char** shares;
  int t=3;
  int n=5;
  int debug=1;

  shamir_split(t,n,debug, &shares);

 
  for(int i=0; i<5; i++)
    printf("%s\n", shares[i]);
  return 0;
}