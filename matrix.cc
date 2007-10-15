/*
 * Matrix utilities
 *
 * Transcribed from Graphics Gems codes, 1990
 *
 * Author : Ricardo Marroquim
 * Date created : 27-01-07
 *
 **/

#ifndef __MATRIX__
#define __MATRIX__

#define SMALL_NUMBER 0

#include "math.h"
#include <algorithm>
#include <iostream>

using namespace std;

void adjoint (double in[4][4], double out[4][4]);
double det4x4( double m[4][4] );
double det3x3( double a1, double a2, double a3,
	       double b1, double b2, double b3, 
	       double c1, double c2, double c3 );
double det2x2(double a, double b, double c, double d);

void transpose (double in[2][2], double out[][2]) {
  out [0][0] = in [0][0];
  out [0][1] = in [1][0];
  out [1][0] = in [0][1];
  out [1][1] = in [1][1];

}

void inverse (double in[2][2], double out[][2]) {
  double det = 1.0 / det2x2(in[0][0], in[0][1], in[1][0], in[1][1]);
  out [0][0] =    det*in[1][1];
  out [0][1] = -1*det*in[0][1];
  out [1][0] = -1*det*in[1][0];
  out [1][1] =    det*in[0][0];
}

/* 
 *   inverse( original_matrix, inverse_matrix )
 * 
 *    calculate the inverse of a 4x4 matrix
 *
 *     -1     
 *     A  = ___1__ adjoint A
 *         det A
 */

void inverse (double in[4][4], double out[][4]) {
  int i, j;
  double det;

  /* calculate the adjoint matrix */

  adjoint( in, out );

  /*  calculate the 4x4 determinent
   *  if the determinent is zero, 
   *  then the inverse matrix is not unique.
   */

  det = det4x4( out );

  if ( fabs( det ) < SMALL_NUMBER) {
    printf("Non-singular matrix, no inverse!\n");
    exit(0);
  }

  /* scale the adjoint matrix to get the inverse */

  for (i=0; i<4; i++)
    for(j=0; j<4; j++)
      out[i][j] = out[i][j] / det;
}


/* 
 *   adjoint( original_matrix, inverse_matrix )
 * 
 *     calculate the adjoint of a 4x4 matrix
 *
 *      Let  a   denote the minor determinant of matrix A obtained by
 *           ij
 *
 *      deleting the ith row and jth column from A.
 *
 *                    i+j
 *     Let  b   = (-1)    a
 *          ij            ji
 *
 *    The matrix B = (b  ) is the adjoint of A
 *                     ij
 */

void adjoint (double in[4][4], double out[][4]) {
  double a1, a2, a3, a4, b1, b2, b3, b4;
  double c1, c2, c3, c4, d1, d2, d3, d4;

  /* assign to individual variable names to aid  */
  /* selecting correct values  */

  a1 = in[0][0]; b1 = in[0][1]; 
  c1 = in[0][2]; d1 = in[0][3];

  a2 = in[1][0]; b2 = in[1][1]; 
  c2 = in[1][2]; d2 = in[1][3];

  a3 = in[2][0]; b3 = in[2][1];
  c3 = in[2][2]; d3 = in[2][3];

  a4 = in[3][0]; b4 = in[3][1]; 
  c4 = in[3][2]; d4 = in[3][3];

  /* row column labeling reversed since we transpose rows & columns */

  out[0][0]  =   det3x3( b2, b3, b4, c2, c3, c4, d2, d3, d4);
  out[1][0]  = - det3x3( a2, a3, a4, c2, c3, c4, d2, d3, d4);
  out[2][0]  =   det3x3( a2, a3, a4, b2, b3, b4, d2, d3, d4);
  out[3][0]  = - det3x3( a2, a3, a4, b2, b3, b4, c2, c3, c4);
        
  out[0][1]  = - det3x3( b1, b3, b4, c1, c3, c4, d1, d3, d4);
  out[1][1]  =   det3x3( a1, a3, a4, c1, c3, c4, d1, d3, d4);
  out[2][1]  = - det3x3( a1, a3, a4, b1, b3, b4, d1, d3, d4);
  out[3][1]  =   det3x3( a1, a3, a4, b1, b3, b4, c1, c3, c4);
        
  out[0][2]  =   det3x3( b1, b2, b4, c1, c2, c4, d1, d2, d4);
  out[1][2]  = - det3x3( a1, a2, a4, c1, c2, c4, d1, d2, d4);
  out[2][2]  =   det3x3( a1, a2, a4, b1, b2, b4, d1, d2, d4);
  out[3][2]  = - det3x3( a1, a2, a4, b1, b2, b4, c1, c2, c4);
        
  out[0][3]  = - det3x3( b1, b2, b3, c1, c2, c3, d1, d2, d3);
  out[1][3]  =   det3x3( a1, a2, a3, c1, c2, c3, d1, d2, d3);
  out[2][3]  = - det3x3( a1, a2, a3, b1, b2, b3, d1, d2, d3);
  out[3][3]  =   det3x3( a1, a2, a3, b1, b2, b3, c1, c2, c3);
}

/*
 * double = det4x4( matrix )
 * 
 * calculate the determinent of a 4x4 matrix.
 */
double det4x4( double m[4][4] ) {

  double ans;
  double a1, a2, a3, a4, b1, b2, b3, b4, c1, c2, c3, c4, d1, d2, d3, d4;

  /* assign to individual variable names to aid selecting */
  /*  correct elements */

  a1 = m[0][0]; b1 = m[0][1]; 
  c1 = m[0][2]; d1 = m[0][3];

  a2 = m[1][0]; b2 = m[1][1]; 
  c2 = m[1][2]; d2 = m[1][3];

  a3 = m[2][0]; b3 = m[2][1]; 
  c3 = m[2][2]; d3 = m[2][3];

  a4 = m[3][0]; b4 = m[3][1]; 
  c4 = m[3][2]; d4 = m[3][3];
  
  ans = a1 * det3x3( b2, b3, b4, c2, c3, c4, d2, d3, d4)
    - b1 * det3x3( a2, a3, a4, c2, c3, c4, d2, d3, d4)
    + c1 * det3x3( a2, a3, a4, b2, b3, b4, d2, d3, d4)
    - d1 * det3x3( a2, a3, a4, b2, b3, b4, c2, c3, c4);
  return ans;
}

/*
 * double = det3x3(  a1, a2, a3, b1, b2, b3, c1, c2, c3 )
 * 
 * calculate the determinent of a 3x3 matrix
 * in the form
 *
 *     | a1,  b1,  c1 |
 *     | a2,  b2,  c2 |
 *     | a3,  b3,  c3 |
 */

double det3x3( double a1, double a2, double a3,
	       double b1, double b2, double b3, 
	       double c1, double c2, double c3 ) {
  double ans;

  ans = a1 * det2x2( b2, b3, c2, c3 )
    - b1 * det2x2( a2, a3, c2, c3 )
    + c1 * det2x2( a2, a3, b2, b3 );
  return ans;
}

/*
 * double = det2x2( double a, double b, double c, double d )
 * 
 * calculate the determinent of a 2x2 matrix.
 */
double det2x2(double a, double b, double c, double d) {
  double ans;
  ans = a * d - b * c;
  return ans;
}

/*
 * double = det2x2( double a, double b, double c, double d )
 * 
 * calculate the determinent of a 2x2 matrix.
 */
double det ( double m[2][2] ) {
  return m[0][0] * m[1][1] - m[0][1] * m[1][0];
}

double infNorm( double m[4][4] ) {
  double n1 = fabs(m[0][0]) + fabs(m[1][0]) + fabs(m[2][0]);
  double n2 = fabs(m[0][1]) + fabs(m[1][1]) + fabs(m[2][1]);
  double n3 = fabs(m[0][2]) + fabs(m[1][2]) + fabs(m[2][2]);
  return max(n1, max(n2,n3));
}

/// Multiply two 4x4 matrices
/// Params m1 and m2 are switched since the original code was in row-major, and
/// OpenGL uses column-major
/// @param m1 First input matrix
/// @param m2 Second input matrix
/// @return Multiplication result
void multiply ( double m2[4][4], double m1[4][4], double out[][4] ) {

  out[0][0] = (m1[0][0]*m2[0][0]) + (m1[1][0]*m2[0][1]) + (m1[2][0]*m2[0][2]) + (m1[3][0]*m2[0][3]);
  out[0][1] = (m1[0][1]*m2[0][0]) + (m1[1][1]*m2[0][1]) + (m1[2][1]*m2[0][2]) + (m1[3][1]*m2[0][3]);
  out[0][2] = (m1[0][2]*m2[0][0]) + (m1[1][2]*m2[0][1]) + (m1[2][2]*m2[0][2]) + (m1[3][2]*m2[0][3]);
  out[0][3] = (m1[0][3]*m2[0][0]) + (m1[1][3]*m2[0][1]) + (m1[2][3]*m2[0][2]) + (m1[3][3]*m2[0][3]);

  out[1][0] = (m1[0][0]*m2[1][0]) + (m1[1][0]*m2[1][1]) + (m1[2][0]*m2[1][2]) + (m1[3][0]*m2[1][3]);
  out[1][1] = (m1[0][1]*m2[1][0]) + (m1[1][1]*m2[1][1]) + (m1[2][1]*m2[1][2]) + (m1[3][1]*m2[1][3]);
  out[1][2] = (m1[0][2]*m2[1][0]) + (m1[1][2]*m2[1][1]) + (m1[2][2]*m2[1][2]) + (m1[3][2]*m2[1][3]);
  out[1][3] = (m1[0][3]*m2[1][0]) + (m1[1][3]*m2[1][1]) + (m1[2][3]*m2[1][2]) + (m1[3][3]*m2[1][3]);

  out[2][0] = (m1[0][0]*m2[2][0]) + (m1[1][0]*m2[2][1]) + (m1[2][0]*m2[2][2]) + (m1[3][0]*m2[2][3]);
  out[2][1] = (m1[0][1]*m2[2][0]) + (m1[1][1]*m2[2][1]) + (m1[2][1]*m2[2][2]) + (m1[3][1]*m2[2][3]);
  out[2][2] = (m1[0][2]*m2[2][0]) + (m1[1][2]*m2[2][1]) + (m1[2][2]*m2[2][2]) + (m1[3][2]*m2[2][3]);
  out[2][3] = (m1[0][3]*m2[2][0]) + (m1[1][3]*m2[2][1]) + (m1[2][3]*m2[2][2]) + (m1[3][3]*m2[2][3]);

  out[3][0] = (m1[0][0]*m2[3][0]) + (m1[1][0]*m2[3][1]) + (m1[2][0]*m2[3][2]) + (m1[3][0]*m2[3][3]);
  out[3][1] = (m1[0][1]*m2[3][0]) + (m1[1][1]*m2[3][1]) + (m1[2][1]*m2[3][2]) + (m1[3][1]*m2[3][3]);
  out[3][2] = (m1[0][2]*m2[3][0]) + (m1[1][2]*m2[3][1]) + (m1[2][2]*m2[3][2]) + (m1[3][2]*m2[3][3]);
  out[3][3] = (m1[0][3]*m2[3][0]) + (m1[1][3]*m2[3][1]) + (m1[2][3]*m2[3][2]) + (m1[3][3]*m2[3][3]);
}

/// Multiply two 4x4 matrices
/// @param m1 First input matrix
/// @param m2 Second input matrix
/// @return Multiplication result
void multiply ( double m2[16], double m1[16], double out[][4] ) {

  out[0][0] = (m1 [0]*m2 [0]) + (m1 [4]*m2 [1]) + (m1 [8]*m2 [2]) + (m1[12]*m2 [3]);
  out[0][1] = (m1 [1]*m2 [0]) + (m1 [5]*m2 [1]) + (m1 [9]*m2 [2]) + (m1[13]*m2 [3]);
  out[0][2] = (m1 [2]*m2 [0]) + (m1 [6]*m2 [1]) + (m1[10]*m2 [2]) + (m1[14]*m2 [3]);
  out[0][3] = (m1 [3]*m2 [0]) + (m1 [7]*m2 [1]) + (m1[11]*m2 [2]) + (m1[15]*m2 [3]);

  out[1][0] = (m1 [0]*m2 [4]) + (m1 [4]*m2 [5]) + (m1 [8]*m2 [6]) + (m1[12]*m2 [7]);
  out[1][1] = (m1 [1]*m2 [4]) + (m1 [5]*m2 [5]) + (m1 [9]*m2 [6]) + (m1[13]*m2 [7]);
  out[1][2] = (m1 [2]*m2 [4]) + (m1 [6]*m2 [5]) + (m1[10]*m2 [6]) + (m1[14]*m2 [7]);
  out[1][3] = (m1 [3]*m2 [4]) + (m1 [7]*m2 [5]) + (m1[11]*m2 [6]) + (m1[15]*m2 [7]);

  out[2][0] = (m1 [0]*m2 [8]) + (m1 [4]*m2 [9]) + (m1 [8]*m2[10]) + (m1[12]*m2[11]);
  out[2][1] = (m1 [1]*m2 [8]) + (m1 [5]*m2 [9]) + (m1 [9]*m2[10]) + (m1[13]*m2[11]);
  out[2][2] = (m1 [2]*m2 [8]) + (m1 [6]*m2 [9]) + (m1[10]*m2[10]) + (m1[14]*m2[11]);
  out[2][3] = (m1 [3]*m2 [8]) + (m1 [7]*m2 [9]) + (m1[11]*m2[10]) + (m1[15]*m2[11]);

  out[3][0] = (m1 [0]*m2[12]) + (m1 [4]*m2[13]) + (m1 [8]*m2[14]) + (m1[12]*m2[15]);
  out[3][1] = (m1 [1]*m2[12]) + (m1 [5]*m2[13]) + (m1 [9]*m2[14]) + (m1[13]*m2[15]);
  out[3][2] = (m1 [2]*m2[12]) + (m1 [6]*m2[13]) + (m1[10]*m2[14]) + (m1[14]*m2[15]);
  out[3][3] = (m1 [3]*m2[12]) + (m1 [7]*m2[13]) + (m1[11]*m2[14]) + (m1[15]*m2[15]);
}

/// Multiply a vector by a matrix
/// @param v Given vector
/// @param m Given matrix
/// @param out Vector reesulting of vector-matrix multiplication
void multiply ( const double v[], const double m[][4], double out[]) {
  out[0] = v[0]*m[0][0] + v[1]*m[0][1] + v[2]*m[0][2] + v[3]*m[0][3];
  out[1] = v[0]*m[1][0] + v[1]*m[1][1] + v[2]*m[1][2] + v[3]*m[1][3];
  out[2] = v[0]*m[2][0] + v[1]*m[2][1] + v[2]*m[2][2] + v[3]*m[2][3];
  out[3] = v[0]*m[3][0] + v[1]*m[3][1] + v[2]*m[3][2] + v[3]*m[3][3];
}

/// Multiply a vector by a matrix
/// @param v Given vector
/// @param m Given matrix
void multiply ( double v[], const double m[][4]) {
  double out[4];
  out[0] = v[0]*m[0][0] + v[1]*m[0][1] + v[2]*m[0][2] + v[3]*m[0][3];
  out[1] = v[0]*m[1][0] + v[1]*m[1][1] + v[2]*m[1][2] + v[3]*m[1][3];
  out[2] = v[0]*m[2][0] + v[1]*m[2][1] + v[2]*m[2][2] + v[3]*m[2][3];
  out[3] = v[0]*m[3][0] + v[1]*m[3][1] + v[2]*m[3][2] + v[3]*m[3][3];

  out[0] = v[0]*m[0][0] + v[1]*m[1][0] + v[2]*m[2][0] + v[3]*m[3][0];
  out[1] = v[0]*m[0][1] + v[1]*m[1][1] + v[2]*m[2][1] + v[3]*m[3][1];
  out[2] = v[0]*m[0][2] + v[1]*m[1][2] + v[2]*m[2][2] + v[3]*m[3][2];
  out[3] = v[0]*m[0][3] + v[1]*m[1][3] + v[2]*m[2][3] + v[3]*m[3][3];
  for (int i =0; i < 4; ++i)
    v[i] = out[i];
}

void transpose( double in[4][4], double out[][4])  {
   
  for (int i = 0; i < 4; ++i)
    for (int j = 0; j < 4; ++j)
      out[i][j] = in[j][i];
}

void add ( double m1[2][2], double m2[2][2], double out[][2]) {
  for (int i = 0; i < 2; ++i)
    for (int j = 0; j < 2; ++j)
      out[i][j] = m1[i][j] + m2[i][j];
}


/// From Bart Adams SurfelViewer, matrix4x4 class
void inverse2(double in[4][4], double out[][4]  ) {


  // OpenGL matrices are column major and can be quite confusing to access 
  // when stored in the typical, one-dimensional array often used by the API.
  // Here are some shorthand conversion macros, which convert a row/column 
  // combination into an array index.
	

#define m11 in[0][0]
#define m12 in[0][1]
#define m13 in[0][2]
#define m14 in[0][3]
#define m21 in[1][0]
#define m22 in[1][1]
#define m23 in[1][2]
#define m24 in[1][3]
#define m31 in[2][0]
#define m32 in[2][1]
#define m33 in[2][2]
#define m34 in[2][3]
#define m41 in[3][0]
#define m42 in[3][1]
#define m43 in[3][2]
#define m44 in[3][3]


  // Inverse = adjoint / det. (See linear algebra texts.)

  // pre-compute 2x2 dets for last two rows when computing
  // cofactors of first two rows.
  float d12 = (m31 * m42 - m41 * m32);
  float d13 = (m31 * m43 - m41 * m33);
  float d23 = (m32 * m43 - m42 * m33);
  float d24 = (m32 * m44 - m42 * m34);
  float d34 = (m33 * m44 - m43 * m34);
  float d41 = (m34 * m41 - m44 * m31);

  float tmp[4][4];
	
  tmp[0][0] =  (m22 * d34 - m23 * d24 + m24 * d23);
  tmp[0][1] = -(m21 * d34 + m23 * d41 + m24 * d13);
  tmp[0][2] =  (m21 * d24 + m22 * d41 + m24 * d12);
  tmp[0][3] = -(m21 * d23 - m22 * d13 + m23 * d12);

  // Compute determinant as early as possible using these cofactors.
  float det = m11 * tmp[0][0] + m12 * tmp[0][1] + m13 * tmp[0][2] + m14 * tmp[0][3];

  // Run singularity test.
  if( det == 0.0 )
    {

      float identity[4][4] = 
	{
	  {1.0, 0.0, 0.0, 0.0},
	  {0.0, 1.0, 0.0, 0.0},
	  {0.0, 0.0, 1.0, 0.0},
	  {0.0, 0.0, 0.0, 1.0}
	};

      for (int i = 0; i < 4; ++i)
	for (int j = 0; j < 4; ++j)
	  out[i][j] = identity[i][j];

      //memcpy( result.m, identity, 16*sizeof(float) );
    }
  else
    {
      float invDet = 1.0f / det;
	   
      // Compute rest of inverse.
      tmp[0][0] *= invDet;
      tmp[0][1] *= invDet;
      tmp[0][2] *= invDet;
      tmp[0][3] *= invDet;

      tmp[1][0] = -(m12 * d34 - m13 * d24 + m14 * d23) * invDet;
      tmp[1][1] =  (m11 * d34 + m13 * d41 + m14 * d13) * invDet;
      tmp[1][2] = -(m11 * d24 + m12 * d41 + m14 * d12) * invDet;
      tmp[1][3] =  (m11 * d23 - m12 * d13 + m13 * d12) * invDet;

      // Pre-compute 2x2 dets for first two rows when computing cofactors 
      // of last two rows.
      d12 = m11 * m22 - m21 * m12;
      d13 = m11 * m23 - m21 * m13;
      d23 = m12 * m23 - m22 * m13;
      d24 = m12 * m24 - m22 * m14;
      d34 = m13 * m24 - m23 * m14;
      d41 = m14 * m21 - m24 * m11;

      tmp[2][0]  =  (m42 * d34 - m43 * d24 + m44 * d23) * invDet;
      tmp[2][1]  = -(m41 * d34 + m43 * d41 + m44 * d13) * invDet;
      tmp[2][2] =  (m41 * d24 + m42 * d41 + m44 * d12) * invDet;
      tmp[2][3] = -(m41 * d23 - m42 * d13 + m43 * d12) * invDet;
      tmp[3][0] = -(m32 * d34 - m33 * d24 + m34 * d23) * invDet;
      tmp[3][1] =  (m31 * d34 + m33 * d41 + m34 * d13) * invDet;
      tmp[3][2] = -(m31 * d24 + m32 * d41 + m34 * d12) * invDet;
      tmp[3][3] =  (m31 * d23 - m32 * d13 + m33 * d12) * invDet;

      for (int i = 0; i < 4; ++i)
	for (int j = 0; j < 4; ++j)
	  out[i][j] = tmp[i][j];

      //memcpy( result.m, tmp, 16*sizeof(float) );
    }

#undef m11
#undef m12
#undef m13
#undef m14
#undef m21
#undef m22
#undef m23
#undef m24
#undef m31
#undef m32
#undef m33
#undef m34
#undef m41
#undef m42
#undef m43
#undef m44
#undef MAT

}

#endif
