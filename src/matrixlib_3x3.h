//////////////////////////////////////////////////////////
// QCADesigner                                          //
// Copyright 2002 Konrad Walus                          //
// All Rights Reserved                                  //
// Author: Konrad Walus                                 //
// Email: qcadesigner@gmail.com                         //
// WEB: http://qcadesigner.ca/                          //
//////////////////////////////////////////////////////////
//******************************************************//
//*********** PLEASE DO NOT REFORMAT THIS CODE *********//
//******************************************************//
// If your editor wraps long lines disable it or don't  //
// save the core files that way. Any independent files  //
// you generate format as you wish.                     //
//////////////////////////////////////////////////////////
// Please use complete names in variables and fucntions //
// This will reduce ramp up time for new people trying  //
// to contribute to the project.                        //
//////////////////////////////////////////////////////////
// Contents:                                            //
//                                                      //
// Complex 3x3 Matrix Library														//
//                                                      //
//////////////////////////////////////////////////////////

typedef struct
{
	double re;
	double im;
}complex;

inline void complexCheckRealMatrix(complex A[3][3]);
inline complex complexMultiply(complex A, complex B);
inline complex complexDivide(complex denominator, complex numerator);
inline complex complexAdd(complex A, complex B);
inline complex complexSub(complex A, complex B);
inline void complexConstMatrixMultiplication(complex A, complex B[3][3], complex result[3][3]);
inline void complexConstMatrixDivision(complex A, complex B[3][3], complex result[3][3]);
inline void complexMatrixMultiplication(complex A[3][3], complex B[3][3], complex result[3][3]);
inline void complexMatrixAddition(complex A[3][3], complex B[3][3], complex result[3][3]);
inline void complexMatrixSubtraction(complex A[3][3], complex B[3][3], complex result[3][3]);
inline void complexMatrixRealExponential(complex result[3][3]);
inline complex complexTr(complex A[3][3]);
inline void complexIdentityMatrix(complex result[3][3]);
inline void complexExtractRow(int row, complex A[3][3], complex vector[3]);
inline void complexExtractColumn(int column, complex A[3][3], complex vector[3]);

#define IS_REAL_3X3(A) \
	(0 == (A)[0][0].im &&  \
	 0 == (A)[0][1].im &&  \
	 0 == (A)[0][2].im &&  \
	 0 == (A)[1][0].im &&  \
	 0 == (A)[1][1].im &&  \
	 0 == (A)[1][2].im &&  \
	 0 == (A)[2][0].im &&  \
	 0 == (A)[2][1].im &&  \
	 0 == (A)[2][2].im)
	 