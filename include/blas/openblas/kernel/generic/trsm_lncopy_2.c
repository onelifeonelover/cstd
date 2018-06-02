



#ifndef UNIT
#define INV(a) (ONE / (a))
#else
#define INV(a) (ONE)
#endif

int CNAME(BLASLONG m, BLASLONG n, FLOAT *a, BLASLONG lda, BLASLONG offset, FLOAT *b){

  BLASLONG i, ii, j, jj;

  FLOAT data01, data02, data03, data04;
  FLOAT *a1, *a2;

  jj = offset;

  j = (n >> 1);
  while (j > 0){

    a1 = a + 0 * lda;
    a2 = a + 1 * lda;

    i = (m >> 1);
    ii = 0;
    while (i > 0) {

      if (ii == jj) {
#ifndef UNIT
	data01 = *(a1 + 0);
#endif
	data02 = *(a1 + 1);

#ifndef UNIT
	data04 = *(a2 + 1);
#endif

	*(b +  0) = INV(data01);
	*(b +  2) = data02;
	*(b +  3) = INV(data04);
      }

      if (ii > jj) {

	data01 = *(a1 + 0);
	data02 = *(a1 + 1);
	data03 = *(a2 + 0);
	data04 = *(a2 + 1);

	*(b +  0) = data01;
	*(b +  1) = data03;
	*(b +  2) = data02;
	*(b +  3) = data04;
      }

      a1 += 2;
      a2 += 2;
      b += 4;

      i  --;
      ii += 2;
    }

    if ((m & 1) != 0) {

      if (ii== jj) {
#ifndef UNIT
	data01 = *(a1 + 0);
#endif
	*(b +  0) = INV(data01);
      }

      if (ii > jj)  {
	data01 = *(a1 + 0);
	data02 = *(a2 + 0);

	*(b +  0) = data01;
	*(b +  1) = data02;
      }
      b += 2;
    }

    a += 2 * lda;
    jj += 2;
    j  --;
  }

  if (n & 1) {
    a1 = a + 0 * lda;

    i = m;
    ii = 0;
    while (i > 0) {

      if (ii == jj) {
#ifndef UNIT
	data01 = *(a1 + 0);
#endif
	*(b +  0) = INV(data01);
      }

      if (ii > jj) {
	data01 = *(a1 + 0);
	*(b +  0) = data01;
      }

      a1+= 1;
      b += 1;
      i  --;
      ii += 1;
    }
  }

  return 0;
}