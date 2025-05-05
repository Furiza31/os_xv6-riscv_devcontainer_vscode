/*-
 * Random‑number routines taken from FreeBSD's libc, stripped to the
 * Park–Miller ("Minimal Standard") generator (USE_WEAK_SEEDING == 0).
 *
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 * Distributed under the 2‑clause BSD licence; see the original header.
 */

#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

//
// wrapper so that it's OK if main() does not call exit().
//
void
_main(void)
{
  extern int main(void);
  main();
  exit(0);
}

/* ------------------------------------------------------------------- */
/*  String / memory helpers (unchanged from the stock xv6 user library) */
/* ------------------------------------------------------------------- */

char*
strcpy(char *s, const char *t)
{
  char *os = s;
  while ((*s++ = *t++) != 0)
    ;
  return os;
}

int
strcmp(const char *p, const char *q)
{
  while (*p && *p == *q)
    p++, q++;
  return (uchar)*p - (uchar)*q;
}

uint
strlen(const char *s)
{
  uint n = 0;
  while (s[n])
    n++;
  return n;
}

void*
memset(void *dst, int c, uint n)
{
  char *d = (char *)dst;
  for (uint i = 0; i < n; i++)
    d[i] = c;
  return dst;
}

char*
strchr(const char *s, char c)
{
  for (; *s; s++)
    if (*s == c)
      return (char *)s;
  return 0;
}

char*
gets(char *buf, int max)
{
  int i = 0, cc;
  char c;

  while (i + 1 < max) {
    cc = read(0, &c, 1);
    if (cc < 1)
      break;
    buf[i++] = c;
    if (c == '\n' || c == '\r')
      break;
  }
  buf[i] = '\0';
  return buf;
}

int
stat(const char *n, struct stat *st)
{
  int fd = open(n, O_RDONLY);
  if (fd < 0)
    return -1;
  int r = fstat(fd, st);
  close(fd);
  return r;
}

int
atoi(const char *s)
{
  int n = 0;
  while ('0' <= *s && *s <= '9')
    n = n * 10 + *s++ - '0';
  return n;
}

void*
memmove(void *vdst, const void *vsrc, int n)
{
  char *dst = vdst;
  const char *src = vsrc;

  if (src > dst) {
    while (n-- > 0)
      *dst++ = *src++;
  } else {
    dst += n;
    src += n;
    while (n-- > 0)
      *--dst = *--src;
  }
  return vdst;
}

int
memcmp(const void *s1, const void *s2, uint n)
{
  const uchar *p1 = s1, *p2 = s2;
  while (n-- > 0) {
    if (*p1 != *p2)
      return *p1 - *p2;
    p1++; p2++;
  }
  return 0;
}

void*
memcpy(void *dst, const void *src, uint n)
{
  return memmove(dst, src, n);
}

#ifndef RAND_MAX
#define RAND_MAX 0x7ffffffd
#endif

static unsigned long next_rand = 2;

static int
do_rand(unsigned long *ctx)
{
  long hi = *ctx / 127773;
  long lo = *ctx % 127773;
  long x  = 16807 * lo - 2836 * hi;
  if (x < 0)
    x += 0x7fffffff;
  *ctx = (unsigned long)x;
  return x - 1;
}

int
rand(void)
{
  return do_rand(&next_rand);
}

void
srand(unsigned int seed)
{
  next_rand = (seed % 0x7ffffffe) + 1;
}

int
rand_r(unsigned int *state)
{
  unsigned long val = (*state % 0x7ffffffe) + 1;
  int r = do_rand(&val);
  *state = (unsigned int)(val - 1);
  return r;
}

int
abs(int x) {
  return x < 0 ? -x : x;
}