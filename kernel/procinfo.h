#pragma once

#include "procstate.h"
#include "types.h"

#define PROCNAME_SIZE 16

struct procinfo
{
    uint64 pid;
    char name[PROCNAME_SIZE];
    enum procstate state;
    uint64 parent_pid;
};