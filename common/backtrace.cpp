// Copyright 2023 Paweł Sacawa. All rights reserved.
#include <execinfo.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FRAMES 100

void system_die(const char *msg) {
  perror(msg);
  exit(EXIT_FAILURE);
}

static void dump_backtrace() {
  int nptrs;
  void *buffer[MAX_FRAMES];
  char **strings;

  nptrs = backtrace(buffer, MAX_FRAMES);
  printf("backtrace has %d frames\n", nptrs);
  strings = backtrace_symbols(buffer, nptrs);
  if (strings == NULL)
    system_die("backtrace_symbols");

  for (int i = 0; i != nptrs; ++i) {
    printf("%s\n", strings[i]);
  }
}

void segfault_handler(int signum, siginfo_t *info, void *data) {
  printf("%s\n", strsignal(signum));
  dump_backtrace();
  exit(EXIT_FAILURE);
}

void install_backtrace_signal_handler(int signum) {
  struct sigaction action;
  sigset_t mask;
  sigemptyset(&mask);
  if (sigaddset(&mask, signum) == -1)
    system_die("sigaddset");

  action.sa_flags = SA_SIGINFO;
  action.sa_mask = mask;
  action.sa_sigaction = segfault_handler;

  if (sigaction(signum, &action, NULL) < 0)
    system_die("sigaction");
  fprintf(stderr, "Installed backtrace signal handler on signal %s",
          strsignal(signum));
}
