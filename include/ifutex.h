#ifndef IFUTEX_H_
#define IFUTEX_H_

#include <deftypei.h>

typedef ilib_uint32_t u32;

extern int futex_wait_zero(u32 *uaddr);
extern int futex_wake_one(u32 *uaddr);

#endif // IFUTEX_H_
