

#include <ctype.h>

#ifdef XDOUBLE
#define ERROR_NAME "QSPMV "
#elif defined(DOUBLE)
#define ERROR_NAME "DSPMV "
#else
#define ERROR_NAME "SSPMV "
#endif

static  int (*spmv[])(BLASLONG, FLOAT, FLOAT *, FLOAT *, BLASLONG, FLOAT *, BLASLONG, void *) = {
#ifdef XDOUBLE
  qspmv_U, qspmv_L,
#elif defined(DOUBLE)
  dspmv_U, dspmv_L,
#else
  sspmv_U, sspmv_L,
#endif
};

#ifdef SMPTEST
static  int (*spmv_thread[])(BLASLONG, FLOAT, FLOAT *, FLOAT *, BLASLONG, FLOAT *, BLASLONG, FLOAT *, int) = {
#ifdef XDOUBLE
  qspmv_thread_U, qspmv_thread_L,
#elif defined(DOUBLE)
  dspmv_thread_U, dspmv_thread_L,
#else
  sspmv_thread_U, sspmv_thread_L,
#endif
};
#endif

#ifndef CBLAS

void NAME(char *UPLO, blasint *N, FLOAT  *ALPHA, FLOAT *a,
            FLOAT  *x, blasint *INCX, FLOAT *BETA, FLOAT *y, blasint *INCY){

  char uplo_arg = *UPLO;
  blasint n		= *N;
  FLOAT alpha	= *ALPHA;
  blasint incx	= *INCX;
  FLOAT beta	= *BETA;
  blasint incy	= *INCY;

  blasint info;
  int uplo;
  FLOAT *buffer;
#ifdef SMPTEST
  int nthreads;
#endif

  PRINT_DEBUG_NAME;

  TOUPPER(uplo_arg);
  uplo  = -1;

  if (uplo_arg  == 'U') uplo  = 0;
  if (uplo_arg  == 'L') uplo  = 1;

  info = 0;

  if (incy == 0)          info =  9;
  if (incx == 0)          info =  6;
  if (n < 0)              info =  2;
  if (uplo  < 0)          info =  1;

  if (info != 0) {
    BLASFUNC(xerbla)(ERROR_NAME, &info, sizeof(ERROR_NAME));
    return;
  }

#else

void CNAME(enum CBLAS_ORDER order,
	   enum CBLAS_UPLO Uplo,
	   blasint n,
	   FLOAT alpha,
	   FLOAT  *a,
	   FLOAT  *x, blasint incx,
	   FLOAT beta,
	   FLOAT  *y, blasint incy){

  FLOAT *buffer;
  int uplo;
  blasint info;
#ifdef SMPTEST
  int nthreads;
#endif

  PRINT_DEBUG_CNAME;

  uplo = -1;
  info =  0;

  if (order == CblasColMajor) {
    if (Uplo == CblasUpper)         uplo  = 0;
    if (Uplo == CblasLower)         uplo  = 1;

    info = -1;

    if (incy == 0)          info =  9;
    if (incx == 0)          info =  6;
    if (n < 0)              info =  2;
    if (uplo  < 0)          info =  1;
  }

  if (order == CblasRowMajor) {
    if (Uplo == CblasUpper)         uplo  = 1;
    if (Uplo == CblasLower)         uplo  = 0;

    info = -1;

    if (incy == 0)          info =  9;
    if (incx == 0)          info =  6;
    if (n < 0)              info =  2;
    if (uplo  < 0)          info =  1;
  }

  if (info >= 0) {
    BLASFUNC(xerbla)(ERROR_NAME, &info, sizeof(ERROR_NAME));
    return;
  }

#endif

  if (n == 0) return;

  if (beta != ONE) SCAL_K(n, 0, 0, beta, y, abs(incy), NULL, 0, NULL, 0);

  if (alpha == ZERO) return;

  IDEBUG_START;

  FUNCTION_PROFILE_START();

  if (incx < 0 ) x -= (n - 1) * incx;
  if (incy < 0 ) y -= (n - 1) * incy;

  buffer = (FLOAT *)blas_memory_alloc(1);

#ifdef SMPTEST
  nthreads = num_cpu_avail(2);

  if (nthreads == 1) {
#endif

  (spmv[uplo])(n, alpha, a, x, incx, y, incy, buffer);

#ifdef SMPTEST
  } else {

    (spmv_thread[uplo])(n, alpha, a, x, incx, y, incy, buffer, nthreads);

  }
#endif

  blas_memory_free(buffer);

  FUNCTION_PROFILE_END(1, n * n / 2 + n, n * n);

  IDEBUG_END;

  return;
}