# 1 "y.tab.c"
# 1 "<built-in>"
# 1 "<command line>"
# 1 "y.tab.c"
# 55 "y.tab.c"
   enum pvs_yytokentype {
     AND = 258,
     EQUAL = 259,
     PLUS = 260,
     TIMES = 261,
     COMPOSE = 262,
     APPLY = 263,
     SUBSCRIPT = 264,
     IDENTIFIER = 265,
     FUNCTION = 266
   };
# 1 "pvsparse.y"

# 1 "/usr/include/stdlib.h" 1 3 4
# 25 "/usr/include/stdlib.h" 3 4
# 1 "/usr/include/features.h" 1 3 4
# 314 "/usr/include/features.h" 3 4
# 1 "/usr/include/sys/cdefs.h" 1 3 4
# 315 "/usr/include/features.h" 2 3 4
# 337 "/usr/include/features.h" 3 4
# 1 "/usr/include/gnu/stubs.h" 1 3 4
# 338 "/usr/include/features.h" 2 3 4
# 26 "/usr/include/stdlib.h" 2 3 4







# 1 "/usr/lib/gcc/i386-redhat-linux/3.4.4/include/stddef.h" 1 3 4
# 213 "/usr/lib/gcc/i386-redhat-linux/3.4.4/include/stddef.h" 3 4
typedef unsigned int size_t;
# 325 "/usr/lib/gcc/i386-redhat-linux/3.4.4/include/stddef.h" 3 4
typedef long int wchar_t;
# 34 "/usr/include/stdlib.h" 2 3 4


# 96 "/usr/include/stdlib.h" 3 4


typedef struct
  {
    int quot;
    int rem;
  } div_t;



typedef struct
  {
    long int quot;
    long int rem;
  } ldiv_t;



# 140 "/usr/include/stdlib.h" 3 4
extern size_t __ctype_get_mb_cur_max (void) __attribute__ ((__nothrow__));




extern double atof (__const char *__nptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));

extern int atoi (__const char *__nptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));

extern long int atol (__const char *__nptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));





__extension__ extern long long int atoll (__const char *__nptr)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));





extern double strtod (__const char *__restrict __nptr,
        char **__restrict __endptr) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));

# 181 "/usr/include/stdlib.h" 3 4


extern long int strtol (__const char *__restrict __nptr,
   char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));

extern unsigned long int strtoul (__const char *__restrict __nptr,
      char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));




__extension__
extern long long int strtoq (__const char *__restrict __nptr,
        char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));

__extension__
extern unsigned long long int strtouq (__const char *__restrict __nptr,
           char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));





__extension__
extern long long int strtoll (__const char *__restrict __nptr,
         char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));

__extension__
extern unsigned long long int strtoull (__const char *__restrict __nptr,
     char **__restrict __endptr, int __base)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));

# 277 "/usr/include/stdlib.h" 3 4
extern double __strtod_internal (__const char *__restrict __nptr,
     char **__restrict __endptr, int __group)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));
extern float __strtof_internal (__const char *__restrict __nptr,
    char **__restrict __endptr, int __group)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));
extern long double __strtold_internal (__const char *__restrict __nptr,
           char **__restrict __endptr,
           int __group) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));

extern long int __strtol_internal (__const char *__restrict __nptr,
       char **__restrict __endptr,
       int __base, int __group)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));



extern unsigned long int __strtoul_internal (__const char *__restrict __nptr,
          char **__restrict __endptr,
          int __base, int __group)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));




__extension__
extern long long int __strtoll_internal (__const char *__restrict __nptr,
      char **__restrict __endptr,
      int __base, int __group)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));



__extension__
extern unsigned long long int __strtoull_internal (__const char *
         __restrict __nptr,
         char **__restrict __endptr,
         int __base, int __group)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));
# 424 "/usr/include/stdlib.h" 3 4
extern char *l64a (long int __n) __attribute__ ((__nothrow__));


extern long int a64l (__const char *__s)
     __attribute__ ((__nothrow__)) __attribute__ ((__pure__)) __attribute__ ((__nonnull__ (1)));




# 1 "/usr/include/sys/types.h" 1 3 4
# 29 "/usr/include/sys/types.h" 3 4


# 1 "/usr/include/bits/types.h" 1 3 4
# 28 "/usr/include/bits/types.h" 3 4
# 1 "/usr/include/bits/wordsize.h" 1 3 4
# 29 "/usr/include/bits/types.h" 2 3 4


# 1 "/usr/lib/gcc/i386-redhat-linux/3.4.4/include/stddef.h" 1 3 4
# 32 "/usr/include/bits/types.h" 2 3 4


typedef unsigned char __u_char;
typedef unsigned short int __u_short;
typedef unsigned int __u_int;
typedef unsigned long int __u_long;


typedef signed char __int8_t;
typedef unsigned char __uint8_t;
typedef signed short int __int16_t;
typedef unsigned short int __uint16_t;
typedef signed int __int32_t;
typedef unsigned int __uint32_t;




__extension__ typedef signed long long int __int64_t;
__extension__ typedef unsigned long long int __uint64_t;







__extension__ typedef long long int __quad_t;
__extension__ typedef unsigned long long int __u_quad_t;
# 129 "/usr/include/bits/types.h" 3 4
# 1 "/usr/include/bits/typesizes.h" 1 3 4
# 130 "/usr/include/bits/types.h" 2 3 4






__extension__ typedef __u_quad_t __dev_t;
__extension__ typedef unsigned int __uid_t;
__extension__ typedef unsigned int __gid_t;
__extension__ typedef unsigned long int __ino_t;
__extension__ typedef __u_quad_t __ino64_t;
__extension__ typedef unsigned int __mode_t;
__extension__ typedef unsigned int __nlink_t;
__extension__ typedef long int __off_t;
__extension__ typedef __quad_t __off64_t;
__extension__ typedef int __pid_t;
__extension__ typedef struct { int __val[2]; } __fsid_t;
__extension__ typedef long int __clock_t;
__extension__ typedef unsigned long int __rlim_t;
__extension__ typedef __u_quad_t __rlim64_t;
__extension__ typedef unsigned int __id_t;
__extension__ typedef long int __time_t;
__extension__ typedef unsigned int __useconds_t;
__extension__ typedef long int __suseconds_t;

__extension__ typedef int __daddr_t;
__extension__ typedef long int __swblk_t;
__extension__ typedef int __key_t;


__extension__ typedef int __clockid_t;


__extension__ typedef int __timer_t;


__extension__ typedef long int __blksize_t;




__extension__ typedef long int __blkcnt_t;
__extension__ typedef __quad_t __blkcnt64_t;


__extension__ typedef unsigned long int __fsblkcnt_t;
__extension__ typedef __u_quad_t __fsblkcnt64_t;


__extension__ typedef unsigned long int __fsfilcnt_t;
__extension__ typedef __u_quad_t __fsfilcnt64_t;

__extension__ typedef int __ssize_t;



typedef __off64_t __loff_t;
typedef __quad_t *__qaddr_t;
typedef char *__caddr_t;


__extension__ typedef int __intptr_t;


__extension__ typedef unsigned int __socklen_t;
# 32 "/usr/include/sys/types.h" 2 3 4



typedef __u_char u_char;
typedef __u_short u_short;
typedef __u_int u_int;
typedef __u_long u_long;
typedef __quad_t quad_t;
typedef __u_quad_t u_quad_t;
typedef __fsid_t fsid_t;




typedef __loff_t loff_t;



typedef __ino_t ino_t;
# 62 "/usr/include/sys/types.h" 3 4
typedef __dev_t dev_t;




typedef __gid_t gid_t;




typedef __mode_t mode_t;




typedef __nlink_t nlink_t;




typedef __uid_t uid_t;





typedef __off_t off_t;
# 100 "/usr/include/sys/types.h" 3 4
typedef __pid_t pid_t;




typedef __id_t id_t;




typedef __ssize_t ssize_t;





typedef __daddr_t daddr_t;
typedef __caddr_t caddr_t;





typedef __key_t key_t;
# 133 "/usr/include/sys/types.h" 3 4
# 1 "/usr/include/time.h" 1 3 4
# 74 "/usr/include/time.h" 3 4


typedef __time_t time_t;



# 92 "/usr/include/time.h" 3 4
typedef __clockid_t clockid_t;
# 104 "/usr/include/time.h" 3 4
typedef __timer_t timer_t;
# 134 "/usr/include/sys/types.h" 2 3 4
# 147 "/usr/include/sys/types.h" 3 4
# 1 "/usr/lib/gcc/i386-redhat-linux/3.4.4/include/stddef.h" 1 3 4
# 148 "/usr/include/sys/types.h" 2 3 4



typedef unsigned long int ulong;
typedef unsigned short int ushort;
typedef unsigned int uint;
# 191 "/usr/include/sys/types.h" 3 4
typedef int int8_t __attribute__ ((__mode__ (__QI__)));
typedef int int16_t __attribute__ ((__mode__ (__HI__)));
typedef int int32_t __attribute__ ((__mode__ (__SI__)));
typedef int int64_t __attribute__ ((__mode__ (__DI__)));


typedef unsigned int u_int8_t __attribute__ ((__mode__ (__QI__)));
typedef unsigned int u_int16_t __attribute__ ((__mode__ (__HI__)));
typedef unsigned int u_int32_t __attribute__ ((__mode__ (__SI__)));
typedef unsigned int u_int64_t __attribute__ ((__mode__ (__DI__)));

typedef int register_t __attribute__ ((__mode__ (__word__)));
# 213 "/usr/include/sys/types.h" 3 4
# 1 "/usr/include/endian.h" 1 3 4
# 37 "/usr/include/endian.h" 3 4
# 1 "/usr/include/bits/endian.h" 1 3 4
# 38 "/usr/include/endian.h" 2 3 4
# 214 "/usr/include/sys/types.h" 2 3 4


# 1 "/usr/include/sys/select.h" 1 3 4
# 31 "/usr/include/sys/select.h" 3 4
# 1 "/usr/include/bits/select.h" 1 3 4
# 32 "/usr/include/sys/select.h" 2 3 4


# 1 "/usr/include/bits/sigset.h" 1 3 4
# 23 "/usr/include/bits/sigset.h" 3 4
typedef int __sig_atomic_t;




typedef struct
  {
    unsigned long int __val[(1024 / (8 * sizeof (unsigned long int)))];
  } __sigset_t;
# 35 "/usr/include/sys/select.h" 2 3 4



typedef __sigset_t sigset_t;





# 1 "/usr/include/time.h" 1 3 4
# 118 "/usr/include/time.h" 3 4
struct timespec
  {
    __time_t tv_sec;
    long int tv_nsec;
  };
# 45 "/usr/include/sys/select.h" 2 3 4

# 1 "/usr/include/bits/time.h" 1 3 4
# 69 "/usr/include/bits/time.h" 3 4
struct timeval
  {
    __time_t tv_sec;
    __suseconds_t tv_usec;
  };
# 47 "/usr/include/sys/select.h" 2 3 4


typedef __suseconds_t suseconds_t;





typedef long int __fd_mask;
# 67 "/usr/include/sys/select.h" 3 4
typedef struct
  {






    __fd_mask __fds_bits[1024 / (8 * sizeof (__fd_mask))];


  } fd_set;






typedef __fd_mask fd_mask;
# 99 "/usr/include/sys/select.h" 3 4

# 109 "/usr/include/sys/select.h" 3 4
extern int select (int __nfds, fd_set *__restrict __readfds,
     fd_set *__restrict __writefds,
     fd_set *__restrict __exceptfds,
     struct timeval *__restrict __timeout);
# 128 "/usr/include/sys/select.h" 3 4

# 217 "/usr/include/sys/types.h" 2 3 4


# 1 "/usr/include/sys/sysmacros.h" 1 3 4
# 29 "/usr/include/sys/sysmacros.h" 3 4
__extension__
extern __inline unsigned int gnu_dev_major (unsigned long long int __dev)
     __attribute__ ((__nothrow__));
__extension__
extern __inline unsigned int gnu_dev_minor (unsigned long long int __dev)
     __attribute__ ((__nothrow__));
__extension__
extern __inline unsigned long long int gnu_dev_makedev (unsigned int __major,
       unsigned int __minor)
     __attribute__ ((__nothrow__));


__extension__ extern __inline unsigned int
__attribute__ ((__nothrow__)) gnu_dev_major (unsigned long long int __dev)
{
  return ((__dev >> 8) & 0xfff) | ((unsigned int) (__dev >> 32) & ~0xfff);
}

__extension__ extern __inline unsigned int
__attribute__ ((__nothrow__)) gnu_dev_minor (unsigned long long int __dev)
{
  return (__dev & 0xff) | ((unsigned int) (__dev >> 12) & ~0xff);
}

__extension__ extern __inline unsigned long long int
__attribute__ ((__nothrow__)) gnu_dev_makedev (unsigned int __major, unsigned int __minor)
{
  return ((__minor & 0xff) | ((__major & 0xfff) << 8)
   | (((unsigned long long int) (__minor & ~0xff)) << 12)
   | (((unsigned long long int) (__major & ~0xfff)) << 32));
}
# 220 "/usr/include/sys/types.h" 2 3 4
# 231 "/usr/include/sys/types.h" 3 4
typedef __blkcnt_t blkcnt_t;



typedef __fsblkcnt_t fsblkcnt_t;



typedef __fsfilcnt_t fsfilcnt_t;
# 266 "/usr/include/sys/types.h" 3 4
# 1 "/usr/include/bits/pthreadtypes.h" 1 3 4
# 23 "/usr/include/bits/pthreadtypes.h" 3 4
# 1 "/usr/include/bits/sched.h" 1 3 4
# 83 "/usr/include/bits/sched.h" 3 4
struct __sched_param
  {
    int __sched_priority;
  };
# 24 "/usr/include/bits/pthreadtypes.h" 2 3 4


struct _pthread_fastlock
{
  long int __status;
  int __spinlock;

};



typedef struct _pthread_descr_struct *_pthread_descr;





typedef struct __pthread_attr_s
{
  int __detachstate;
  int __schedpolicy;
  struct __sched_param __schedparam;
  int __inheritsched;
  int __scope;
  size_t __guardsize;
  int __stackaddr_set;
  void *__stackaddr;
  size_t __stacksize;
} pthread_attr_t;





__extension__ typedef long long __pthread_cond_align_t;




typedef struct
{
  struct _pthread_fastlock __c_lock;
  _pthread_descr __c_waiting;
  char __padding[48 - sizeof (struct _pthread_fastlock)
   - sizeof (_pthread_descr) - sizeof (__pthread_cond_align_t)];
  __pthread_cond_align_t __align;
} pthread_cond_t;



typedef struct
{
  int __dummy;
} pthread_condattr_t;


typedef unsigned int pthread_key_t;





typedef struct
{
  int __m_reserved;
  int __m_count;
  _pthread_descr __m_owner;
  int __m_kind;
  struct _pthread_fastlock __m_lock;
} pthread_mutex_t;



typedef struct
{
  int __mutexkind;
} pthread_mutexattr_t;



typedef int pthread_once_t;
# 150 "/usr/include/bits/pthreadtypes.h" 3 4
typedef unsigned long int pthread_t;
# 267 "/usr/include/sys/types.h" 2 3 4



# 434 "/usr/include/stdlib.h" 2 3 4






extern long int random (void) __attribute__ ((__nothrow__));


extern void srandom (unsigned int __seed) __attribute__ ((__nothrow__));





extern char *initstate (unsigned int __seed, char *__statebuf,
   size_t __statelen) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));



extern char *setstate (char *__statebuf) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));







struct random_data
  {
    int32_t *fptr;
    int32_t *rptr;
    int32_t *state;
    int rand_type;
    int rand_deg;
    int rand_sep;
    int32_t *end_ptr;
  };

extern int random_r (struct random_data *__restrict __buf,
       int32_t *__restrict __result) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));

extern int srandom_r (unsigned int __seed, struct random_data *__buf)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));

extern int initstate_r (unsigned int __seed, char *__restrict __statebuf,
   size_t __statelen,
   struct random_data *__restrict __buf)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2, 4)));

extern int setstate_r (char *__restrict __statebuf,
         struct random_data *__restrict __buf)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));






extern int rand (void) __attribute__ ((__nothrow__));

extern void srand (unsigned int __seed) __attribute__ ((__nothrow__));




extern int rand_r (unsigned int *__seed) __attribute__ ((__nothrow__));







extern double drand48 (void) __attribute__ ((__nothrow__));
extern double erand48 (unsigned short int __xsubi[3]) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern long int lrand48 (void) __attribute__ ((__nothrow__));
extern long int nrand48 (unsigned short int __xsubi[3])
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern long int mrand48 (void) __attribute__ ((__nothrow__));
extern long int jrand48 (unsigned short int __xsubi[3])
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));


extern void srand48 (long int __seedval) __attribute__ ((__nothrow__));
extern unsigned short int *seed48 (unsigned short int __seed16v[3])
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));
extern void lcong48 (unsigned short int __param[7]) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));





struct drand48_data
  {
    unsigned short int __x[3];
    unsigned short int __old_x[3];
    unsigned short int __c;
    unsigned short int __init;
    unsigned long long int __a;
  };


extern int drand48_r (struct drand48_data *__restrict __buffer,
        double *__restrict __result) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));
extern int erand48_r (unsigned short int __xsubi[3],
        struct drand48_data *__restrict __buffer,
        double *__restrict __result) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));


extern int lrand48_r (struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));
extern int nrand48_r (unsigned short int __xsubi[3],
        struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));


extern int mrand48_r (struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));
extern int jrand48_r (unsigned short int __xsubi[3],
        struct drand48_data *__restrict __buffer,
        long int *__restrict __result)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));


extern int srand48_r (long int __seedval, struct drand48_data *__buffer)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));

extern int seed48_r (unsigned short int __seed16v[3],
       struct drand48_data *__buffer) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));

extern int lcong48_r (unsigned short int __param[7],
        struct drand48_data *__buffer)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1, 2)));









extern void *malloc (size_t __size) __attribute__ ((__nothrow__)) __attribute__ ((__malloc__));

extern void *calloc (size_t __nmemb, size_t __size)
     __attribute__ ((__nothrow__)) __attribute__ ((__malloc__));







extern void *realloc (void *__ptr, size_t __size) __attribute__ ((__nothrow__)) __attribute__ ((__malloc__));

extern void free (void *__ptr) __attribute__ ((__nothrow__));




extern void cfree (void *__ptr) __attribute__ ((__nothrow__));



# 1 "/usr/include/alloca.h" 1 3 4
# 25 "/usr/include/alloca.h" 3 4
# 1 "/usr/lib/gcc/i386-redhat-linux/3.4.4/include/stddef.h" 1 3 4
# 26 "/usr/include/alloca.h" 2 3 4







extern void *alloca (size_t __size) __attribute__ ((__nothrow__));






# 607 "/usr/include/stdlib.h" 2 3 4




extern void *valloc (size_t __size) __attribute__ ((__nothrow__)) __attribute__ ((__malloc__));
# 620 "/usr/include/stdlib.h" 3 4


extern void abort (void) __attribute__ ((__nothrow__)) __attribute__ ((__noreturn__));



extern int atexit (void (*__func) (void)) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));





extern int on_exit (void (*__func) (int __status, void *__arg), void *__arg)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));






extern void exit (int __status) __attribute__ ((__nothrow__)) __attribute__ ((__noreturn__));

# 652 "/usr/include/stdlib.h" 3 4


extern char *getenv (__const char *__name) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));




extern char *__secure_getenv (__const char *__name) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));





extern int putenv (char *__string) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));





extern int setenv (__const char *__name, __const char *__value, int __replace)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (2)));


extern int unsetenv (__const char *__name) __attribute__ ((__nothrow__));






extern int clearenv (void) __attribute__ ((__nothrow__));
# 691 "/usr/include/stdlib.h" 3 4
extern char *mktemp (char *__template) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));
# 702 "/usr/include/stdlib.h" 3 4
extern int mkstemp (char *__template) __attribute__ ((__nonnull__ (1)));
# 721 "/usr/include/stdlib.h" 3 4
extern char *mkdtemp (char *__template) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));








extern int system (__const char *__command);

# 749 "/usr/include/stdlib.h" 3 4
extern char *realpath (__const char *__restrict __name,
         char *__restrict __resolved) __attribute__ ((__nothrow__));






typedef int (*__compar_fn_t) (__const void *, __const void *);









extern void *bsearch (__const void *__key, __const void *__base,
        size_t __nmemb, size_t __size, __compar_fn_t __compar)
     __attribute__ ((__nonnull__ (1, 2, 5)));



extern void qsort (void *__base, size_t __nmemb, size_t __size,
     __compar_fn_t __compar) __attribute__ ((__nonnull__ (1, 4)));



extern int abs (int __x) __attribute__ ((__nothrow__)) __attribute__ ((__const__));
extern long int labs (long int __x) __attribute__ ((__nothrow__)) __attribute__ ((__const__));












extern div_t div (int __numer, int __denom)
     __attribute__ ((__nothrow__)) __attribute__ ((__const__));
extern ldiv_t ldiv (long int __numer, long int __denom)
     __attribute__ ((__nothrow__)) __attribute__ ((__const__));

# 814 "/usr/include/stdlib.h" 3 4
extern char *ecvt (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 4)));




extern char *fcvt (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 4)));




extern char *gcvt (double __value, int __ndigit, char *__buf)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3)));




extern char *qecvt (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 4)));
extern char *qfcvt (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 4)));
extern char *qgcvt (long double __value, int __ndigit, char *__buf)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3)));




extern int ecvt_r (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign, char *__restrict __buf,
     size_t __len) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 4, 5)));
extern int fcvt_r (double __value, int __ndigit, int *__restrict __decpt,
     int *__restrict __sign, char *__restrict __buf,
     size_t __len) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 4, 5)));

extern int qecvt_r (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign,
      char *__restrict __buf, size_t __len)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 4, 5)));
extern int qfcvt_r (long double __value, int __ndigit,
      int *__restrict __decpt, int *__restrict __sign,
      char *__restrict __buf, size_t __len)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (3, 4, 5)));







extern int mblen (__const char *__s, size_t __n) __attribute__ ((__nothrow__));


extern int mbtowc (wchar_t *__restrict __pwc,
     __const char *__restrict __s, size_t __n) __attribute__ ((__nothrow__));


extern int wctomb (char *__s, wchar_t __wchar) __attribute__ ((__nothrow__));



extern size_t mbstowcs (wchar_t *__restrict __pwcs,
   __const char *__restrict __s, size_t __n) __attribute__ ((__nothrow__));

extern size_t wcstombs (char *__restrict __s,
   __const wchar_t *__restrict __pwcs, size_t __n)
     __attribute__ ((__nothrow__));








extern int rpmatch (__const char *__response) __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));
# 954 "/usr/include/stdlib.h" 3 4
extern int getloadavg (double __loadavg[], int __nelem)
     __attribute__ ((__nothrow__)) __attribute__ ((__nonnull__ (1)));






# 3 "pvsparse.y" 2
# 1 "/usr/include/stdio.h" 1 3 4
# 30 "/usr/include/stdio.h" 3 4




# 1 "/usr/lib/gcc/i386-redhat-linux/3.4.4/include/stddef.h" 1 3 4
# 35 "/usr/include/stdio.h" 2 3 4
# 44 "/usr/include/stdio.h" 3 4


typedef struct _IO_FILE FILE;





# 62 "/usr/include/stdio.h" 3 4
typedef struct _IO_FILE __FILE;
# 72 "/usr/include/stdio.h" 3 4
# 1 "/usr/include/libio.h" 1 3 4
# 32 "/usr/include/libio.h" 3 4
# 1 "/usr/include/_G_config.h" 1 3 4
# 14 "/usr/include/_G_config.h" 3 4
# 1 "/usr/lib/gcc/i386-redhat-linux/3.4.4/include/stddef.h" 1 3 4
# 354 "/usr/lib/gcc/i386-redhat-linux/3.4.4/include/stddef.h" 3 4
typedef unsigned int wint_t;
# 15 "/usr/include/_G_config.h" 2 3 4
# 24 "/usr/include/_G_config.h" 3 4
# 1 "/usr/include/wchar.h" 1 3 4
# 48 "/usr/include/wchar.h" 3 4
# 1 "/usr/lib/gcc/i386-redhat-linux/3.4.4/include/stddef.h" 1 3 4
# 49 "/usr/include/wchar.h" 2 3 4

# 1 "/usr/include/bits/wchar.h" 1 3 4
# 51 "/usr/include/wchar.h" 2 3 4
# 76 "/usr/include/wchar.h" 3 4
typedef struct
{
  int __count;
  union
  {
    wint_t __wch;
    char __wchb[4];
  } __value;
} __mbstate_t;
# 25 "/usr/include/_G_config.h" 2 3 4

typedef struct
{
  __off_t __pos;
  __mbstate_t __state;
} _G_fpos_t;
typedef struct
{
  __off64_t __pos;
  __mbstate_t __state;
} _G_fpos64_t;
# 44 "/usr/include/_G_config.h" 3 4
# 1 "/usr/include/gconv.h" 1 3 4
# 28 "/usr/include/gconv.h" 3 4
# 1 "/usr/include/wchar.h" 1 3 4
# 48 "/usr/include/wchar.h" 3 4
# 1 "/usr/lib/gcc/i386-redhat-linux/3.4.4/include/stddef.h" 1 3 4
# 49 "/usr/include/wchar.h" 2 3 4
# 29 "/usr/include/gconv.h" 2 3 4


# 1 "/usr/lib/gcc/i386-redhat-linux/3.4.4/include/stddef.h" 1 3 4
# 32 "/usr/include/gconv.h" 2 3 4





enum
{
  __GCONV_OK = 0,
  __GCONV_NOCONV,
  __GCONV_NODB,
  __GCONV_NOMEM,

  __GCONV_EMPTY_INPUT,
  __GCONV_FULL_OUTPUT,
  __GCONV_ILLEGAL_INPUT,
  __GCONV_INCOMPLETE_INPUT,

  __GCONV_ILLEGAL_DESCRIPTOR,
  __GCONV_INTERNAL_ERROR
};



enum
{
  __GCONV_IS_LAST = 0x0001,
  __GCONV_IGNORE_ERRORS = 0x0002
};



struct __gconv_step;
struct __gconv_step_data;
struct __gconv_loaded_object;
struct __gconv_trans_data;



typedef int (*__gconv_fct) (struct __gconv_step *, struct __gconv_step_data *,
       __const unsigned char **, __const unsigned char *,
       unsigned char **, size_t *, int, int);


typedef wint_t (*__gconv_btowc_fct) (struct __gconv_step *, unsigned char);


typedef int (*__gconv_init_fct) (struct __gconv_step *);
typedef void (*__gconv_end_fct) (struct __gconv_step *);



typedef int (*__gconv_trans_fct) (struct __gconv_step *,
      struct __gconv_step_data *, void *,
      __const unsigned char *,
      __const unsigned char **,
      __const unsigned char *, unsigned char **,
      size_t *);


typedef int (*__gconv_trans_context_fct) (void *, __const unsigned char *,
       __const unsigned char *,
       unsigned char *, unsigned char *);


typedef int (*__gconv_trans_query_fct) (__const char *, __const char ***,
     size_t *);


typedef int (*__gconv_trans_init_fct) (void **, const char *);
typedef void (*__gconv_trans_end_fct) (void *);

struct __gconv_trans_data
{

  __gconv_trans_fct __trans_fct;
  __gconv_trans_context_fct __trans_context_fct;
  __gconv_trans_end_fct __trans_end_fct;
  void *__data;
  struct __gconv_trans_data *__next;
};



struct __gconv_step
{
  struct __gconv_loaded_object *__shlib_handle;
  __const char *__modname;

  int __counter;

  char *__from_name;
  char *__to_name;

  __gconv_fct __fct;
  __gconv_btowc_fct __btowc_fct;
  __gconv_init_fct __init_fct;
  __gconv_end_fct __end_fct;



  int __min_needed_from;
  int __max_needed_from;
  int __min_needed_to;
  int __max_needed_to;


  int __stateful;

  void *__data;
};



struct __gconv_step_data
{
  unsigned char *__outbuf;
  unsigned char *__outbufend;



  int __flags;



  int __invocation_counter;



  int __internal_use;

  __mbstate_t *__statep;
  __mbstate_t __state;



  struct __gconv_trans_data *__trans;
};



typedef struct __gconv_info
{
  size_t __nsteps;
  struct __gconv_step *__steps;
  __extension__ struct __gconv_step_data __data [];
} *__gconv_t;
# 45 "/usr/include/_G_config.h" 2 3 4
typedef union
{
  struct __gconv_info __cd;
  struct
  {
    struct __gconv_info __cd;
    struct __gconv_step_data __data;
  } __combined;
} _G_iconv_t;

typedef int _G_int16_t __attribute__ ((__mode__ (__HI__)));
typedef int _G_int32_t __attribute__ ((__mode__ (__SI__)));
typedef unsigned int _G_uint16_t __attribute__ ((__mode__ (__HI__)));
typedef unsigned int _G_uint32_t __attribute__ ((__mode__ (__SI__)));
# 33 "/usr/include/libio.h" 2 3 4
# 53 "/usr/include/libio.h" 3 4
# 1 "/usr/lib/gcc/i386-redhat-linux/3.4.4/include/stdarg.h" 1 3 4
# 43 "/usr/lib/gcc/i386-redhat-linux/3.4.4/include/stdarg.h" 3 4
typedef __builtin_va_list __gnuc_va_list;
# 54 "/usr/include/libio.h" 2 3 4
# 166 "/usr/include/libio.h" 3 4
struct _IO_jump_t; struct _IO_FILE;
# 176 "/usr/include/libio.h" 3 4
typedef void _IO_lock_t;





struct _IO_marker {
  struct _IO_marker *_next;
  struct _IO_FILE *_sbuf;



  int _pos;
# 199 "/usr/include/libio.h" 3 4
};


enum __codecvt_result
{
  __codecvt_ok,
  __codecvt_partial,
  __codecvt_error,
  __codecvt_noconv
};
# 267 "/usr/include/libio.h" 3 4
struct _IO_FILE {
  int _flags;




  char* _IO_read_ptr;
  char* _IO_read_end;
  char* _IO_read_base;
  char* _IO_write_base;
  char* _IO_write_ptr;
  char* _IO_write_end;
  char* _IO_buf_base;
  char* _IO_buf_end;

  char *_IO_save_base;
  char *_IO_backup_base;
  char *_IO_save_end;

  struct _IO_marker *_markers;

  struct _IO_FILE *_chain;

  int _fileno;



  int _flags2;

  __off_t _old_offset;



  unsigned short _cur_column;
  signed char _vtable_offset;
  char _shortbuf[1];



  _IO_lock_t *_lock;
# 315 "/usr/include/libio.h" 3 4
  __off64_t _offset;





  void *__pad1;
  void *__pad2;

  int _mode;

  char _unused2[15 * sizeof (int) - 2 * sizeof (void *)];

};


typedef struct _IO_FILE _IO_FILE;


struct _IO_FILE_plus;

extern struct _IO_FILE_plus _IO_2_1_stdin_;
extern struct _IO_FILE_plus _IO_2_1_stdout_;
extern struct _IO_FILE_plus _IO_2_1_stderr_;
# 354 "/usr/include/libio.h" 3 4
typedef __ssize_t __io_read_fn (void *__cookie, char *__buf, size_t __nbytes);







typedef __ssize_t __io_write_fn (void *__cookie, __const char *__buf,
     size_t __n);







typedef int __io_seek_fn (void *__cookie, __off64_t *__pos, int __w);


typedef int __io_close_fn (void *__cookie);
# 406 "/usr/include/libio.h" 3 4
extern int __underflow (_IO_FILE *) __attribute__ ((__nothrow__));
extern int __uflow (_IO_FILE *) __attribute__ ((__nothrow__));
extern int __overflow (_IO_FILE *, int) __attribute__ ((__nothrow__));
extern wint_t __wunderflow (_IO_FILE *) __attribute__ ((__nothrow__));
extern wint_t __wuflow (_IO_FILE *) __attribute__ ((__nothrow__));
extern wint_t __woverflow (_IO_FILE *, wint_t) __attribute__ ((__nothrow__));
# 444 "/usr/include/libio.h" 3 4
extern int _IO_getc (_IO_FILE *__fp) __attribute__ ((__nothrow__));
extern int _IO_putc (int __c, _IO_FILE *__fp) __attribute__ ((__nothrow__));
extern int _IO_feof (_IO_FILE *__fp) __attribute__ ((__nothrow__));
extern int _IO_ferror (_IO_FILE *__fp) __attribute__ ((__nothrow__));

extern int _IO_peekc_locked (_IO_FILE *__fp) __attribute__ ((__nothrow__));





extern void _IO_flockfile (_IO_FILE *) __attribute__ ((__nothrow__));
extern void _IO_funlockfile (_IO_FILE *) __attribute__ ((__nothrow__));
extern int _IO_ftrylockfile (_IO_FILE *) __attribute__ ((__nothrow__));
# 474 "/usr/include/libio.h" 3 4
extern int _IO_vfscanf (_IO_FILE * __restrict, const char * __restrict,
   __gnuc_va_list, int *__restrict) __attribute__ ((__nothrow__));
extern int _IO_vfprintf (_IO_FILE *__restrict, const char *__restrict,
    __gnuc_va_list) __attribute__ ((__nothrow__));
extern __ssize_t _IO_padn (_IO_FILE *, int, __ssize_t) __attribute__ ((__nothrow__));
extern size_t _IO_sgetn (_IO_FILE *, void *, size_t) __attribute__ ((__nothrow__));

extern __off64_t _IO_seekoff (_IO_FILE *, __off64_t, int, int) __attribute__ ((__nothrow__));
extern __off64_t _IO_seekpos (_IO_FILE *, __off64_t, int) __attribute__ ((__nothrow__));

extern void _IO_free_backup_area (_IO_FILE *) __attribute__ ((__nothrow__));
# 73 "/usr/include/stdio.h" 2 3 4
# 86 "/usr/include/stdio.h" 3 4


typedef _G_fpos_t fpos_t;




# 138 "/usr/include/stdio.h" 3 4
# 1 "/usr/include/bits/stdio_lim.h" 1 3 4
# 139 "/usr/include/stdio.h" 2 3 4



extern struct _IO_FILE *stdin;
extern struct _IO_FILE *stdout;
extern struct _IO_FILE *stderr;









extern int remove (__const char *__filename) __attribute__ ((__nothrow__));

extern int rename (__const char *__old, __const char *__new) __attribute__ ((__nothrow__));









extern FILE *tmpfile (void);
# 180 "/usr/include/stdio.h" 3 4
extern char *tmpnam (char *__s) __attribute__ ((__nothrow__));





extern char *tmpnam_r (char *__s) __attribute__ ((__nothrow__));
# 198 "/usr/include/stdio.h" 3 4
extern char *tempnam (__const char *__dir, __const char *__pfx)
     __attribute__ ((__nothrow__)) __attribute__ ((__malloc__));








extern int fclose (FILE *__stream);




extern int fflush (FILE *__stream);

# 223 "/usr/include/stdio.h" 3 4
extern int fflush_unlocked (FILE *__stream);
# 237 "/usr/include/stdio.h" 3 4






extern FILE *fopen (__const char *__restrict __filename,
      __const char *__restrict __modes);




extern FILE *freopen (__const char *__restrict __filename,
        __const char *__restrict __modes,
        FILE *__restrict __stream);
# 264 "/usr/include/stdio.h" 3 4

# 275 "/usr/include/stdio.h" 3 4
extern FILE *fdopen (int __fd, __const char *__modes) __attribute__ ((__nothrow__));
# 296 "/usr/include/stdio.h" 3 4



extern void setbuf (FILE *__restrict __stream, char *__restrict __buf) __attribute__ ((__nothrow__));



extern int setvbuf (FILE *__restrict __stream, char *__restrict __buf,
      int __modes, size_t __n) __attribute__ ((__nothrow__));





extern void setbuffer (FILE *__restrict __stream, char *__restrict __buf,
         size_t __size) __attribute__ ((__nothrow__));


extern void setlinebuf (FILE *__stream) __attribute__ ((__nothrow__));








extern int fprintf (FILE *__restrict __stream,
      __const char *__restrict __format, ...);




extern int printf (__const char *__restrict __format, ...);

extern int sprintf (char *__restrict __s,
      __const char *__restrict __format, ...) __attribute__ ((__nothrow__));





extern int vfprintf (FILE *__restrict __s, __const char *__restrict __format,
       __gnuc_va_list __arg);




extern int vprintf (__const char *__restrict __format, __gnuc_va_list __arg);

extern int vsprintf (char *__restrict __s, __const char *__restrict __format,
       __gnuc_va_list __arg) __attribute__ ((__nothrow__));





extern int snprintf (char *__restrict __s, size_t __maxlen,
       __const char *__restrict __format, ...)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 3, 4)));

extern int vsnprintf (char *__restrict __s, size_t __maxlen,
        __const char *__restrict __format, __gnuc_va_list __arg)
     __attribute__ ((__nothrow__)) __attribute__ ((__format__ (__printf__, 3, 0)));

# 390 "/usr/include/stdio.h" 3 4





extern int fscanf (FILE *__restrict __stream,
     __const char *__restrict __format, ...);




extern int scanf (__const char *__restrict __format, ...);

extern int sscanf (__const char *__restrict __s,
     __const char *__restrict __format, ...) __attribute__ ((__nothrow__));

# 432 "/usr/include/stdio.h" 3 4





extern int fgetc (FILE *__stream);
extern int getc (FILE *__stream);





extern int getchar (void);

# 456 "/usr/include/stdio.h" 3 4
extern int getc_unlocked (FILE *__stream);
extern int getchar_unlocked (void);
# 467 "/usr/include/stdio.h" 3 4
extern int fgetc_unlocked (FILE *__stream);











extern int fputc (int __c, FILE *__stream);
extern int putc (int __c, FILE *__stream);





extern int putchar (int __c);

# 500 "/usr/include/stdio.h" 3 4
extern int fputc_unlocked (int __c, FILE *__stream);







extern int putc_unlocked (int __c, FILE *__stream);
extern int putchar_unlocked (int __c);






extern int getw (FILE *__stream);


extern int putw (int __w, FILE *__stream);








extern char *fgets (char *__restrict __s, int __n, FILE *__restrict __stream);






extern char *gets (char *__s);

# 580 "/usr/include/stdio.h" 3 4





extern int fputs (__const char *__restrict __s, FILE *__restrict __stream);





extern int puts (__const char *__s);






extern int ungetc (int __c, FILE *__stream);






extern size_t fread (void *__restrict __ptr, size_t __size,
       size_t __n, FILE *__restrict __stream);




extern size_t fwrite (__const void *__restrict __ptr, size_t __size,
        size_t __n, FILE *__restrict __s);

# 633 "/usr/include/stdio.h" 3 4
extern size_t fread_unlocked (void *__restrict __ptr, size_t __size,
         size_t __n, FILE *__restrict __stream);
extern size_t fwrite_unlocked (__const void *__restrict __ptr, size_t __size,
          size_t __n, FILE *__restrict __stream);








extern int fseek (FILE *__stream, long int __off, int __whence);




extern long int ftell (FILE *__stream);




extern void rewind (FILE *__stream);

# 688 "/usr/include/stdio.h" 3 4






extern int fgetpos (FILE *__restrict __stream, fpos_t *__restrict __pos);




extern int fsetpos (FILE *__stream, __const fpos_t *__pos);
# 711 "/usr/include/stdio.h" 3 4

# 720 "/usr/include/stdio.h" 3 4


extern void clearerr (FILE *__stream) __attribute__ ((__nothrow__));

extern int feof (FILE *__stream) __attribute__ ((__nothrow__));

extern int ferror (FILE *__stream) __attribute__ ((__nothrow__));




extern void clearerr_unlocked (FILE *__stream) __attribute__ ((__nothrow__));
extern int feof_unlocked (FILE *__stream) __attribute__ ((__nothrow__));
extern int ferror_unlocked (FILE *__stream) __attribute__ ((__nothrow__));








extern void perror (__const char *__s);






# 1 "/usr/include/bits/sys_errlist.h" 1 3 4
# 27 "/usr/include/bits/sys_errlist.h" 3 4
extern int sys_nerr;
extern __const char *__const sys_errlist[];
# 750 "/usr/include/stdio.h" 2 3 4




extern int fileno (FILE *__stream) __attribute__ ((__nothrow__));




extern int fileno_unlocked (FILE *__stream) __attribute__ ((__nothrow__));
# 769 "/usr/include/stdio.h" 3 4
extern FILE *popen (__const char *__command, __const char *__modes);





extern int pclose (FILE *__stream);





extern char *ctermid (char *__s) __attribute__ ((__nothrow__));
# 809 "/usr/include/stdio.h" 3 4
extern void flockfile (FILE *__stream) __attribute__ ((__nothrow__));



extern int ftrylockfile (FILE *__stream) __attribute__ ((__nothrow__));


extern void funlockfile (FILE *__stream) __attribute__ ((__nothrow__));
# 836 "/usr/include/stdio.h" 3 4

# 4 "pvsparse.y" 2
# 1 "../../include/mathpad.h" 1
# 13 "../../include/mathpad.h"
typedef unsigned int Index;
typedef int Offset;

typedef enum Opkind { None, Prefix, Postfix,
                      Infix, LeftBinding, RightBinding
                    } Opkind;




# 1 "../../include/unicode.h" 1
# 30 "../../include/unicode.h"
# 1 "../../include/memman.h" 1
# 55 "../../include/memman.h"
extern void *mm_malloc(size_t size);
extern void *mm_realloc(void *p, size_t size);
extern void *mm_calloc(size_t n, size_t size);
extern void mm_free(void *p);






typedef struct {
  void *data;
  int length;
  int regis;
  int size;
  int min;
  int max;
} FreeList;
# 88 "../../include/memman.h"
extern void FreeList_free(FreeList *fl, void *data);


extern void *FreeList_malloc(FreeList *fl);


extern void FreeList_fill(FreeList *fl);


extern void FreeList_clear(FreeList *fl);
# 111 "../../include/memman.h"
extern void RegisterPaybackFunction(void (*function)(void));

extern void RemovePaybackFunction(void (*function)(void));
# 31 "../../include/unicode.h" 2

typedef unsigned short Uchar;
# 43 "../../include/unicode.h"
typedef char **MapBool;
typedef char **MapChar;
typedef Uchar **MapUchar;
typedef Uchar ***MapUstr;
typedef char ***MapStr;
typedef int **MapInt;
# 24 "../../include/mathpad.h" 2
# 103 "../../include/mathpad.h"
typedef enum { Normal, Reverse, Xor } TextMode;
# 149 "../../include/mathpad.h"
typedef void (*Cpfv)(void);
# 159 "../../include/mathpad.h"
extern Uchar *translate(char *string);
# 5 "pvsparse.y" 2
# 1 "../../include/editor.h" 1
# 13 "../../include/editor.h"
extern void display_left(Index);
extern void display_right(Index);
extern void up(Index);
extern void up_selection(void*);
extern void down(Index);
extern void forward_char(Index);
extern void backward_char(Index);
extern void forward_line(Index);
extern void backward_line(Index);
extern void begin_of_line(void);
extern void end_of_line(void);
extern void scroll_up(Index);
extern void scroll_down(Index);
extern void recenter(void);
extern void move_to_center(void);
extern void begin_of_buffer(void);
extern void end_of_buffer(void);
extern void forward_word(Index);
extern void backward_word(Index);
extern void transpose_chars(Index);
extern void transpose_words(Index);
extern void upcase_word(void);
extern void downcase_word(void);


extern void next_node_or_insert(int, Index);
extern void next_node_insert(int, Index);
extern void openparen_insert(int, Index);
extern void closeparen_insert(int, Index);
extern void make_list_insert(int, Index);
extern void insert_char(int,Index);
extern void insert_string(Uchar *);
extern void insert_expr(Index);
extern void insert_text(Index);
extern void insert_disp(Index);
extern void insert_var(Index);
extern void insert_id(Index);
extern void insert_op(Index);
extern void insert_newline(Index);
extern void insert_hard_newline(Index);
extern void insert_rtab(Index);
extern void insert_symbol(int, Index);

extern void backward_remove_char(Index);
extern void forward_remove_char(Index);
extern void remove_double_chars(void);
extern void remove_region(void);
extern void kill_region(void);
extern void kill_word(Index);
extern void backward_kill_word(Index);
extern void kill_line(Index);
extern void backward_kill_line(Index);
extern void append_next_kill(void);
extern void yank(void);

extern void set_index_nr(Index);
extern void increase_spacing(Index);
extern void decrease_spacing(Index);
extern void reset_spacing(void);
extern void raise_node(void);
extern void lower_region(void);
extern void copy_region(void);
extern void swap_region(void);
extern void set_parens(void);
extern void unset_parens(void);
extern void clear_parens(void);
extern void switch_parens(void);
extern void insert_notation(Index);
extern void rename_id(void);
extern void apply(void);
extern void distribute(void);
extern void factorise(void);
extern void commute(void);
extern void goto_latex_line(Index);
extern void latex_text_only(int tonly);
extern void latex_all_parens(int allparens);
extern void latex_selection(int selnum);

extern void join_selections(Index,Index);
extern void swap_selections(Index,Index);
extern void copy_selections(Index,Index);
extern void unset_select(Index);
extern int ps_notation(Offset *vnr);
extern int ss_notation(Offset *vnr);
extern int selected_notation(int selnr, Offset *vnr);
extern int ps_id_font(void);
extern void new_id_font(Offset nfnr);
extern void new_version(Offset nnnr);
extern void stack_position(void);
extern void use_stack(void);
extern void clear_stack(void);
extern void clear_stack_and_use(void);
extern int check_find(void);
extern int check_find_replace(void);
extern int find_tree(void);
extern void replace_tree(void);
extern void replace_all_tree(void);
extern void remove_find_stack(void);
extern void find_prev_on_stack(void);
extern void find_next_on_stack(void);
extern void find_new_on_stack(void);
extern int get_findrep(Uchar *, int*,int);
extern int put_findrep(void);
extern int find_string(Uchar *);
extern int find_backward_string(Uchar*);
extern int findprev_string(Uchar *);
extern int findnext_string(Uchar *);
extern int findwrap_backward_string(Uchar *);
extern int findwrap_string(Uchar *);
extern int find_replace(Uchar *);
extern int findnext_replace(Uchar *);
extern void replace_string(Uchar *, Uchar *);
extern void replace_all(Uchar *, Uchar *);
extern int find_stencil(Index);
extern int find_backward_stencil(Index);
extern int findprev_stencil(Index);
extern int findnext_stencil(Index);
extern int findwrap_backward_stencil(Index);
extern int findwrap_stencil(Index);
extern int find_replace_stencil(Index);
extern int findnext_replace_stencil(Index);
extern void replace_notation(Index,Index);
extern void replace_all_notation(Index, Index);

extern void construct_selection(void **);
extern void destruct_selection(void *);
extern void copy_selection(void **, void *);
extern void filter_template_selection(void *, Index *);
extern void insert_template_selection(void *, Index);
extern void insert_string_selection(void *, Uchar *);
extern void commute_selection(void *);
extern void next_node_selection(void *);
extern void insert_parse_result(void *);
extern void insert_selection(void *, Uchar);
extern void filter_selection(void *, Uchar *);
extern int get_selection_path(void *full_sel, void *sub_sel,
          int *list, int max);
extern void change_selection(void *, int*, int);
extern void redraw_selection(void *);
extern void *get_selection(int nr);

extern void draw(void *);
extern void update_selections(void);
extern void redraw_window(void*);
extern void word_wrap_window(void*);
extern void word_wrap_selection(int);
extern void clear_tab_positions(void);
extern void resize_window(void*,unsigned int,unsigned int);
extern void other_window(void*);
extern void editwindow_line(void*,int);
extern void editwindow_topto(void*, Uchar*);
extern int line_number(void *);
extern int number_of_lines(void*);
extern void cursor_use_xor(int);

extern void *open_miniwindow(void*,unsigned int, unsigned int);
extern void *open_scratchwindow(void*,unsigned int,unsigned int);
extern void close_scratchwindow(void);
extern void *open_findwindow(void*,unsigned int,unsigned int);
extern void close_findwindow(void);
extern void *open_replacewindow(void*,unsigned int,unsigned int);
extern void close_replacewindow(void);
extern void *open_editwindow(void*,unsigned int,unsigned int);
extern void close_editwindow(void*);
extern void old_load_editwindow(void*,FILE*);
extern void old_include_editwindow(void*, FILE*);
extern void append_editwindow(void*, Uchar *, unsigned int);
extern void append_structure(void*);
extern void load_editwindow(void*);
extern void include_editwindow(void*);
extern void include_selection(void);
extern void save_editwindow(void*);
extern int window_changed(void*);
extern int window_empty(void*);
extern void latex_editwindow(void*);
extern void clear_window(void*);
extern int get_node(Uchar*,int*,int);
extern int get_ascii_node(Uchar*,int*,int);
extern void cleanup_nodestack(void);
extern void *make_node(Uchar type, Uchar *txt, int len, int nnr, int spacing);
extern void apply_leibnitz(void);
extern void join_parse_stack(void);
extern void *add_parse_stack(Uchar *txt, int len);

extern void mouse_down(void*,int,int, Index button);
extern void mouse_move(int, int);
extern void mouse_up(int,int);
extern void dbl_click(void);

extern int move_selection;

extern Uchar *get_subnode_string(int selnr, int *posi, int len);
extern void *get_subnode(int selnr, int *posi, int len);
extern int node_notation(void *node, int *vnr);
extern void latex_node(void *node);
extern void *first_node(void *selection);
extern void *last_node(void *selection);
# 6 "pvsparse.y" 2

static unsigned long
  subscript_uid=3407435781UL,
  compose_uid=3406469154UL,
  times_uid=3407510728UL,
  plus_uid=3407510727UL,
  equal_uid=3383944400UL,
  apply_uid=2946556517UL,
  and_uid=2666975502UL,
  id_uid=3368830881UL;


void MakeBinOpNode(void)
{
  Uchar buffer[4];
  buffer[0]=buffer[2]=0xF700;
  buffer[1]=0xF710;
  buffer[3]=0;
  make_node(0xF700, buffer, 3,0,0x3fff);
}
void MakeSubscriptNode(void)
{
   Uchar buffer[3];
   buffer[0]=((0xF700) | (1));
   buffer[1]=((0xF700) | (2));
   buffer[2]=0;
   make_node(0xF710, buffer, 2, notation_with_number(subscript_uid), 0);
}
# 46 "pvsparse.y"
int pvs_yyerror(char *c)
{
  fprintf(stderr, "%s\n", c);
  return 0;
}
# 68 "pvsparse.y"
typedef int YYSTYPE;
# 162 "y.tab.c"
# 207 "y.tab.c"
union pvs_yyalloc
{
  short pvs_yyss;
  YYSTYPE pvs_yyvs;
  };
# 259 "y.tab.c"
   typedef signed char pvs_yysigned_char;
# 286 "y.tab.c"
static const unsigned char pvs_yytranslate[] =
{
       0, 2, 2, 2, 2, 2, 2, 2, 2, 2,
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
      12, 13, 2, 2, 2, 2, 2, 2, 2, 2,
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
       2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
       2, 2, 2, 2, 2, 2, 1, 2, 3, 4,
       5, 6, 7, 8, 9, 10, 11
};
# 367 "y.tab.c"
static const unsigned char pvs_yyr1[] =
{
       0, 14, 16, 15, 17, 15, 18, 15, 19, 15,
      20, 15, 21, 15, 15, 15, 15, 15
};


static const unsigned char pvs_yyr2[] =
{
       0, 2, 0, 4, 0, 4, 0, 4, 0, 4,
       0, 4, 0, 4, 4, 1, 3, 1
};




static const unsigned char pvs_yydefact[] =
{
       0, 17, 15, 0, 0, 0, 0, 1, 10, 8,
       2, 4, 6, 12, 0, 16, 0, 0, 0, 0,
       0, 0, 14, 11, 9, 3, 5, 7, 13
};


static const pvs_yysigned_char pvs_yydefgoto[] =
{
      -1, 4, 18, 19, 20, 17, 16, 21
};




static const pvs_yysigned_char pvs_yypact[] =
{
       0, -10, -9, 0, 1, 0, 16, -10, -10, -10,
     -10, -10, -10, -10, 27, -10, 0, 0, 0, 0,
       0, 0, -10, 37, 20, 30, 39, 31, -10
};


static const pvs_yysigned_char pvs_yypgoto[] =
{
     -10, -3, -10, -10, -10, -10, -10, -10
};






static const unsigned char pvs_yytable[] =
{
       6, 7, 14, 5, 8, 9, 10, 11, 12, 13,
       1, 2, 3, 23, 24, 25, 26, 27, 28, 8,
       9, 10, 11, 12, 13, 10, 11, 12, 13, 15,
       8, 9, 10, 11, 12, 13, 11, 12, 13, 13,
      22, 9, 10, 11, 12, 13, 12, 13
};

static const unsigned char pvs_yycheck[] =
{
       3, 0, 5, 12, 3, 4, 5, 6, 7, 8,
      10, 11, 12, 16, 17, 18, 19, 20, 21, 3,
       4, 5, 6, 7, 8, 5, 6, 7, 8, 13,
       3, 4, 5, 6, 7, 8, 6, 7, 8, 8,
      13, 4, 5, 6, 7, 8, 7, 8
};



static const unsigned char pvs_yystos[] =
{
       0, 10, 11, 12, 15, 12, 15, 0, 3, 4,
       5, 6, 7, 8, 15, 13, 20, 19, 16, 17,
      18, 21, 13, 15, 15, 15, 15, 15, 15
};
# 737 "y.tab.c"
static void
pvs_yydestruct (int pvs_yytype, YYSTYPE *pvs_yyvaluep)






{

  (void) pvs_yyvaluep;

  switch (pvs_yytype)
    {

      default:
        break;
    }
}
# 768 "y.tab.c"
int pvs_yyparse (void);
# 777 "y.tab.c"
int pvs_yychar;


YYSTYPE pvs_yylval;


int pvs_yynerrs;
# 800 "y.tab.c"
int
pvs_yyparse (void)






{

  register int pvs_yystate;
  register int pvs_yyn;
  int pvs_yyresult;

  int pvs_yyerrstatus;

  int pvs_yytoken = 0;
# 827 "y.tab.c"
  short pvs_yyssa[200];
  short *pvs_yyss = pvs_yyssa;
  register short *pvs_yyssp;


  YYSTYPE pvs_yyvsa[200];
  YYSTYPE *pvs_yyvs = pvs_yyvsa;
  register YYSTYPE *pvs_yyvsp;





  unsigned int pvs_yystacksize = 200;



  YYSTYPE pvs_yyval;




  int pvs_yylen;

  ;

  pvs_yystate = 0;
  pvs_yyerrstatus = 0;
  pvs_yynerrs = 0;
  pvs_yychar = (-2);






  pvs_yyssp = pvs_yyss;
  pvs_yyvsp = pvs_yyvs;

  goto pvs_yysetstate;




 pvs_yynewstate:



  pvs_yyssp++;

 pvs_yysetstate:
  *pvs_yyssp = pvs_yystate;

  if (pvs_yyss + pvs_yystacksize - 1 <= pvs_yyssp)
    {

      unsigned int pvs_yysize = pvs_yyssp - pvs_yyss + 1;
# 912 "y.tab.c"
      if (10000 <= pvs_yystacksize)
 goto pvs_yyoverflowlab;
      pvs_yystacksize *= 2;
      if (10000 < pvs_yystacksize)
 pvs_yystacksize = 10000;

      {
 short *pvs_yyss1 = pvs_yyss;
 union pvs_yyalloc *pvs_yyptr =
   (union pvs_yyalloc *) __builtin_alloca (((pvs_yystacksize) * (sizeof (short) + sizeof (YYSTYPE)) + (sizeof (union pvs_yyalloc) - 1)));
 if (! pvs_yyptr)
   goto pvs_yyoverflowlab;
 do { unsigned int pvs_yynewbytes; __builtin_memcpy (&pvs_yyptr->pvs_yyss, pvs_yyss, (pvs_yysize) * sizeof (*(pvs_yyss))); pvs_yyss = &pvs_yyptr->pvs_yyss; pvs_yynewbytes = pvs_yystacksize * sizeof (*pvs_yyss) + (sizeof (union pvs_yyalloc) - 1); pvs_yyptr += pvs_yynewbytes / sizeof (*pvs_yyptr); } while (0);
 do { unsigned int pvs_yynewbytes; __builtin_memcpy (&pvs_yyptr->pvs_yyvs, pvs_yyvs, (pvs_yysize) * sizeof (*(pvs_yyvs))); pvs_yyvs = &pvs_yyptr->pvs_yyvs; pvs_yynewbytes = pvs_yystacksize * sizeof (*pvs_yyvs) + (sizeof (union pvs_yyalloc) - 1); pvs_yyptr += pvs_yynewbytes / sizeof (*pvs_yyptr); } while (0);


 if (pvs_yyss1 != pvs_yyssa)
   do { ; } while (0);
      }



      pvs_yyssp = pvs_yyss + pvs_yysize - 1;
      pvs_yyvsp = pvs_yyvs + pvs_yysize - 1;


      ;


      if (pvs_yyss + pvs_yystacksize - 1 <= pvs_yyssp)
 goto pvs_yyabortlab;
    }

  ;

  goto pvs_yybackup;




pvs_yybackup:







  pvs_yyn = pvs_yypact[pvs_yystate];
  if (pvs_yyn == -10)
    goto pvs_yydefault;




  if (pvs_yychar == (-2))
    {
      ;
      pvs_yychar = pvs_yylex ();
    }

  if (pvs_yychar <= 0)
    {
      pvs_yychar = pvs_yytoken = 0;
      ;
    }
  else
    {
      pvs_yytoken = ((unsigned int) (pvs_yychar) <= 266 ? pvs_yytranslate[pvs_yychar] : 2);
      ;
    }



  pvs_yyn += pvs_yytoken;
  if (pvs_yyn < 0 || 47 < pvs_yyn || pvs_yycheck[pvs_yyn] != pvs_yytoken)
    goto pvs_yydefault;
  pvs_yyn = pvs_yytable[pvs_yyn];
  if (pvs_yyn <= 0)
    {
      if (pvs_yyn == 0 || pvs_yyn == -1)
 goto pvs_yyerrlab;
      pvs_yyn = -pvs_yyn;
      goto pvs_yyreduce;
    }

  if (pvs_yyn == 7)
    goto pvs_yyacceptlab;


  ;


  if (pvs_yychar != 0)
    pvs_yychar = (-2);

  *++pvs_yyvsp = pvs_yylval;




  if (pvs_yyerrstatus)
    pvs_yyerrstatus--;

  pvs_yystate = pvs_yyn;
  goto pvs_yynewstate;





pvs_yydefault:
  pvs_yyn = pvs_yydefact[pvs_yystate];
  if (pvs_yyn == 0)
    goto pvs_yyerrlab;
  goto pvs_yyreduce;





pvs_yyreduce:

  pvs_yylen = pvs_yyr2[pvs_yyn];
# 1045 "y.tab.c"
  pvs_yyval = pvs_yyvsp[1-pvs_yylen];


  ;
  switch (pvs_yyn)
    {
        case 2:
# 63 "pvsparse.y"
    { Uchar buffer[1];
                  make_node(0xF710,buffer,0,notation_with_number(plus_uid),0);
               }
    break;

  case 3:
# 67 "pvsparse.y"
    { MakeBinOpNode(); }
    break;

  case 4:
# 69 "pvsparse.y"
    { Uchar buffer[1];
                  make_node(0xF710,buffer,0,notation_with_number(times_uid),0);
               }
    break;

  case 5:
# 73 "pvsparse.y"
    { MakeBinOpNode(); }
    break;

  case 6:
# 75 "pvsparse.y"
    { Uchar buffer[1];
                 make_node(0xF710,buffer,0,notation_with_number(compose_uid),0);
               }
    break;

  case 7:
# 79 "pvsparse.y"
    { MakeBinOpNode(); }
    break;

  case 8:
# 81 "pvsparse.y"
    { Uchar buffer[1];
                  make_node(0xF710,buffer,0,notation_with_number(equal_uid),0);
               }
    break;

  case 9:
# 85 "pvsparse.y"
    { MakeBinOpNode(); }
    break;

  case 10:
# 87 "pvsparse.y"
    { Uchar buffer[1];
                  make_node(0xF710,buffer,0,notation_with_number(and_uid),0);
               }
    break;

  case 11:
# 91 "pvsparse.y"
    { MakeBinOpNode(); }
    break;

  case 12:
# 93 "pvsparse.y"
    { Uchar buffer[1];
                  make_node(0xF710,buffer,0,notation_with_number(apply_uid),0);
               }
    break;

  case 13:
# 97 "pvsparse.y"
    { MakeBinOpNode(); }
    break;

  case 14:
# 99 "pvsparse.y"
    { MakeSubscriptNode(); }
    break;

  case 15:
# 101 "pvsparse.y"
    { }
    break;

  case 16:
# 103 "pvsparse.y"
    { ; }

    break;

  case 17:
# 106 "pvsparse.y"
    { ; }
    break;


    }
# 1149 "y.tab.c"

  pvs_yyvsp -= pvs_yylen;
  pvs_yyssp -= pvs_yylen;


  ;

  *++pvs_yyvsp = pvs_yyval;






  pvs_yyn = pvs_yyr1[pvs_yyn];

  pvs_yystate = pvs_yypgoto[pvs_yyn - 14] + *pvs_yyssp;
  if (0 <= pvs_yystate && pvs_yystate <= 47 && pvs_yycheck[pvs_yystate] == *pvs_yyssp)
    pvs_yystate = pvs_yytable[pvs_yystate];
  else
    pvs_yystate = pvs_yydefgoto[pvs_yyn - 14];

  goto pvs_yynewstate;





pvs_yyerrlab:

  if (!pvs_yyerrstatus)
    {
      ++pvs_yynerrs;
# 1241 "y.tab.c"
 pvs_yyerror ("syntax error");
    }



  if (pvs_yyerrstatus == 3)
    {



      if (pvs_yychar <= 0)
        {


   if (pvs_yychar == 0)
      for (;;)
        {
   (pvs_yyvsp--, pvs_yyssp--);
   if (pvs_yyssp == pvs_yyss)
     goto pvs_yyabortlab;
   ;
   pvs_yydestruct (pvs_yystos[*pvs_yyssp], pvs_yyvsp);
        }
        }
      else
 {
   ;
   pvs_yydestruct (pvs_yytoken, &pvs_yylval);
   pvs_yychar = (-2);

 }
    }



  goto pvs_yyerrlab1;





pvs_yyerrorlab:




  if (0)
     goto pvs_yyerrorlab;


  pvs_yyvsp -= pvs_yylen;
  pvs_yyssp -= pvs_yylen;
  pvs_yystate = *pvs_yyssp;
  goto pvs_yyerrlab1;





pvs_yyerrlab1:
  pvs_yyerrstatus = 3;

  for (;;)
    {
      pvs_yyn = pvs_yypact[pvs_yystate];
      if (pvs_yyn != -10)
 {
   pvs_yyn += 1;
   if (0 <= pvs_yyn && pvs_yyn <= 47 && pvs_yycheck[pvs_yyn] == 1)
     {
       pvs_yyn = pvs_yytable[pvs_yyn];
       if (0 < pvs_yyn)
  break;
     }
 }


      if (pvs_yyssp == pvs_yyss)
 goto pvs_yyabortlab;

      ;
      pvs_yydestruct (pvs_yystos[pvs_yystate], pvs_yyvsp);
      (pvs_yyvsp--, pvs_yyssp--);
      pvs_yystate = *pvs_yyssp;
      ;
    }

  if (pvs_yyn == 7)
    goto pvs_yyacceptlab;

  ;

  *++pvs_yyvsp = pvs_yylval;


  pvs_yystate = pvs_yyn;
  goto pvs_yynewstate;





pvs_yyacceptlab:
  pvs_yyresult = 0;
  goto pvs_yyreturn;




pvs_yyabortlab:
  pvs_yyresult = 1;
  goto pvs_yyreturn;





pvs_yyoverflowlab:
  pvs_yyerror ("parser stack overflow");
  pvs_yyresult = 2;



pvs_yyreturn:

  if (pvs_yyss != pvs_yyssa)
    do { ; } while (0);

  return pvs_yyresult;
}
# 108 "pvsparse.y"

static int lastchar=' ';
static char *input_string;
static char unput_buffer[100];
static int unput_pos=0;
static int is_pos=0;




static int str_input(void)
{
  if (input_string) {
    if (unput_pos)
      return unput_buffer[--unput_pos];
    else
      return input_string[is_pos++];
  } else {
    return 0;
  }
}

static void str_unput(char c)
{
  if (input_string) {
    unput_buffer[unput_pos++]=c;
  }
}

static void str_output(char c)
{
  _IO_putc (c, stdout);
}

static int str_wrapup(void)
{
  lastchar=' ';
  return 1;
}

int pvs_parse_string(char *str)
{
  input_string=str;
  is_pos=0;
  unput_pos=0;
  lastchar=' ';



  pvs_yyparse();
  return 1;
}

# 1 "pvsparselex.c" 1
# 24 "pvsparselex.c"
 int pvs_yyback(int *, int);
 int pvs_yyinput(void);
 int pvs_yylook(void);
 void pvs_yyoutput(int);
 int pvs_yyracc(int);
 int pvs_yyreject(void);
 void pvs_yyunput(int);
 int pvs_yylex(void);





 int pvs_yyless(int);


 int pvs_yywrap(void);
# 53 "pvsparselex.c"
 void exit(int);
# 68 "pvsparselex.c"
int pvs_yyleng;

char pvs_yytext[8192];
int pvs_yymorfg;
extern char *pvs_yysptr, pvs_yysbuf[];
int pvs_yytchar;
FILE *pvs_yyin = 0, *pvs_yyout = 0;
extern int pvs_yylineno;
struct pvs_yysvf {
 struct pvs_yywork *pvs_yystoff;
 struct pvs_yysvf *pvs_yyother;
 int *pvs_yystops;};
struct pvs_yysvf *pvs_yyestate;
extern struct pvs_yysvf pvs_yysvec[], *pvs_yybgin;
# 91 "pvsparselex.c"
pvs_yylex(){
int nstr; extern int pvs_yyprevious;





while((nstr = pvs_yylook()) >= 0)
pvs_yyfussy: switch(nstr){
case 0:
if(str_wrapup()) return(0); break;
case 1:
# 10 "pvsparse.lex"
    {

            lastchar=262;
            return (262); }
break;
case 2:
# 14 "pvsparse.lex"
   {

            lastchar=261;
            return (261); }
break;
case 3:
# 18 "pvsparse.lex"
       {

            lastchar=260;
            return (260); }
break;
case 4:
# 22 "pvsparse.lex"
       {

            lastchar=259;
            return (259); }
break;
case 5:
# 26 "pvsparse.lex"
      { lastchar=258;
     return (258); }
break;
case 6:
# 28 "pvsparse.lex"
{ Uchar buffer[512];int len;
            if (!strncmp(pvs_yytext, "I", pvs_yyleng)) {
       buffer[0]=0; len=0;
       make_node(0xF710, buffer,len, notation_with_number(id_uid), 0);
     } else {
       for (len=0; len<pvs_yyleng && len<511; len++) {
  buffer[len]=pvs_yytext[len];
       }
       buffer[len]=0;

       make_node(0xF720,buffer,len,0,0);
     }
            lastchar=266;
            return (266); }
break;
case 7:
# 42 "pvsparse.lex"
case 8:
# 43 "pvsparse.lex"
  { Uchar buffer[512]; int len;
        for (len=0;
      len<pvs_yyleng && len<511 && pvs_yytext[len]!='!';
      len++) {
   buffer[len]=pvs_yytext[len];
        }
        buffer[len]=0;
        make_node(0xF720,buffer,len,0,0);
        lastchar=265;
                      return (265); }
break;
case 9:
# 53 "pvsparse.lex"
       { if (lastchar==')') {



               lastchar=(263);
        str_unput('(');
               return (263);
             }
             lastchar='('; return '(';
           }
break;
case 10:
# 63 "pvsparse.lex"
{ lastchar=')'; return ')'; }
break;
case 11:
# 64 "pvsparse.lex"
   ;
break;
case 12:
# 65 "pvsparse.lex"
{ lastchar=pvs_yytext[0]; return pvs_yytext[0]; }
break;
case -1:
break;
default:
(void)fprintf(pvs_yyout,"bad switch pvs_yylook %d",nstr);
} return(0); }

int pvs_yyvstop[] = {
0,

6,
0,

6,
0,

12,
0,

11,
12,
0,

11,
0,

11,
12,
0,

12,
0,

9,
12,
0,

10,
12,
0,

3,
12,
0,

7,
12,
0,

4,
12,
0,

6,
12,
0,

6,
12,
0,

8,
0,

7,
0,

6,
0,

6,
0,

2,
0,

1,
0,

5,
6,
0,
0};

struct pvs_yywork { unsigned char verify, advance; } pvs_yycrank[] = {
0,0, 0,0, 1,3, 0,0,
0,0, 0,0, 0,0, 0,0,
0,0, 0,0, 1,4, 1,5,
0,0, 0,0, 0,0, 0,0,
0,0, 0,0, 0,0, 0,0,
0,0, 0,0, 0,0, 0,0,
0,0, 0,0, 0,0, 0,0,
0,0, 0,0, 0,0, 0,0,
0,0, 1,6, 1,7, 15,22,
16,23, 0,0, 0,0, 0,0,
0,0, 1,8, 1,9, 0,0,
1,10, 0,0, 0,0, 0,0,
0,0, 1,11, 0,0, 0,0,
0,0, 0,0, 0,0, 0,0,
0,0, 6,15, 0,0, 0,0,
0,0, 0,0, 1,12, 0,0,
0,0, 0,0, 1,13, 1,14,
1,14, 1,14, 1,14, 1,14,
1,14, 1,14, 1,14, 1,14,
1,14, 1,14, 1,14, 1,14,
1,14, 1,14, 1,14, 1,14,
1,14, 1,14, 1,14, 1,14,
1,14, 1,14, 1,14, 1,14,
14,20, 20,20, 24,20, 0,0,
0,0, 0,0, 1,14, 1,14,
1,14, 1,14, 1,14, 1,14,
1,14, 1,14, 1,14, 1,14,
1,14, 1,14, 1,14, 1,14,
1,14, 1,14, 1,14, 1,14,
1,14, 1,14, 1,14, 1,14,
1,14, 1,14, 1,14, 1,14,
2,6, 2,7, 6,16, 0,0,
0,0, 0,0, 0,0, 0,0,
0,0, 0,0, 0,0, 2,10,
7,17, 7,17, 7,17, 7,17,
7,17, 7,17, 7,17, 7,17,
7,17, 7,17, 21,24, 0,0,
0,0, 0,0, 0,0, 0,0,
0,0, 2,12, 0,0, 0,0,
21,20, 0,0, 2,14, 2,14,
2,14, 2,14, 2,14, 2,14,
2,14, 2,14, 2,14, 2,14,
2,14, 2,14, 2,14, 2,14,
2,14, 2,14, 2,14, 2,14,
2,14, 2,14, 2,14, 2,14,
2,14, 2,14, 2,14, 0,0,
0,0, 0,0, 0,0, 0,0,
0,0, 2,14, 2,14, 2,14,
2,14, 2,14, 2,14, 2,14,
2,14, 2,14, 2,14, 2,14,
2,14, 2,14, 2,14, 2,14,
2,14, 2,14, 2,14, 2,14,
2,14, 2,14, 2,14, 2,14,
2,14, 2,14, 2,14, 11,18,
11,18, 11,18, 11,18, 11,18,
11,18, 11,18, 11,18, 11,18,
11,18, 13,19, 0,0, 0,0,
0,0, 0,0, 0,0, 0,0,
0,0, 0,0, 0,0, 0,0,
0,0, 0,0, 0,0, 0,0,
0,0, 0,0, 0,0, 0,0,
0,0, 0,0, 0,0, 0,0,
0,0, 0,0, 0,0, 0,0,
0,0, 0,0, 0,0, 0,0,
0,0, 13,20, 13,20, 13,20,
13,20, 13,20, 13,20, 13,20,
13,20, 13,20, 13,20, 13,20,
13,20, 13,20, 13,21, 13,20,
13,20, 13,20, 13,20, 13,20,
13,20, 13,20, 13,20, 13,20,
13,20, 13,20, 13,20, 0,0,
0,0, 0,0, 0,0, 0,0,
0,0, 13,20, 13,20, 13,20,
13,20, 13,20, 13,20, 13,20,
13,20, 13,20, 13,20, 13,20,
13,20, 13,20, 13,20, 13,20,
13,20, 13,20, 13,20, 13,20,
13,20, 13,20, 13,20, 13,20,
13,20, 13,20, 13,20, 0,0,
0,0};
struct pvs_yysvf pvs_yysvec[] = {
0, 0, 0,
pvs_yycrank+-1, 0, pvs_yyvstop+1,
pvs_yycrank+-92, pvs_yysvec+1, pvs_yyvstop+3,
pvs_yycrank+0, 0, pvs_yyvstop+5,
pvs_yycrank+0, 0, pvs_yyvstop+7,
pvs_yycrank+0, 0, pvs_yyvstop+10,
pvs_yycrank+15, 0, pvs_yyvstop+12,
pvs_yycrank+88, 0, pvs_yyvstop+15,
pvs_yycrank+0, 0, pvs_yyvstop+17,
pvs_yycrank+0, 0, pvs_yyvstop+20,
pvs_yycrank+0, 0, pvs_yyvstop+23,
pvs_yycrank+167, 0, pvs_yyvstop+26,
pvs_yycrank+0, 0, pvs_yyvstop+29,
pvs_yycrank+192, 0, pvs_yyvstop+32,
pvs_yycrank+14, pvs_yysvec+13, pvs_yyvstop+35,
pvs_yycrank+3, 0, 0,
pvs_yycrank+4, 0, 0,
pvs_yycrank+0, pvs_yysvec+7, pvs_yyvstop+38,
pvs_yycrank+0, pvs_yysvec+11, pvs_yyvstop+40,
pvs_yycrank+0, pvs_yysvec+7, 0,
pvs_yycrank+15, pvs_yysvec+13, pvs_yyvstop+42,
pvs_yycrank+78, pvs_yysvec+13, pvs_yyvstop+44,
pvs_yycrank+0, 0, pvs_yyvstop+46,
pvs_yycrank+0, 0, pvs_yyvstop+48,
pvs_yycrank+16, pvs_yysvec+13, pvs_yyvstop+50,
0, 0, 0};
struct pvs_yywork *pvs_yytop = pvs_yycrank+314;
struct pvs_yysvf *pvs_yybgin = pvs_yysvec+1;
char pvs_yymatch[] = {
  0, 1, 1, 1, 1, 1, 1, 1,
  1, 9, 10, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  9, 1, 1, 1, 1, 1, 1, 1,
 40, 41, 1, 1, 1, 1, 1, 1,
 48, 48, 48, 48, 48, 48, 48, 48,
 48, 48, 1, 1, 1, 1, 1, 1,
  1, 65, 65, 65, 65, 65, 65, 65,
 65, 65, 65, 65, 65, 65, 65, 65,
 65, 65, 65, 65, 65, 65, 65, 65,
 65, 65, 65, 1, 1, 1, 1, 1,
  1, 65, 65, 65, 65, 65, 65, 65,
 65, 65, 65, 65, 65, 65, 65, 65,
 65, 65, 65, 65, 65, 65, 65, 65,
 65, 65, 65, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
  1, 1, 1, 1, 1, 1, 1, 1,
0};
char pvs_yyextra[] = {
0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,
0};







#pragma ident "@(#)ncform	6.11	97/01/06 SMI"

int pvs_yylineno =1;


struct pvs_yysvf *pvs_yylstate [8192], **pvs_yylsp, **pvs_yyolsp;
char pvs_yysbuf[8192];
char *pvs_yysptr = pvs_yysbuf;
int *pvs_yyfnd;
extern struct pvs_yysvf *pvs_yyestate;
int pvs_yyprevious = 10;

int pvs_yylook(void)



{
 register struct pvs_yysvf *pvs_yystate, **lsp;
 register struct pvs_yywork *pvs_yyt;
 struct pvs_yysvf *pvs_yyz;
 int pvs_yych, pvs_yyfirst;
 struct pvs_yywork *pvs_yyr;



 char *pvs_yylastch;




 pvs_yyfirst=1;
 if (!pvs_yymorfg)
  pvs_yylastch = pvs_yytext;
 else {
  pvs_yymorfg=0;
  pvs_yylastch = pvs_yytext+pvs_yyleng;
  }
 for(;;){
  lsp = pvs_yylstate;
  pvs_yyestate = pvs_yystate = pvs_yybgin;
  if (pvs_yyprevious==10) pvs_yystate++;
  for (;;){



   pvs_yyt = pvs_yystate->pvs_yystoff;
   if(pvs_yyt == pvs_yycrank && !pvs_yyfirst){
    pvs_yyz = pvs_yystate->pvs_yyother;
    if(pvs_yyz == 0)break;
    if(pvs_yyz->pvs_yystoff == pvs_yycrank)break;
    }

   *pvs_yylastch++ = pvs_yych = str_input();




   if(pvs_yylastch > &pvs_yytext[8192]) {
    fprintf(pvs_yyout,"Input string too long, limit %d\n",8192);
    exit(1);
   }
# 385 "pvsparse.lex"
   pvs_yyfirst=0;
  tryagain:







   pvs_yyr = pvs_yyt;
   if ( (int)pvs_yyt > (int)pvs_yycrank){
    pvs_yyt = pvs_yyr + pvs_yych;
    if (pvs_yyt <= pvs_yytop && pvs_yyt->verify+pvs_yysvec == pvs_yystate){
     if(pvs_yyt->advance+pvs_yysvec == pvs_yysvec)
      {str_unput(*--pvs_yylastch);break;}
     *lsp++ = pvs_yystate = pvs_yyt->advance+pvs_yysvec;
     if(lsp > &pvs_yylstate[8192]) {
      fprintf(pvs_yyout,"Input string too long, limit %d\n",8192);
      exit(1);
     }
     goto contin;
     }
    }

   else if((int)pvs_yyt < (int)pvs_yycrank) {
    pvs_yyt = pvs_yyr = pvs_yycrank+(pvs_yycrank-pvs_yyt);



    pvs_yyt = pvs_yyt + pvs_yych;
    if(pvs_yyt <= pvs_yytop && pvs_yyt->verify+pvs_yysvec == pvs_yystate){
     if(pvs_yyt->advance+pvs_yysvec == pvs_yysvec)
      {str_unput(*--pvs_yylastch);break;}
     *lsp++ = pvs_yystate = pvs_yyt->advance+pvs_yysvec;
     if(lsp > &pvs_yylstate[8192]) {
      fprintf(pvs_yyout,"Input string too long, limit %d\n",8192);
      exit(1);
     }
     goto contin;
     }
    pvs_yyt = pvs_yyr + pvs_yymatch[pvs_yych];







    if(pvs_yyt <= pvs_yytop && pvs_yyt->verify+pvs_yysvec == pvs_yystate){
     if(pvs_yyt->advance+pvs_yysvec == pvs_yysvec)
      {str_unput(*--pvs_yylastch);break;}
     *lsp++ = pvs_yystate = pvs_yyt->advance+pvs_yysvec;
     if(lsp > &pvs_yylstate[8192]) {
      fprintf(pvs_yyout,"Input string too long, limit %d\n",8192);
      exit(1);
     }
     goto contin;
     }
    }
   if ((pvs_yystate = pvs_yystate->pvs_yyother) && (pvs_yyt= pvs_yystate->pvs_yystoff) != pvs_yycrank){



    goto tryagain;
    }

   else
    {str_unput(*--pvs_yylastch);break;}
  contin:







   ;
   }







  while (lsp-- > pvs_yylstate){
   *pvs_yylastch-- = 0;
   if (*lsp != 0 && (pvs_yyfnd= (*lsp)->pvs_yystops) && *pvs_yyfnd > 0){
    pvs_yyolsp = lsp;
    if(pvs_yyextra[*pvs_yyfnd]){
     while(pvs_yyback((*lsp)->pvs_yystops,-*pvs_yyfnd) != 1 && lsp > pvs_yylstate){
      lsp--;
      str_unput(*pvs_yylastch--);
      }
     }
    pvs_yyprevious = *pvs_yylastch;
    pvs_yylsp = lsp;
    pvs_yyleng = pvs_yylastch-pvs_yytext+1;
    pvs_yytext[pvs_yyleng] = 0;







    return(*pvs_yyfnd++);
    }
   str_unput(*pvs_yylastch);
   }
  if (pvs_yytext[0] == 0 )
   {
   pvs_yysptr=pvs_yysbuf;
   return(0);
   }

  pvs_yyprevious = pvs_yytext[0] = str_input();
  if (pvs_yyprevious>0)
   str_output(pvs_yyprevious);





  pvs_yylastch=pvs_yytext;



  }
 }

int pvs_yyback(int *p, int m)




{
 if (p==0) return(0);
 while (*p) {
  if (*p++ == m)
   return(1);
 }
 return(0);
}


int pvs_yyinput(void)



{

 return(str_input());



 }

void pvs_yyoutput(int c)




{

 str_output(c);



 }

void pvs_yyunput(int c)




{
 str_unput(c);
 }
# 162 "pvsparse.y" 2
