
#ifndef COMMON_H
#define COMMON_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#ifndef __USE_XOPEN
#define __USE_XOPEN
#endif

#ifndef __USE_SVID
#define __USE_SVID
#endif

#ifdef BUILD_KERNEL
#include "config_kernel.h"
#else
#include "config.h"
#endif

#undef ENABLE_SSE_EXCEPTION

#if defined(SMP_SERVER) || defined(SMP_ONDEMAND)
#define SMP
#endif

#if defined(OS_WINNT) || defined(OS_CYGWIN_NT) || defined(OS_INTERIX)
#define WINDOWS_ABI
#define OS_WINDOWS

#endif

#if !defined(NOINCLUDE) && !defined(ASSEMBLER)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if !defined(_MSC_VER)
#include <unistd.h>
#endif
#include <time.h>

#ifdef OS_LINUX
#include <malloc.h>
#include <sched.h>
#endif

#if defined(OS_DARWIN) || defined(OS_FREEBSD) || defined(OS_NETBSD) || defined(OS_ANDROID)
#include <sched.h>
#endif

#ifdef OS_ANDROID
#define NO_SYSV_IPC
//Android NDK only supports complex.h since Android 5.0
#if __ANDROID_API__ < 21
#define FORCE_OPENBLAS_COMPLEX_STRUCT
#endif
#endif

#ifdef OS_WINDOWS
#ifdef  ATOM
#define GOTO_ATOM ATOM
#undef  ATOM
#endif
#include <windows.h>
#include <math.h>
#ifdef  GOTO_ATOM
#define ATOM GOTO_ATOM
#undef  GOTO_ATOM
#endif
#else
#include <sys/mman.h>
#ifndef NO_SYSV_IPC
#include <sys/shm.h>
#endif
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <math.h>
#ifdef SMP
#include <pthread.h>
#endif
#endif

#if defined(OS_SUNOS)
#include <thread.h>
#endif

#ifdef __DECC
#include <c_asm.h>
#include <machine/builtins.h>
#endif

#if defined(ARCH_IA64) && defined(ENABLE_SSE_EXCEPTION)
#include <fenv.h>
#endif

#endif

#undef DEBUG_INFO
#define SMP_DEBUG
#undef MALLOC_DEBUG
#undef SMP_ALLOC_DEBUG


#define BITMASK(a, b, c) ((((a) >> (b)) & (c)))

#define ALLOCA_ALIGN 63UL

#define NUM_BUFFERS (MAX_CPU_NUMBER * 2)

#ifdef NEEDBUNDERSCORE
#define BLASFUNC(FUNC) FUNC
#else
#define BLASFUNC(FUNC) FUNC
#endif

#undef	USE_PTHREAD_LOCK
#undef	USE_PTHREAD_SPINLOCK

#if defined(USE_PTHREAD_LOCK) && defined(USE_PTHREAD_SPINLOCK)
#error "You can't specify both LOCK operation!"
#endif

#ifdef SMP
#define USE_PTHREAD_LOCK
#undef	USE_PTHREAD_SPINLOCK
#endif

#ifdef OS_WINDOWS
#undef	USE_PTHREAD_LOCK
#undef	USE_PTHREAD_SPINLOCK
#endif

#if   defined(USE_PTHREAD_LOCK)
#define   LOCK_COMMAND(x)   pthread_mutex_lock(x)
#define UNLOCK_COMMAND(x)   pthread_mutex_unlock(x)
#elif defined(USE_PTHREAD_SPINLOCK)
#ifndef ASSEMBLER
typedef volatile int pthread_spinlock_t;
int pthread_spin_lock (pthread_spinlock_t *__lock);
int pthread_spin_unlock (pthread_spinlock_t *__lock);
#endif
#define   LOCK_COMMAND(x)   pthread_spin_lock(x)
#define UNLOCK_COMMAND(x)   pthread_spin_unlock(x)
#else
#define   LOCK_COMMAND(x)   blas_lock(x)
#define UNLOCK_COMMAND(x)   blas_unlock(x)
#endif

#define GOTO_SHMID	0x510510

#if 0
#ifndef __CUDACC__
#define __global__
#define __device__
#define __host__
#define __shared__
#endif
#endif

#ifndef ASSEMBLER

#ifdef QUAD_PRECISION
typedef struct {
  unsigned long x[2];
}  xdouble;
#elif defined EXPRECISION
#define xdouble long double
#else
#define xdouble double
#endif

#if defined(OS_WINDOWS) && defined(__64BIT__)
typedef long long BLASLONG;
typedef unsigned long long BLASULONG;
#else
typedef long BLASLONG;
typedef unsigned long BLASULONG;
#endif

#ifdef USE64BITINT
typedef BLASLONG blasint;
#else
typedef int blasint;
#endif
#else
#ifdef USE64BITINT
#define INTSHIFT	3
#define INTSIZE		8
#else
#define INTSHIFT	2
#define INTSIZE		4
#endif
#endif


#define Address_H(x) (((x)+(1<<15))>>16)
#define Address_L(x) ((x)-((Address_H(x))<<16))

#ifndef MAX_CPU_NUMBER
#define MAX_CPU_NUMBER 2
#endif

#if defined(OS_SUNOS)
#define YIELDING	thr_yield()
#endif

#if defined(OS_WINDOWS)
#if defined(_MSC_VER) && !defined(__clang__)
#define YIELDING    YieldProcessor()
#else
#define YIELDING	SwitchToThread()
#endif
#endif

#if defined(ARMV7) || defined(ARMV6) || defined(ARMV8) || defined(ARMV5)
#define YIELDING        asm volatile ("nop;nop;nop;nop;nop;nop;nop;nop; \n");
#endif

#ifdef BULLDOZER
#ifndef YIELDING
#define YIELDING        __asm__ __volatile__ ("nop;nop;nop;nop;nop;nop;nop;nop;\n");
#endif
#endif

#ifdef POWER8
#ifndef YIELDING
#define YIELDING        __asm__ __volatile__ ("nop;nop;nop;nop;nop;nop;nop;nop;\n");
#endif
#endif


/*
#ifdef PILEDRIVER
#ifndef YIELDING
#define YIELDING        __asm__ __volatile__ ("nop;nop;nop;nop;nop;nop;nop;nop;\n");
#endif
#endif
*/

/*
#ifdef STEAMROLLER
#ifndef YIELDING
#define YIELDING        __asm__ __volatile__ ("nop;nop;nop;nop;nop;nop;nop;nop;\n");
#endif
#endif
*/

#ifndef YIELDING
#define YIELDING	sched_yield()
#endif

/***
To alloc job_t on heap or statck.
please https://github.com/xianyi/OpenBLAS/issues/246
***/
#if defined(OS_WINDOWS)
#define GETRF_MEM_ALLOC_THRESHOLD 32
#define BLAS3_MEM_ALLOC_THRESHOLD 32
#endif

#ifndef GETRF_MEM_ALLOC_THRESHOLD
#define GETRF_MEM_ALLOC_THRESHOLD 80
#endif

#ifndef BLAS3_MEM_ALLOC_THRESHOLD
#define BLAS3_MEM_ALLOC_THRESHOLD 160
#endif

#ifdef QUAD_PRECISION
#include "common_quad.h"
#endif

#ifdef ARCH_ALPHA
#include "common_alpha.h"
#endif

#ifdef ARCH_X86
#include "common_x86.h"
#endif

#ifdef ARCH_X86_64
#include "common_x86_64.h"
#endif

#ifdef ARCH_IA64
#include "common_ia64.h"
#endif

#ifdef ARCH_POWER
#include "common_power.h"
#endif

#ifdef sparc
#include "common_sparc.h"
#endif

#ifdef ARCH_MIPS
#include "common_mips.h"
#endif

#ifdef ARCH_MIPS64
#include "common_mips64.h"
#endif

#ifdef ARCH_ARM
#include "common_arm.h"
#endif

#ifdef ARCH_ARM64
#include "common_arm64.h"
#endif

#ifndef ASSEMBLER
#ifdef OS_WINDOWS
typedef char env_var_t[MAX_PATH];
#define readenv(p, n) GetEnvironmentVariable((LPCTSTR)(n), (LPTSTR)(p), sizeof(p))
#else
typedef char* env_var_t;
#define readenv(p, n) ((p)=getenv(n))
#endif

#if !defined(RPCC_DEFINED) && !defined(OS_WINDOWS)
#ifdef _POSIX_MONOTONIC_CLOCK
#if defined(__GLIBC_PREREQ) // cut the if condition if two lines, otherwise will fail at __GLIBC_PREREQ(2, 17)
#if __GLIBC_PREREQ(2, 17) // don't require -lrt
#define USE_MONOTONIC
#endif
#elif defined(OS_ANDROID)
#define USE_MONOTONIC
#endif
#endif
/* use similar scale as x86 rdtsc for timeouts to work correctly */
static inline unsigned long long rpcc(void){
#ifdef USE_MONOTONIC
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (unsigned long long)ts.tv_sec * 1000000000ull + ts.tv_nsec;
#else
  struct timeval tv;
  gettimeofday(&tv,NULL);
  return (unsigned long long)tv.tv_sec * 1000000000ull + tv.tv_usec * 1000;
#endif
}
#define RPCC_DEFINED
#define RPCC64BIT
#endif // !RPCC_DEFINED

#if !defined(BLAS_LOCK_DEFINED) && defined(__GNUC__)
static void __inline blas_lock(volatile BLASULONG *address){

  do {
    while (*address) {YIELDING;};

  } while (!__sync_bool_compare_and_swap(address, 0, 1));
}
#define BLAS_LOCK_DEFINED
#endif

#ifndef RPCC_DEFINED
#error "rpcc() implementation is missing for your platform"
#endif
#ifndef BLAS_LOCK_DEFINED
#error "blas_lock() implementation is missing for your platform"
#endif
#endif // !ASSEMBLER

#ifdef OS_LINUX
#include "common_linux.h"
#endif

#define MMAP_ACCESS (PROT_READ | PROT_WRITE)

#ifdef __NetBSD__
#define MMAP_POLICY (MAP_PRIVATE | MAP_ANON)
#else
#define MMAP_POLICY (MAP_PRIVATE | MAP_ANONYMOUS)
#endif

#include "param.h"
#include "common_param.h"

#ifndef STDERR
#define STDERR stderr
#endif

#ifndef MASK
#define MASK(a, b) (((a) + ((b) - 1)) & ~((b) - 1))
#endif

#if defined(XDOUBLE) || defined(DOUBLE)
#define FLOATRET	FLOAT
#else
#ifdef NEED_F2CCONV
#define FLOATRET	double
#else
#define FLOATRET	float
#endif
#endif

#ifndef ASSEMBLER
#ifndef NOINCLUDE
/* Inclusion of a standard header file is needed for definition of __STDC_*
   predefined macros with some compilers (e.g. GCC 4.7 on Linux).  This occurs
   as a side effect of including either <features.h> or <stdc-predef.h>. */
#include <stdio.h>
#endif  // NOINCLUDE

/* C99 supports complex floating numbers natively, which GCC also offers as an
   extension since version 3.0.  If neither are available, use a compatible
   structure as fallback (see Clause 6.2.5.13 of the C99 standard). */
#if ((defined(__STDC_IEC_559_COMPLEX__) || __STDC_VERSION__ >= 199901L || \
      (__GNUC__ >= 3 && !defined(__cplusplus))) && !(defined(FORCE_OPENBLAS_COMPLEX_STRUCT)))
  #define OPENBLAS_COMPLEX_C99
  #ifndef __cplusplus
    #include <complex.h>
  #endif
  typedef float _Complex openblas_complex_float;
  typedef double _Complex openblas_complex_double;
  typedef xdouble _Complex openblas_complex_xdouble;
  #define openblas_make_complex_float(real, imag)    ((real) + ((imag) * _Complex_I))
  #define openblas_make_complex_double(real, imag)   ((real) + ((imag) * _Complex_I))
  #define openblas_make_complex_xdouble(real, imag)  ((real) + ((imag) * _Complex_I))
#else
  #define OPENBLAS_COMPLEX_STRUCT
  typedef struct { float real, imag; } openblas_complex_float;
  typedef struct { double real, imag; } openblas_complex_double;
  typedef struct { xdouble real, imag; } openblas_complex_xdouble;
  #define openblas_make_complex_float(real, imag)    {(real), (imag)}
  #define openblas_make_complex_double(real, imag)   {(real), (imag)}
  #define openblas_make_complex_xdouble(real, imag)  {(real), (imag)}
#endif

#ifdef XDOUBLE
#define OPENBLAS_COMPLEX_FLOAT openblas_complex_xdouble
#define OPENBLAS_MAKE_COMPLEX_FLOAT(r,i) openblas_make_complex_xdouble(r,i)
#elif defined(DOUBLE)
#define OPENBLAS_COMPLEX_FLOAT openblas_complex_double
#define OPENBLAS_MAKE_COMPLEX_FLOAT(r,i) openblas_make_complex_double(r,i)
#else
#define OPENBLAS_COMPLEX_FLOAT openblas_complex_float
#define OPENBLAS_MAKE_COMPLEX_FLOAT(r,i) openblas_make_complex_float(r,i)
#endif

#if defined(C_PGI) || defined(C_SUN)
#define CREAL(X)	(*((FLOAT *)&X + 0))
#define CIMAG(X)	(*((FLOAT *)&X + 1))
#else
#ifdef OPENBLAS_COMPLEX_STRUCT
#define CREAL(Z)	((Z).real)
#define CIMAG(Z)	((Z).imag)
#else
#define CREAL	__real__
#define CIMAG	__imag__
#endif
#endif

#endif  // ASSEMBLER

#ifndef IFLUSH
#define IFLUSH
#endif

#ifndef IFLUSH_HALF
#define IFLUSH_HALF
#endif

#if defined(C_GCC) && (( __GNUC__ <= 3) || ((__GNUC__ == 4) && (__GNUC_MINOR__ < 2)))
#ifdef USE_OPENMP
#undef USE_OPENMP
#endif
#endif

#if defined(C_MSVC)
#define inline __inline
#endif

#ifndef ASSEMBLER

#ifndef MIN
#define MIN(a,b)   (a>b? b:a)
#endif

#ifndef MAX
#define MAX(a,b)   (a<b? b:a)
#endif

#define TOUPPER(a) {if ((a) > 0x60) (a) -= 0x20;}

#if defined(__FreeBSD__) || defined(__APPLE__)
#define MAP_ANONYMOUS MAP_ANON
#endif

/* Common Memory Management Routine */
void  blas_set_parameter(void);
int   blas_get_cpu_number(void);
void *blas_memory_alloc  (int);
void  blas_memory_free   (void *);
void *blas_memory_alloc_nolock  (int); //use malloc without blas_lock
void  blas_memory_free_nolock   (void *);

int  get_num_procs (void);

#if defined(OS_LINUX) && defined(SMP) && !defined(NO_AFFINITY)
int  get_num_nodes (void);
int get_num_proc   (int);
int get_node_equal (void);
#endif

void goto_set_num_threads(int);

void gotoblas_affinity_init(void);
void gotoblas_affinity_quit(void);
void gotoblas_dynamic_init(void);
void gotoblas_dynamic_quit(void);
void gotoblas_profile_init(void);
void gotoblas_profile_quit(void);

#ifdef USE_OPENMP
#ifndef C_MSVC
int omp_in_parallel(void);
int omp_get_num_procs(void);
#else
__declspec(dllimport) int __cdecl omp_in_parallel(void);
__declspec(dllimport) int __cdecl omp_get_num_procs(void);
#endif
#else
#ifdef __ELF__
int omp_in_parallel  (void) __attribute__ ((weak));
int omp_get_num_procs(void) __attribute__ ((weak));
#endif
#endif

static __inline void blas_unlock(volatile BLASULONG *address){
  MB;
  *address = 0;
}


#ifdef OS_WINDOWS
static __inline int readenv_atoi(char *env) {
  env_var_t p;
  return readenv(p,env) ? 0 : atoi(p);
}
#else
static __inline int readenv_atoi(char *env) {
  char *p;
  if (( p = getenv(env) ))
  	return (atoi(p));
  else
	return(0);
}
#endif

#ifdef MALLOC_DEBUG
void *blas_debug_alloc(int);
void *blas_debug_free(void *);
#undef malloc
#undef free
#define malloc(a) blas_debug_alloc(a)
#define free(a)   blas_debug_free (a)
#endif

#ifndef COPYOVERHEAD
#define GEMMRETTYPE  int
#else

typedef struct {
  double outercopy;
  double innercopy;
  double kernel;
  double mflops;
} copyoverhead_t;

#define GEMMRETTYPE  copyoverhead_t
#endif
#endif

#ifndef BUILD_KERNEL
#define KNAME(A, B) A
#else
#define KNAME(A, B) A##B
#endif

#include "common_interface.h"
#ifdef SANITY_CHECK
#include "common_reference.h"
#endif

#ifndef ASSEMBLER
#if defined(ARCH_X86) || defined(ARCH_X86_64) || defined(ARCH_IA64) || defined(ARCH_MIPS64)
extern BLASLONG gemm_offset_a;
extern BLASLONG gemm_offset_b;
extern BLASLONG sgemm_p;
extern BLASLONG sgemm_q;
extern BLASLONG sgemm_r;
extern BLASLONG dgemm_p;
extern BLASLONG dgemm_q;
extern BLASLONG dgemm_r;
extern BLASLONG qgemm_p;
extern BLASLONG qgemm_q;
extern BLASLONG qgemm_r;
extern BLASLONG cgemm_p;
extern BLASLONG cgemm_q;
extern BLASLONG cgemm_r;
extern BLASLONG zgemm_p;
extern BLASLONG zgemm_q;
extern BLASLONG zgemm_r;
extern BLASLONG xgemm_p;
extern BLASLONG xgemm_q;
extern BLASLONG xgemm_r;
#endif

typedef struct {
  void *a, *b, *c, *d, *alpha, *beta;
  BLASLONG	m, n, k, lda, ldb, ldc, ldd;

#ifdef SMP
  void *common;
  BLASLONG nthreads;
#endif

#ifdef PARAMTEST
  BLASLONG gemm_p, gemm_q, gemm_r;
#endif

#ifdef PREFETCHTEST
  BLASLONG prea, preb, prec, pred;
#endif

} blas_arg_t;
#endif


#include "common_s.h"
#include "common_d.h"
#include "common_q.h"

#include "common_c.h"
#include "common_z.h"
#include "common_x.h"


//#include "common_macro.h"
#include "common_level1.h"
#include "common_level2.h"
#include "common_level3.h"
#include "common_lapack.h"

#ifdef CBLAS
# define OPENBLAS_CONST     /* see comment in cblas.h */
# include "cblas.h"
#endif

#ifndef ASSEMBLER
#include "common_stackalloc.h"
#if 0
#include "symcopy.h"
#endif

#if defined(SMP_SERVER) && defined(SMP_ONDEMAND)
#error Both SMP_SERVER and SMP_ONDEMAND are specified.
#endif

#if defined(SMP_SERVER) || defined(SMP_ONDEMAND)
#include "common_thread.h"
#endif

#endif

#define INFO_NUM 99

#ifndef DEFAULT_CPU_NUMBER
#define DEFAULT_CPU_NUMBER 4
#endif

#ifndef IDEBUG_START
#define IDEBUG_START
#endif

#ifndef IDEBUG_END
#define IDEBUG_END
#endif

#if !defined(ASSEMBLER) && defined(FUNCTION_PROFILE)

typedef struct {
  int func;
  unsigned long long calls, fops, area, cycles, tcycles;
} func_profile_t;

extern func_profile_t function_profile_table[];
extern int gotoblas_profile;

#define FUNCTION_PROFILE_START() { unsigned long long profile_start = rpcc(), profile_end;
#ifdef SMP
#define FUNCTION_PROFILE_END(COMP, AREA, OPS) \
	if (gotoblas_profile) { \
	profile_end = rpcc(); \
	function_profile_table[PROFILE_FUNC_NAME].calls ++; \
	function_profile_table[PROFILE_FUNC_NAME].area    += SIZE * COMPSIZE * (AREA); \
	function_profile_table[PROFILE_FUNC_NAME].fops    += (COMP) * (OPS) / NUMOPT; \
	function_profile_table[PROFILE_FUNC_NAME].cycles  += (profile_end - profile_start); \
	function_profile_table[PROFILE_FUNC_NAME].tcycles += blas_cpu_number * (profile_end - profile_start); \
	} \
	}
#else
#define FUNCTION_PROFILE_END(COMP, AREA, OPS) \
	if (gotoblas_profile) { \
	profile_end = rpcc(); \
	function_profile_table[PROFILE_FUNC_NAME].calls ++; \
	function_profile_table[PROFILE_FUNC_NAME].area    += SIZE * COMPSIZE * (AREA); \
	function_profile_table[PROFILE_FUNC_NAME].fops    += (COMP) * (OPS) / NUMOPT; \
	function_profile_table[PROFILE_FUNC_NAME].cycles  += (profile_end - profile_start); \
	function_profile_table[PROFILE_FUNC_NAME].tcycles += (profile_end - profile_start); \
	} \
	}
#endif

#else
#define FUNCTION_PROFILE_START()
#define FUNCTION_PROFILE_END(COMP, AREA, OPS)
#endif

#if 1
#define PRINT_DEBUG_CNAME
#define PRINT_DEBUG_NAME
#else
#define PRINT_DEBUG_CNAME if (readenv_atoi("GOTO_DEBUG")) fprintf(stderr, "GotoBLAS : %s\n", CHAR_CNAME)
#define PRINT_DEBUG_NAME  if (readenv_atoi("GOTO_DEBUG")) fprintf(stderr, "GotoBLAS : %s\n", CHAR_NAME)
#endif


#endif