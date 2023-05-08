/*
 *  ssss version 0.5  -  Copyright 2005,2006 B. Poettering
 * 
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307 USA
 */

/*
 * http://point-at-infinity.org/ssss/
 *
 * This is an implementation of Shamir's Secret Sharing Scheme. See 
 * the project's homepage http://point-at-infinity.org/ssss/ for more 
 * information on this topic.
 *
 * This code links against the GNU multiprecision library "libgmp".
 * I compiled the code successfully with gmp 4.1.4.
 * You will need a system that has a /dev/random entropy source.
 *
 * Compile with 
 * "gcc -O2 -lgmp -o ssss-split ssss.c && ln ssss-split ssss-combine"
 *
 * Compile with -DNOMLOCK to obtain a version without memory locking.
 *
 * Report bugs to: ssss AT point-at-infinity.org
 *  
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <termios.h>
#include <sys/mman.h>

#include <gmp.h>
#include "ssss-split.h"

#define VERSION "0.5"
#define RANDOM_SOURCE "/dev/random"
#define MAXDEGREE 1024
#define MAXTOKENLEN 128

/* coefficients of some irreducible polynomials over GF(2) */
static const unsigned char irred_coeff[] = {
  4,3,1,5,3,1,4,3,1,7,3,2,5,4,3,5,3,2,7,4,2,4,3,1,10,9,3,9,4,2,7,6,2,10,9,
  6,4,3,1,5,4,3,4,3,1,7,2,1,5,3,2,7,4,2,6,3,2,5,3,2,15,3,2,11,3,2,9,8,7,7,
  2,1,5,3,2,9,3,1,7,3,1,9,8,3,9,4,2,8,5,3,15,14,10,10,5,2,9,6,2,9,3,2,9,5,
  2,11,10,1,7,3,2,11,2,1,9,7,4,4,3,1,8,3,1,7,4,1,7,2,1,13,11,6,5,3,2,7,3,2,
  8,7,5,12,3,2,13,10,6,5,3,2,5,3,2,9,5,2,9,7,2,13,4,3,4,3,1,11,6,4,18,9,6,
  19,18,13,11,3,2,15,9,6,4,3,1,16,5,2,15,14,6,8,5,2,15,11,2,11,6,2,7,5,3,8,
  3,1,19,16,9,11,9,6,15,7,6,13,4,3,14,13,3,13,6,3,9,5,2,19,13,6,19,10,3,11,
  6,5,9,2,1,14,3,2,13,3,1,7,5,4,11,9,8,11,6,5,23,16,9,19,14,6,23,10,2,8,3,
  2,5,4,3,9,6,4,4,3,2,13,8,6,13,11,1,13,10,3,11,6,5,19,17,4,15,14,7,13,9,6,
  9,7,3,9,7,1,14,3,2,11,8,2,11,6,4,13,5,2,11,5,1,11,4,1,19,10,3,21,10,6,13,
  3,1,15,7,5,19,18,10,7,5,3,12,7,2,7,5,1,14,9,6,10,3,2,15,13,12,12,11,9,16,
  9,7,12,9,3,9,5,2,17,10,6,24,9,3,17,15,13,5,4,3,19,17,8,15,6,3,19,6,1 };

int opt_showversion = 0;
int opt_help = 0;
int opt_quiet = 0;
int opt_QUIET = 0;
int opt_hex = 0;
int opt_diffusion = 1;
int opt_security = 0;
int opt_threshold = -1;
int opt_number = -1;
char *opt_token = NULL;

unsigned int degree;
mpz_t poly;
int cprng;
struct termios echo_orig, echo_off;

#define mpz_lshift(A, B, l) mpz_mul_2exp(A, B, l)
#define mpz_sizeinbits(A) (mpz_cmp_ui(A, 0) ? mpz_sizeinbase(A, 2) : 0)

/* emergency abort and warning functions */

void fatal(char *msg)
{
  tcsetattr(0, TCSANOW, &echo_orig);
  fprintf(stderr, "%sFATAL: %s.\n", isatty(2) ? "\a" : "", msg);
  exit(1);
}

void warning(char *msg)
{
  if (! opt_QUIET)
    fprintf(stderr, "%sWARNING: %s.\n", isatty(2) ? "\a" : "", msg);
}

/* field arithmetic routines */

int field_size_valid(int deg)
{
  return (deg >= 8) && (deg <= MAXDEGREE) && (deg % 8 == 0);
}

/* initialize 'poly' to a bitfield representing the coefficients of an
   irreducible polynomial of degree 'deg' */

void field_init(int deg)
{
  assert(field_size_valid(deg));
  mpz_init_set_ui(poly, 0);
  mpz_setbit(poly, deg);
  mpz_setbit(poly, irred_coeff[3 * (deg / 8 - 1) + 0]);
  mpz_setbit(poly, irred_coeff[3 * (deg / 8 - 1) + 1]);
  mpz_setbit(poly, irred_coeff[3 * (deg / 8 - 1) + 2]);
  mpz_setbit(poly, 0);
  degree = deg;
}

void field_deinit(void)
{
  mpz_clear(poly);
}

/* I/O routines for GF(2^deg) field elements */

void field_import(mpz_t x, const char *s, int hexmode)
{
  if (hexmode) {
    if (strlen(s) > degree / 4)
      fatal("input string too long");
    if (strlen(s) < degree / 4)
      warning("input string too short, adding null padding on the left");
    if (mpz_set_str(x, s, 16) || (mpz_cmp_ui(x, 0) < 0))
      fatal("invalid syntax");
  }
  else {
    int i;
    int warn = 0;
    if (strlen(s) > degree / 8)
      fatal("input string too long");
    for(i = strlen(s) - 1; i >= 0; i--)
      warn = warn || (s[i] < 32) || (s[i] >= 127);
    if (warn)
      warning("binary data detected, use -x mode instead");
    mpz_import(x, strlen(s), 1, 1, 0, 0, s);
  }
}

void field_print(FILE* stream, const mpz_t x, int hexmode)
{
  int i;
  if (hexmode) {
    for(i = degree / 4 - mpz_sizeinbase(x, 16); i; i--)
      fprintf(stream, "0");
    mpz_out_str(stream, 16, x);
    fprintf(stream, "\n");
  }
  else {
    char buf[MAXDEGREE / 8 + 1];
    size_t t;
    unsigned int i;
    int printable, warn = 0;
    memset(buf, degree / 8 + 1, 0);
    mpz_export(buf, &t, 1, 1, 0, 0, x);
    for(i = 0; i < t; i++) {
      printable = (buf[i] >= 32) && (buf[i] < 127);
      warn = warn || ! printable;
      fprintf(stream, "%c", printable ? buf[i] : '.');
    }
    fprintf(stream, "\n");
    if (warn)
      warning("binary data detected, use -x mode instead");
  }
}



/* basic field arithmetic in GF(2^deg) */

void field_add(mpz_t z, const mpz_t x, const mpz_t y)
{
  mpz_xor(z, x, y);
}

void field_mult(mpz_t z, const mpz_t x, const mpz_t y)
{
  mpz_t b;
  unsigned int i;
  assert(z != y);
  mpz_init_set(b, x);
  if (mpz_tstbit(y, 0))
    mpz_set(z, b);
  else
    mpz_set_ui(z, 0);
  for(i = 1; i < degree; i++) {
    mpz_lshift(b, b, 1);
    if (mpz_tstbit(b, degree))
      mpz_xor(b, b, poly);
    if (mpz_tstbit(y, i))
      mpz_xor(z, z, b);
  }
  mpz_clear(b);
}

void field_invert(mpz_t z, const mpz_t x)
{
  mpz_t u, v, g, h;
  int i;
  assert(mpz_cmp_ui(x, 0));
  mpz_init_set(u, x);
  mpz_init_set(v, poly);
  mpz_init_set_ui(g, 0);
  mpz_set_ui(z, 1);
  mpz_init(h);
  while (mpz_cmp_ui(u, 1)) {
    i = mpz_sizeinbits(u) - mpz_sizeinbits(v);
    if (i < 0) {
      mpz_swap(u, v);
      mpz_swap(z, g);
      i = -i;
    }
    mpz_lshift(h, v, i);
    mpz_xor(u, u, h);
    mpz_lshift(h, g, i);
    mpz_xor(z, z, h);
  }
  mpz_clear(u); mpz_clear(v); mpz_clear(g); mpz_clear(h);
}

/* routines for the random number generator */

void cprng_init(void)
{
  if ((cprng = open(RANDOM_SOURCE, O_RDONLY)) < 0)
    fatal("couldn't open " RANDOM_SOURCE);
}

void cprng_deinit(void)
{
  if (close(cprng) < 0)
    fatal("couldn't close " RANDOM_SOURCE);
}

void cprng_read(mpz_t x)
{
  char buf[MAXDEGREE / 8];
  unsigned int count;
  int i;
  for(count = 0; count < degree / 8; count += i)
    if ((i = read(cprng, buf + count, degree / 8 - count)) < 0) {
      close(cprng);
      fatal("couldn't read from " RANDOM_SOURCE);
    }
  mpz_import(x, degree / 8, 1, 1, 0, 0, buf);
}

/* a 64 bit pseudo random permutation (based on the XTEA cipher) */

void encipher_block(uint32_t *v) 
{
  uint32_t sum = 0, delta = 0x9E3779B9;
  int i;
  for(i = 0; i < 32; i++) {
    v[0] += (((v[1] << 4) ^ (v[1] >> 5)) + v[1]) ^ sum;
    sum += delta;
    v[1] += (((v[0] << 4) ^ (v[0] >> 5)) + v[0]) ^ sum;
  }
}

void decipher_block(uint32_t *v)
{
  uint32_t sum = 0xC6EF3720, delta = 0x9E3779B9;
  int i;
  for(i = 0; i < 32; i++) {
    v[1] -= ((v[0] << 4 ^ v[0] >> 5) + v[0]) ^ sum;
    sum -= delta;
    v[0] -= ((v[1] << 4 ^ v[1] >> 5) + v[1]) ^ sum;
  }
}

void encode_slice(uint8_t *data, int idx, int len, 
		  void (*process_block)(uint32_t*))
{
  uint32_t v[2];
  int i;
  for(i = 0; i < 2; i++)
    v[i] = data[(idx + 4 * i) % len] << 24 | 
      data[(idx + 4 * i + 1) % len] << 16 | 
      data[(idx + 4 * i + 2) % len] << 8 | data[(idx + 4 * i + 3) % len];
  process_block(v);
  for(i = 0; i < 2; i++) {
    data[(idx + 4 * i + 0) % len] = v[i] >> 24;
    data[(idx + 4 * i + 1) % len] = (v[i] >> 16) & 0xff;
    data[(idx + 4 * i + 2) % len] = (v[i] >> 8) & 0xff;
    data[(idx + 4 * i + 3) % len] = v[i] & 0xff;
  }
}

enum encdec {ENCODE, DECODE};

void encode_mpz(mpz_t x, enum encdec encdecmode)
{
  uint8_t v[(MAXDEGREE + 8) / 16 * 2];
  size_t t;
  int i;
  memset(v, 0, (degree + 8) / 16 * 2);
  mpz_export(v, &t, -1, 2, 1, 0, x);
  if (degree % 16 == 8)
    v[degree / 8 - 1] = v[degree / 8];
  if (encdecmode == ENCODE)             /* 40 rounds are more than enough!*/
    for(i = 0; i < 40 * ((int)degree / 8); i += 2)
      encode_slice(v, i, degree / 8, encipher_block);
  else
    for(i = 40 * (degree / 8) - 2; i >= 0; i -= 2)
      encode_slice(v, i, degree / 8, decipher_block);
  if (degree % 16 == 8) {
    v[degree / 8] = v[degree / 8 - 1];
    v[degree / 8 - 1] = 0;
  }
  mpz_import(x, (degree + 8) / 16, -1, 2, 1, 0, v);
  assert(mpz_sizeinbits(x) <= degree);
}

/* evaluate polynomials efficiently */

void horner(int n, mpz_t y, const mpz_t x, const mpz_t coeff[])
{
  int i;
  mpz_set(y, x);
  for(i = n - 1; i; i--) {
    field_add(y, y, coeff[i]);
    field_mult(y, y, x);
  }
  field_add(y, y, coeff[0]);
}

/* calculate the secret from a set of shares solving a linear equation system */

#define MPZ_SWAP(A, B) \
  do { mpz_set(h, A); mpz_set(A, B); mpz_set(B, h); } while(0)

/* Prompt for a secret, generate shares for it */

void split(int debug, char* shares[], char* input){
  
  unsigned int fmt_len;
  mpz_t x, y, coeff[opt_threshold];
  int deg, i;

  for(fmt_len = 1, i = opt_number; i >= 10; i /= 10, fmt_len++);

  tcsetattr(0, TCSANOW, &echo_off);
  tcsetattr(0, TCSANOW, &echo_orig);
  input[strcspn(input, "\r\n")] = '\0';

  if (! opt_security) {
    opt_security = opt_hex ? 4 * ((strlen(input) + 1) & ~1): 8 * strlen(input);
    if (! field_size_valid(opt_security))
      fatal("security level invalid (secret too long?)");
    if (! opt_quiet)
      fprintf(stderr, "Using a %d bit security level.\n", opt_security);
  }

  field_init(opt_security);

  mpz_init(coeff[0]);
  field_import(coeff[0], input, opt_hex);

  if (opt_diffusion) {
    if (degree >= 64)
      encode_mpz(coeff[0], ENCODE);
    else 
      warning("security level too small for the diffusion layer");
  }
  
  cprng_init();


  for(i = 1; i < opt_threshold; i++) {
    mpz_init(coeff[i]);
    cprng_read(coeff[i]);
  }
  cprng_deinit();

  mpz_init(x);
  mpz_init(y);

  // Array to store the shares
  int full_size;
  int j;

  for(i = 0; i < opt_number; i++) {
    
    mpz_set_ui(x, i + 1);
    horner(opt_threshold, y, x, (const mpz_t*)coeff);
    if (opt_token)
      printf("%s-", opt_token);
    if(debug){
      printf("%0*d-", fmt_len, i + 1);
      field_print(stdout, y, 1);  // Display the share //void field_print(FILE* stream, const mpz_t x, int hexmode)
    }
    

    //copy to array
    char* share_id;
    char* share_value;
    char* share_value_min;
    char* share;

    share_id = (char*)malloc((fmt_len + 2) * sizeof(char)); 
    sprintf(share_id, "%0*d-", fmt_len, i + 1);

    char leading_zeros[100] = "";
    for(j = degree / 4 - mpz_sizeinbase(y, 16); j; j--)
      strcat(leading_zeros, "0");
    

    share_value_min = mpz_get_str(NULL,16,y);
    share_value = (char*) malloc(strlen(share_value_min) + strlen(leading_zeros) + 1);
    memset(share_value, '0', strlen(leading_zeros));
    strcpy(share_value, leading_zeros);
    strcat(share_value, share_value_min);
    //share_value = mpz_get_str(NULL,16,y);

    full_size = strlen( share_id ) + strlen( share_value ) + 1;
    share = (char *) malloc( full_size );

    strcpy( share, share_id );
    strcat( share, share_value);
    shares[i] = share;
  }
  mpz_clear(x);
  mpz_clear(y);

  for(i = 0; i < opt_threshold; i++)
    mpz_clear(coeff[i]);
  field_deinit();

  // Access the shares from the array if required
  /*if(debug){
    for(i = 0; i < opt_number; i++) {
      printf("Share %d = %s\n", i + 1, shares[i]);
    } 
  }*/
}
/***********************************************************************************************************/

int shamir_split(int t, int n, int debug, char*** shares, char* input){
  int i;

  if (getuid() != geteuid())
    seteuid(getuid());

  tcgetattr(0, &echo_orig);
  echo_off = echo_orig;
  echo_off.c_lflag &= ~ECHO;

  opt_threshold = t;
  opt_number = n;

  if (opt_threshold < 2)
      fatal("invalid parameters: invalid threshold value");

  if (opt_number < opt_threshold)
      fatal("invalid parameters: number of shares smaller than threshold");

  *shares = (char**) malloc(opt_number*sizeof(char*)); 
  split(debug, *shares, input);
  
  return 0;
}
