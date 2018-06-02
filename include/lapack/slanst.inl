/* ../../../dependencies/lapack/src/slanst.f -- translated by f2c (version 20061008).
   You must link the resulting object file with libf2c:
        on Microsoft Windows system, link with libf2c.lib;
        on Linux or Unix systems, link with .../path/to/libf2c.a -lm
        or, if you install libf2c.a in a standard place, with -lf2c -lm
        -- in that order, at the end of the command line, as in
                cc *.o -lf2c -lm
        Source for libf2c is in /netlib/f2c/libf2c.zip, e.g.,

                http://www.netlib.org/f2c/libf2c.zip
*/




static doublereal slanst_(char *norm, integer *n, real *d__, real *e, ftnlen 
        norm_len)
{
/* Table of constant values */

static integer c__1 = 1;
    /* System generated locals */
    integer i__1;
    real ret_val, r__1, r__2, r__3, r__4, r__5;

    /* Builtin functions */
    double sqrt(doublereal);

    /* Local variables */
    static integer i__;
    static real sum, scale;
     logical lsame_(char *, char *, ftnlen, ftnlen);
    static real anorm;
     /* Subroutine */ int slassq_(integer *, real *, integer *, real *, 
            real *);


/*  -- LAPACK auxiliary routine (version 3.0) -- */
/*     Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd., */
/*     Courant Institute, Argonne National Lab, and Rice University */
/*     February 29, 1992 */

/*     .. Scalar Arguments .. */
/*     .. */
/*     .. Array Arguments .. */
/*     .. */

/*  Purpose */
/*  ======= */

/*  SLANST  returns the value of the one norm,  or the Frobenius norm, or */
/*  the  infinity norm,  or the  element of  largest absolute value  of a */
/*  real symmetric tridiagonal matrix A. */

/*  Description */
/*  =========== */

/*  SLANST returns the value */

/*     SLANST = ( MAX(ABS(A(i,j))), NORM = 'M' or 'm' */
/*              ( */
/*              ( norm1(A),         NORM = '1', 'O' or 'o' */
/*              ( */
/*              ( normI(A),         NORM = 'I' or 'i' */
/*              ( */
/*              ( normF(A),         NORM = 'F', 'f', 'E' or 'e' */

/*  where  norm1  denotes the  one norm of a matrix (maximum column sum), */
/*  normI  denotes the  infinity norm  of a matrix  (maximum row sum) and */
/*  normF  denotes the  Frobenius norm of a matrix (square root of sum of */
/*  squares).  Note that  MAX(ABS(A(i,j)))  is not a  matrix norm. */

/*  Arguments */
/*  ========= */

/*  NORM    (input) CHARACTER*1 */
/*          Specifies the value to be returned in SLANST as described */
/*          above. */

/*  N       (input) INTEGER */
/*          The order of the matrix A.  N >= 0.  When N = 0, SLANST is */
/*          set to zero. */

/*  D       (input) REAL array, dimension (N) */
/*          The diagonal elements of A. */

/*  E       (input) REAL array, dimension (N-1) */
/*          The (n-1) sub-diagonal or super-diagonal elements of A. */

/*  ===================================================================== */

/*     .. Parameters .. */
/*     .. */
/*     .. Local Scalars .. */
/*     .. */
/*     .. External Functions .. */
/*     .. */
/*     .. External Subroutines .. */
/*     .. */
/*     .. Intrinsic Functions .. */
/*     .. */
/*     .. Executable Statements .. */

    /* Parameter adjustments */
    --e;
    --d__;

    /* Function Body */
    if (*n <= 0) {
        anorm = 0.f;
    } else if (lsame_(norm, "M", (ftnlen)1, (ftnlen)1)) {

/*        Find MAX(ABS(A(i,j))). */

        anorm = (r__1 = d__[*n], SABS(r__1));
        i__1 = *n - 1;
        for (i__ = 1; i__ <= i__1; ++i__) {
/* Computing MAX */
            r__2 = anorm, r__3 = (r__1 = d__[i__], SABS(r__1));
            anorm = dmax(r__2,r__3);
/* Computing MAX */
            r__2 = anorm, r__3 = (r__1 = e[i__], SABS(r__1));
            anorm = dmax(r__2,r__3);
/* L10: */
        }
    } else if (lsame_(norm, "O", (ftnlen)1, (ftnlen)1) || *(unsigned char *)
            norm == '1' || lsame_(norm, "I", (ftnlen)1, (ftnlen)1)) {

/*        Find norm1(A). */

        if (*n == 1) {
            anorm = SABS(d__[1]);
        } else {
/* Computing MAX */
            r__3 = SABS(d__[1]) + SABS(e[1]), r__4 = (r__1 = e[*n - 1], SABS(
                    r__1)) + (r__2 = d__[*n], SABS(r__2));
            anorm = dmax(r__3,r__4);
            i__1 = *n - 1;
            for (i__ = 2; i__ <= i__1; ++i__) {
/* Computing MAX */
                r__4 = anorm, r__5 = (r__1 = d__[i__], SABS(r__1)) + (r__2 = 
                        e[i__], SABS(r__2)) + (r__3 = e[i__ - 1], SABS(r__3));
                anorm = dmax(r__4,r__5);
/* L20: */
            }
        }
    } else if (lsame_(norm, "F", (ftnlen)1, (ftnlen)1) || lsame_(norm, "E", (
            ftnlen)1, (ftnlen)1)) {

/*        Find normF(A). */

        scale = 0.f;
        sum = 1.f;
        if (*n > 1) {
            i__1 = *n - 1;
            slassq_(&i__1, &e[1], &c__1, &scale, &sum);
            sum *= 2;
        }
        slassq_(n, &d__[1], &c__1, &scale, &sum);
        anorm = (real)(scale * sqrt(sum));
    }

    ret_val = anorm;
    return ret_val;

/*     End of SLANST */

} /* slanst_ */
