#pragma once

#define UTIL_FUNC(NAME) util_##NAME

#include <time.h>

time_t UTIL_FUNC(mktime)(struct tm * tmp);
