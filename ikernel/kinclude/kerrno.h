#ifndef KERRNO_H_
#define KERRNO_H_

#include <kinclude/ktypes.h>

#define EPERM		1	/* Operation not permitted */
#define ENOENT		2	/* No such file or directory */
#define ESRCH		3	/* No such process */
#define EINTR		4	/* Interrupted system call */
#define EIO		5	/* I/O error */
#define ENXIO		6	/* No such device or address */
#define E2BIG		7	/* Argument list too long */
#define ENOEXEC		8	/* Exec format error */
#define EBADF		9	/* Bad file number */
#define ECHILD		10	/* No child processes */
#define EAGAIN		11	/* Try again */
#define ENOMEM		12	/* Out of memory */
#define EACCES		13	/* Permission denied */
#define EFAULT		14	/* Bad address */
#define EBUSY		16	/* Device or resource busy */
#define EEXIST		17	/* File exists */
#define EINVAL		22	/* Invalid argument */
#define ENOSYS		38	/* Invalid system call number */
#define ERESTARTSYS	512

#define MAX_ERRNO 4095
#define IS_ERR_VALUE(x) unlikely((unsigned long)(x) >= (unsigned long)-MAX_ERRNO)

static inline bool IS_ERR(const void *ptr)
{
	return IS_ERR_VALUE((unsigned long)ptr);
}

static inline long PTR_ERR(const void *ptr)
{
	return (long) ptr;
}

static inline void *ERR_PTR(long error)
{
	return (void *) error;
}

#endif // KERRNO_H_
