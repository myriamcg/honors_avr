/*
   american fuzzy lop++ - a trivial program to test the build
   --------------------------------------------------------
   Originally written by Michal Zalewski
   Copyright 2014 Google Inc. All rights reserved.
   Copyright 2019-2024 AFLplusplus Project. All rights reserved.
   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at:
     https://www.apache.org/licenses/LICENSE-2.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef TEST_SHARED_OBJECT
  #define main main_exported
#endif

int main(int argc, char **argv) {

  int   fd = 0, cnt;
  char  buff[8];
  char *buf = buff;

  // we support command line parameter and stdin
  if (argc == 2) {

    buf = argv[1];

  } else {

    if (argc >= 3 && strcmp(argv[1], "-f") == 0) {

      if ((fd = open(argv[2], O_RDONLY)) < 0) {

        fprintf(stderr, "Error: unable to open %s\n", argv[2]);
        exit(-1);

      }

    }

    if ((cnt = read(fd, buf, sizeof(buf) - 1)) < 1) {

      printf("Hum?\n");
#ifdef EXIT_AT_END
      exit(1);
#else
      return 1;
#endif

    }

    buf[cnt] = 0;

  }

  if (getenv("AFL_DEBUG")) fprintf(stderr, "test-instr: %s\n", buf);

  // we support three input cases (plus a 4th if stdin is used but there is no
  // input)
  switch (buf[0]) {

    case '0':
      printf("Looks like a zero to me!\n");
      break;

    case '1':
      printf("Pretty sure that is a one!\n");
      break;

    default:
      printf("Neither one or zero? How quaint!\n");
      break;

  }

#ifdef EXIT_AT_END
  exit(0);
#endif

  return 0;

}

