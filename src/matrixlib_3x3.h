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
// Complex 3x3 Matrix Library							//
//                                                      //
//////////////////////////////////////////////////////////

typedef struct
{
	double re;
	double im;
}complex;

complex complexMultiply(complex A, complex B);
complex complexAdd(complex A, complex B);
complex complexSub(complex A, complex B);
void complexConstMatrixMultiplication(complex A, complex B[3][3], complex result[3][3]);
void complexMatrixMultiplication(complex A[3][3], complex B[3][3], complex result[3][3]);
void complexMatrixAddition(complex A[3][3], complex B[3][3], complex result[3][3]);
void complexMatrixSubtraction(complex A[3][3], complex B[3][3], complex result[3][3]);
complex complexTr(complex A[3][3]);
void complexIdentityMatrix(complex result[3][3]);
void complexExtractRow(int row, complex A[3][3], complex vector[3]);
void complexExtractColumn(int column, complex A[3][3], complex vector[3]);