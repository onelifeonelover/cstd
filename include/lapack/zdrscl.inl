/* ../../../dependencies/lapack/src/zdrscl.f -- translated by f2c (version 20061008).
   You must link the resulting object file with libf2c:
        on Microsoft Windows system, link with libf2c.lib;
        on Linux or Unix systems, link with .../path/to/libf2c.a -lm
        or, if you install libf2c.a in a standard place, with -lf2c -lm
        -- in that order, at the end of the command line, as in
                cc *.o -lf2c -lm
        Source for libf2c is in /netlib/f2c/libf2c.zip, e.g.,

                http://www.netlib.org/f2c/libf2c.zip
*/



/* Subroutine */ static int zdrscl_(integer *n, doublereal *sa, dcomplex *sx, 
        integer *incx)
{
    static doublereal mul, cden;
    static logical done;
    static doublereal cnum, cden1, cnum1;
     /* Subroutine */ int dlabad_(doublereal *, doublereal *);
     doublereal dlamch_(char *, ftnlen);
     /* Subroutine */ int zdscal_(integer *, doublereal *, 
            dcomplex *, integer *);
    static doublereal bignum, smlnum;


/*  -- LAPACK auxiliary routine (version 3.0) -- */
/*     Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd., */
/*     Courant Institute, Argonne National Lab, and Rice University */
/*     September 30, 1994 */

/*     .. Scalar Arguments .. */
/*     .. */
/*     .. Array Arguments .. */
/*     .. */

/*  Purpose */
/*  ======= */

/*  ZDRSCL multiplies an n-element scomplex vector x by the real scalar */
/*  1/a.  This is done without overflow or underflow as long as */
/*  the final result x/a does not overflow or underflow. */

/*  Arguments */
/*  ========= */

/*  N       (input) INTEGER */
/*          The number of components of the vector x. */

/*  SA      (input) DOUBLE PRECISION */
/*          The scalar a which is used to divide each component of x. */
/*          SA must be >= 0, or the subroutine will divide by zero. */

/*  SX      (input/output) COMPLEX*16 array, dimension */
/*                         (1+(N-1)*ABS(INCX)) */
/*          The n-element vector x. */

/*  INCX    (input) INTEGER */
/*          The increment between successive values of the vector SX. */
/*          > 0:  SX(1) = X(1) and SX(1+(i-1)*INCX) = x(i),     1< i<= n */

/* ===================================================================== */

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

/*     Quick return if possible */

    /* Parameter adjustments */
    --sx;

    /* Function Body */
    if (*n <= 0) {
        return 0;
    }

/*     Get machine parameters */

    smlnum = dlamch_("S", (ftnlen)1);
    bignum = 1. / smlnum;
    dlabad_(&smlnum, &bignum);

/*     Initialize the denominator to SA and the numerator to 1. */

    cden = *sa;
    cnum = 1.;

L10:
    cden1 = cden * smlnum;
    cnum1 = cnum / bignum;
    if (ABS(cden1) > ABS(cnum) && cnum != 0.) {

/*        Pre-multiply X by SMLNUM if CDEN is large compared to CNUM. */

        mul = smlnum;
        done = FALSE_;
        cden = cden1;
    } else if (ABS(cnum1) > ABS(cden)) {

/*        Pre-multiply X by BIGNUM if CDEN is small1 compared to CNUM. */

        mul = bignum;
        done = FALSE_;
        cnum = cnum1;
    } else {

/*        Multiply X by CNUM / CDEN and return. */

        mul = cnum / cden;
        done = TRUE_;
    }

/*     Scale the vector X by MUL */

    zdscal_(n, &mul, &sx[1], incx);

    if (! done) {
        goto L10;
    }

    return 0;

/*     End of ZDRSCL */

} /* zdrscl_ */

