
#pragma once

#include <signal.h>

#include <applibs/eventloop.h>

static volatile sig_atomic_t exitCode = 0;

void CloseFd(int fd, const char *fdName);