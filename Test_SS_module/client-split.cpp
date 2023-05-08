#include "ssss-split.h"
#include "stdio.h"

#define MAXDEGREE 1024
#define MAXTOKENLEN 128
#define MAXLINELEN (MAXTOKENLEN + 1 + 10 + 1 + MAXDEGREE / 4 + 10)

int main(int argc, char *argv[])
{
  char** shares;
  int t=3;
  int n=5;
  int debug=0;
  int deg;

  char input[MAXLINELEN];

  printf("Generating shares using a (%d,%d) scheme with ", t, n);
  printf("dynamic");
  printf(" security level.\n");
  
  deg = MAXDEGREE;
  fprintf(stderr, "Enter the secret, ");
  fprintf(stderr, "at most %d ASCII characters: ", deg / 8);

  fgets(input, sizeof(input), stdin);

  shamir_split(t,n,debug, &shares, input);

  for(int i=0; i<5; i++)
    printf("%s\n", shares[i]);
  return 0;
}