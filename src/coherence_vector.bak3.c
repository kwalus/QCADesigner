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
// The coherence vector time-dependent simulation       //
// engine.                                              //
//                                                      //
//////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <cvode/cvode.h>
#include <nvector/nvector_serial.h>
#include <cvode/cvode_dense.h>
#include <sundials/sundials_dense.h>
#include <sundials/sundials_types.h>
#include <sundials/sundials_math.h>

#include "objects/QCADCell.h"
#include "simulation.h"
#include "coherence_vector.h"
#include "custom_widgets.h"
#include "global_consts.h"
#ifdef GTK_GUI
#include "callback_helpers.h"
#endif /* def GTK_GUI */
#include "intl.h"
#include "cvode_impl.h"
#include "cvode_direct_impl.h"

// Calculates the magnitude of the 3D energy vector
#define magnitude_energy_vector(P,G) (hypot(2*(G), (P)) * over_hbar)
//(sqrt((4.0*(G)*(G) + (P)*(P))*over_hbar_sqr))
// Calculates the temperature ratio
#define temp_ratio(P,G,T) (hypot((G),(P)*0.5)/((T) * kB))
#define Ith(v,i)	NV_Ith_S(v, i-1)
#define IJth(A,i,j)	DENSE_ELEM(A,i-1,j-1)
#define dldt(i)		Ith(ydot,i)
#define l(i)		Ith(y,i)

/* Readability Replacements */

#define vec_tmpl     (cv_mem->cv_tempv)
#define mtype     (cvdls_mem->d_type)
#define jacDQ     (cvdls_mem->d_jacDQ)
#define jac       (cvdls_mem->d_djac)
#define d_n         (cvdls_mem->d_n)
#define d_M         (cvdls_mem->d_M)
#define pivots    (cvdls_mem->d_pivots)
#define savedJ    (cvdls_mem->d_savedJ)
#define J_data    (cvdls_mem->d_J_data)
#define last_flag (cvdls_mem->d_last_flag)
#define nje       (cvdls_mem->d_nje)
#define nstlj     (cvdls_mem->d_nstlj)
#define nfeDQ     (cvdls_mem->d_nfeDQ)

/*=================================================================*/
/*             Macros                                              */
/*=================================================================*/

/* Macro: loop */
#define loop for(;;)

/*=================================================================*/
/*             CVODE Private Constants                             */
/*=================================================================*/

#define MIN_INC_MULT RCONST(1000.0)
#define ZERO   RCONST(0.0)     /* real 0.0     */
#define TINY   RCONST(1.0e-10) /* small number */
#define TENTH  RCONST(0.1)     /* real 0.1     */
#define POINT2 RCONST(0.2)     /* real 0.2     */
#define FOURTH RCONST(0.25)    /* real 0.25    */
#define HALF   RCONST(0.5)     /* real 0.5     */
#define ONE    RCONST(1.0)     /* real 1.0     */
#define ONEPT5 RCONST(1.5)     /* real 1.5     */
#define TWO    RCONST(2.0)     /* real 2.0     */
#define THREE  RCONST(3.0)     /* real 3.0     */
#define FOUR   RCONST(4.0)     /* real 4.0     */
#define FIVE   RCONST(5.0)     /* real 5.0     */
#define TWELVE RCONST(12.0)    /* real 12.0    */
#define HUN    RCONST(100.0)   /* real 100.0   */

/*=================================================================*/
/*             CVODE Routine-Specific Constants                   */
/*=================================================================*/

/* 
 * Control constants for lower-level functions used by CVStep 
 * ----------------------------------------------------------
 *
 * CVHin return values:
 *    CV_SUCCESS
 *    CV_RHSFUNC_FAIL
 *    CV_TOO_CLOSE
 *
 * CVStep control constants:
 *    DO_ERROR_TEST
 *    PREDICT_AGAIN
 *
 * CVStep return values: 
 *    CV_SUCCESS,
 *    CV_LSETUP_FAIL,  CV_LSOLVE_FAIL, 
 *    CV_RHSFUNC_FAIL, CV_RTFUNC_FAIL
 *    CV_CONV_FAILURE, CV_ERR_FAILURE,
 *    CV_FIRST_RHSFUNC_ERR
 *
 * CVNls input nflag values:
 *    FIRST_CALL
 *    PREV_CONV_FAIL
 *    PREV_ERR_FAIL
 *    
 * CVNls return values: 
 *    CV_SUCCESS,
 *    CV_LSETUP_FAIL, CV_LSOLVE_FAIL, CV_RHSFUNC_FAIL,
 *    CONV_FAIL, RHSFUNC_RECVR
 * 
 * CVNewtonIteration return values:
 *    CV_SUCCESS, 
 *    CV_LSOLVE_FAIL, CV_RHSFUNC_FAIL
 *    CONV_FAIL, RHSFUNC_RECVR,
 *    TRY_AGAIN
 * 
 */

#define DO_ERROR_TEST    +2
#define PREDICT_AGAIN    +3

#define CONV_FAIL        +4 
#define TRY_AGAIN        +5

#define FIRST_CALL       +6
#define PREV_CONV_FAIL   +7
#define PREV_ERR_FAIL    +8

#define RHSFUNC_RECVR    +9

/*
 * Control constants for lower-level rootfinding functions
 * -------------------------------------------------------
 *
 * CVRcheck1 return values:
 *    CV_SUCCESS,
 *    CV_RTFUNC_FAIL,
 * CVRcheck2 return values:
 *    CV_SUCCESS
 *    CV_RTFUNC_FAIL,
 *    CLOSERT
 *    RTFOUND
 * CVRcheck3 return values:
 *    CV_SUCCESS
 *    CV_RTFUNC_FAIL,
 *    RTFOUND
 * CVRootfind return values:
 *    CV_SUCCESS
 *    CV_RTFUNC_FAIL,
 *    RTFOUND
 */

#define RTFOUND          +1
#define CLOSERT          +3

/*
 * Control constants for tolerances
 * --------------------------------
 */

#define CV_NN  0
#define CV_SS  1
#define CV_SV  2
#define CV_WF  3

/*
 * Algorithmic constants
 * ---------------------
 *
 * CVodeGetDky and CVStep
 *
 *    FUZZ_FACTOR
 *
 * CVHin
 *
 *    HLB_FACTOR
 *    HUB_FACTOR
 *    H_BIAS
 *    MAX_ITERS
 *
 * CVodeCreate 
 *
 *   CORTES
 *
 * CVStep
 *
 *    THRESH
 *    ETAMX1
 *    ETAMX2
 *    ETAMX3
 *    ETAMXF
 *    ETAMIN
 *    ETACF
 *    ADDON
 *    BIAS1
 *    BIAS2
 *    BIAS3
 *    ONEPSM
 *
 *    SMALL_NST   nst > SMALL_NST => use ETAMX3 
 *    MXNCF       max no. of convergence failures during one step try
 *    MXNEF       max no. of error test failures during one step try
 *    MXNEF1      max no. of error test failures before forcing a reduction of order
 *    SMALL_NEF   if an error failure occurs and SMALL_NEF <= nef <= MXNEF1, then
 *                reset eta =  MIN(eta, ETAMXF)
 *    LONG_WAIT   number of steps to wait before considering an order change when
 *                q==1 and MXNEF1 error test failures have occurred
 *
 * CVNls
 *    
 *    NLS_MAXCOR  maximum no. of corrector iterations for the nonlinear solver
 *    CRDOWN      constant used in the estimation of the convergence rate (crate)
 *                of the iterates for the nonlinear equation
 *    DGMAX       iter == CV_NEWTON, |gamma/gammap-1| > DGMAX => call lsetup
 *    RDIV        declare divergence if ratio del/delp > RDIV
 *    MSBP        max no. of steps between lsetup calls
 *    
 */


#define FUZZ_FACTOR RCONST(100.0)

#define HLB_FACTOR RCONST(100.0)
#define HUB_FACTOR RCONST(0.1)
#define H_BIAS     HALF
#define MAX_ITERS  4

#define CORTES RCONST(0.1)

#define THRESH RCONST(1.5)
#define ETAMX1 RCONST(10000.0) 
#define ETAMX2 RCONST(10.0)
#define ETAMX3 RCONST(10.0)
#define ETAMXF RCONST(0.2)
#define ETAMIN RCONST(0.1)
#define ETACF  RCONST(0.25)
#define ADDON  RCONST(0.000001)
#define BIAS1  RCONST(6.0)
#define BIAS2  RCONST(6.0)
#define BIAS3  RCONST(10.0)
#define ONEPSM RCONST(1.000001)

#define SMALL_NST    10
#define MXNCF        10
#define MXNEF         7
#define MXNEF1        3
#define SMALL_NEF     2
#define LONG_WAIT    10

#define NLS_MAXCOR 3
#define CRDOWN RCONST(0.3)
#define DGMAX  RCONST(0.3)

#define RDIV      TWO
#define MSBP       20

//!Options for the coherence simulation engine
//Added by Marco default values for phase shift (0,0,0,0)
//Added by Faizal: wave_numbers (defaults kx=0, ky=0)
coherence_OP coherence_options = {1, 0.1, 5e-4, 500, 9.43e-19, 1.41e-20, 0.0, 2.0, 200, 1, 1.15, EULER_METHOD, FALSE, FALSE,0,0,0,0,0,0,CONT_CLOCKING} ;

typedef struct
  {
  int number_of_neighbours;
  QCADCell **neighbours;
  int *neighbour_layer;
  double *Ek;
  double lambda_x;
  double lambda_y;
  double lambda_z;
  } coherence_model;


/*Param struct*/
typedef struct
	{
	float** Ek;
	float** Ekd; 
	int n;
	int nd;
	float* pd;
	int** K;
	int *clock_value;	
	int xx;
	int xy;
	int xz;
	int yy;
	int yz;
	int zz;
	float* lss;
	coherence_OP *options;	
 } Param;

#ifdef GTK_GUI
extern int STOP_SIMULATION;
#else
static int STOP_SIMULATION = 0 ;
#endif /* def GTK_GUI */

// some often used variables that can be precalculated
typedef struct
  {
  double clock_prefactor;
  double clock_shift;
  double four_pi_over_number_samples;
  double two_pi_over_number_samples;
  double hbar_over_kBT;
	  double over_kBT;
  } coherence_optimizations;

// instance of the optimization options;
static coherence_optimizations optimization_options;

static double coherence_determine_Ek (QCADCell *cell1, QCADCell *cell2, int layer_separation, coherence_OP *options);
static void coherence_refresh_all_Ek (int number_of_cell_layers, int *number_of_cells_in_layer, QCADCell ***sorted_cells, coherence_OP *options);

static void run_coherence_iteration (int sample_number, int number_of_cell_layers, int *number_of_cells_in_layer, QCADCell ***sorted_cells, int total_number_of_inputs, unsigned long int number_samples, const coherence_OP *options, simulation_data *sim_data, int SIMULATION_TYPE, VectorTable *pvt, double *energy);

static inline double calculate_clock_value (unsigned int clock_num, unsigned long int sample, unsigned long int number_samples, int total_number_of_inputs, const coherence_OP *options, int SIMULATION_TYPE, VectorTable *pvt);
static inline double calculate_clock_value_cc (QCADCell *cell, unsigned long int sample, unsigned long int number_samples, int total_number_of_inputs, const coherence_OP *options, int SIMULATION_TYPE, VectorTable *pvt);
static inline double lambda_ss_x (double t, double PEk, double Gamma, const coherence_OP *options);
static inline double lambda_ss_y (double t, double PEk, double Gamma, const coherence_OP *options);
static inline double lambda_ss_z (double t, double PEk, double Gamma, const coherence_OP *options);
static inline double lambda_x_next (double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_OP *options);
static inline double lambda_y_next (double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_OP *options);
static inline double lambda_z_next (double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_OP *options);
static inline double slope_x (double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_OP *options);
static inline double slope_y (double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_OP *options);
static inline double slope_z (double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_OP *options);
static int compareCoherenceQCells (const void *p1, const void *p2) ;
static inline void logic_sim (int number_of_cell_layers, int *number_of_cells_in_layer, QCADCell ***sorted_cells, double **Ek_matrix, int total_number_of_inputs, const coherence_OP *options, int SIMULATION_TYPE, VectorTable *pvt, double *Pol, double *lss);
static inline void calc_pol (double *Pol, double **Ek_matrix, double *Pdriver, int *driver_ind, int num_drivers, int *fork, double Ek_max, int num_cells);
static inline void calc_poli (double *Pol, double **Ek_matrix, double EK_min, double Ek_max, int num_cells, int *fork);
static inline void prob_pols (double *Pol, double *lss, double **Ek_matrix, double Ek_max, double Ek_min, int num_elements, int number_of_cell_layers, int *number_of_cells_in_layer, QCADCell ***sorted_cells, const coherence_OP *options) ;
static inline int search_matrix_row(double **A, double cmp, int row, int num_elements);
static inline int find_matrix_row(double **A, double cmp, int row, int num_elements, int select);
static inline int search_array(double *A, double cmp, int num_elements );
static inline int search_array_int(int *A, double cmp, int num_elements );
static inline int search_array_thresh(double *A, double cmp, int num_elements );
static inline int find_array(double *A, double cmp, int num_elements);
static inline int find_in_array(double *A, double cmp, int num_elements, int select);
static inline double get_max(double **A, int num_elements);
static inline double get_min(double **A, int num_elements);
double *compare_array(double *A, double *B, int length);
static inline void array_copy(double *Arr1, double *Arr0, int length);
static inline void add_array(double *Arr1, double *Arr2, int length, double *Arr0);
static inline void mult_array_by_constant(double *Arr1, double constant, int length);

inline float driversum(Param* par, int i);
inline float Kyzz(Param* par, int i, int j, int k, N_Vector y);
inline float Kxxz(Param* par, int i, int j, int k, N_Vector y);
inline float Kxyz(Param* par, int i, int j, int k, N_Vector y);
inline float Kxzz(Param* par, int i, int j, int k, N_Vector y);
inline float Kyxz(Param* par, int i, int j, int k, N_Vector y);
inline float Kyyz(Param* par, int i, int j, int k, N_Vector y);

 
static int twopcorr(realtype t, N_Vector y, N_Vector ydot, void *user_data);

static int check_flag(void *flagvalue, char *funcname, int opt);
N_Vector N_VNew_Serial(long int length);
N_Vector N_VNewEmpty_Serial(long int length);
void N_VDestroy_Serial(N_Vector v);

/* Private function prototypes */
/* z=x */
static void VCopy_Serial(N_Vector x, N_Vector z);
/* z=x+y */
static void VSum_Serial(N_Vector x, N_Vector y, N_Vector z);
/* z=x-y */
static void VDiff_Serial(N_Vector x, N_Vector y, N_Vector z);
/* z=-x */
static void VNeg_Serial(N_Vector x, N_Vector z);
/* z=c(x+y) */
static void VScaleSum_Serial(realtype c, N_Vector x, N_Vector y, N_Vector z);
/* z=c(x-y) */
static void VScaleDiff_Serial(realtype c, N_Vector x, N_Vector y, N_Vector z); 
/* z=ax+y */
static void VLin1_Serial(realtype a, N_Vector x, N_Vector y, N_Vector z);
/* z=ax-y */
static void VLin2_Serial(realtype a, N_Vector x, N_Vector y, N_Vector z);
/* y <- ax+y */
static void Vaxpy_Serial(realtype a, N_Vector x, N_Vector y);
/* x <- ax */
static void VScaleBy_Serial(realtype a, N_Vector x);

N_Vector N_VNew_Serial(long int vec_length);
N_Vector N_VNewEmpty_Serial(long int vec_length);
N_Vector N_VMake_Serial(long int vec_length, realtype *v_data);
N_Vector *N_VCloneVectorArray_Serial(int count, N_Vector w);
N_Vector *N_VCloneVectorArrayEmpty_Serial(int count, N_Vector w);
void N_VDestroyVectorArray_Serial(N_Vector *vs, int count);
void N_VPrint_Serial(N_Vector v);
N_Vector N_VCloneEmpty_Serial(N_Vector w);
N_Vector N_VClone_Serial(N_Vector w);
void N_VDestroy_Serial(N_Vector v);
void N_VSpace_Serial(N_Vector v, long int *lrw, long int *liw);
realtype *N_VGetArrayPointer_Serial(N_Vector v);
void N_VSetArrayPointer_Serial(realtype *v_data, N_Vector v);
void N_VLinearSum_Serial(realtype a, N_Vector x, realtype b, N_Vector y, N_Vector z);
void N_VConst_Serial(realtype c, N_Vector z);
void N_VProd_Serial(N_Vector x, N_Vector y, N_Vector z);
void N_VDiv_Serial(N_Vector x, N_Vector y, N_Vector z);
void N_VScale_Serial(realtype c, N_Vector x, N_Vector z);
void N_VAbs_Serial(N_Vector x, N_Vector z);
void N_VInv_Serial(N_Vector x, N_Vector z);
void N_VAddConst_Serial(N_Vector x, realtype b, N_Vector z);
realtype N_VDotProd_Serial(N_Vector x, N_Vector y);
realtype N_VMaxNorm_Serial(N_Vector x);
realtype N_VWrmsNorm_Serial(N_Vector x, N_Vector w);
realtype N_VWrmsNormMask_Serial(N_Vector x, N_Vector w, N_Vector id);
realtype N_VMin_Serial(N_Vector x);
realtype N_VWL2Norm_Serial(N_Vector x, N_Vector w);
realtype N_VL1Norm_Serial(N_Vector x);
void N_VCompare_Serial(realtype c, N_Vector x, N_Vector z);
booleantype N_VInvTest_Serial(N_Vector x, N_Vector z);



realtype RSqrt(realtype x);
realtype RPowerI(realtype base, int exponent);

/*=================================================================*/
/*             Private Helper Functions Prototypes                 */
/*=================================================================*/

static booleantype CVCheckNvector(N_Vector tmpl);

static int CVInitialSetup(CVodeMem cv_mem);

static booleantype CVAllocVectors(CVodeMem cv_mem, N_Vector tmpl);
static void CVFreeVectors(CVodeMem cv_mem);

static int CVEwtSetSS(CVodeMem cv_mem, N_Vector ycur, N_Vector weight);
static int CVEwtSetSV(CVodeMem cv_mem, N_Vector ycur, N_Vector weight);

static int CVHin(CVodeMem cv_mem, realtype tout);
static realtype CVUpperBoundH0(CVodeMem cv_mem, realtype tdist);
static int CVYddNorm(CVodeMem cv_mem, realtype hg, realtype *yddnrm);

static int CVStep(CVodeMem cv_mem);

static int CVsldet(CVodeMem cv_mem);

static void CVAdjustParams(CVodeMem cv_mem);
static void CVAdjustOrder(CVodeMem cv_mem, int deltaq);
static void CVAdjustAdams(CVodeMem cv_mem, int deltaq);
static void CVAdjustBDF(CVodeMem cv_mem, int deltaq);
static void CVIncreaseBDF(CVodeMem cv_mem);
static void CVDecreaseBDF(CVodeMem cv_mem);

static void CVRescale(CVodeMem cv_mem);

static void CVPredict(CVodeMem cv_mem);

static void CVSet(CVodeMem cv_mem);
static void CVSetAdams(CVodeMem cv_mem);
static realtype CVAdamsStart(CVodeMem cv_mem, realtype m[]);
static void CVAdamsFinish(CVodeMem cv_mem, realtype m[], realtype M[], realtype hsum);
static realtype CVAltSum(int iend, realtype a[], int k);
static void CVSetBDF(CVodeMem cv_mem);
static void CVSetTqBDF(CVodeMem cv_mem, realtype hsum, realtype alpha0,
                       realtype alpha0_hat, realtype xi_inv, realtype xistar_inv);

static int CVNls(CVodeMem cv_mem, int nflag);
static int CVNlsFunctional(CVodeMem cv_mem);
static int CVNlsNewton(CVodeMem cv_mem, int nflag);
static int CVNewtonIteration(CVodeMem cv_mem);

static int CVHandleNFlag(CVodeMem cv_mem, int *nflagPtr, realtype saved_t,
                         int *ncfPtr);

static void CVRestore(CVodeMem cv_mem, realtype saved_t);

static int CVDoErrorTest(CVodeMem cv_mem, int *nflagPtr,
                         realtype saved_t, int *nefPtr, realtype *dsmPtr);

static void CVCompleteStep(CVodeMem cv_mem);

static void CVPrepareNextStep(CVodeMem cv_mem, realtype dsm);
static void CVSetEta(CVodeMem cv_mem);
static realtype CVComputeEtaqm1(CVodeMem cv_mem);
static realtype CVComputeEtaqp1(CVodeMem cv_mem);
static void CVChooseEta(CVodeMem cv_mem);
static void CVBDFStab(CVodeMem cv_mem);

static int  CVHandleFailure(CVodeMem cv_mem,int flag);

static int CVRcheck1(CVodeMem cv_mem);
static int CVRcheck2(CVodeMem cv_mem);
static int CVRcheck3(CVodeMem cv_mem);
static int CVRootfind(CVodeMem cv_mem);

N_Vector N_VClone(N_Vector w);
N_Vector N_VCloneEmpty(N_Vector w);
void N_VDestroy(N_Vector v);
void N_VSpace(N_Vector v, long int *lrw, long int *liw);
realtype *N_VGetArrayPointer(N_Vector v);
void N_VSetArrayPointer(realtype *v_data, N_Vector v);
void N_VLinearSum(realtype a, N_Vector x, realtype b, N_Vector y, N_Vector z);
void N_VConst(realtype c, N_Vector z);
void N_VProd(N_Vector x, N_Vector y, N_Vector z);
void N_VDiv(N_Vector x, N_Vector y, N_Vector z);
void N_VScale(realtype c, N_Vector x, N_Vector z);
void N_VAbs(N_Vector x, N_Vector z);
void N_VInv(N_Vector x, N_Vector z);
void N_VAddConst(N_Vector x, realtype b, N_Vector z);
realtype N_VDotProd(N_Vector x, N_Vector y);
realtype N_VMaxNorm(N_Vector x);
realtype N_VWrmsNorm(N_Vector x, N_Vector w);
realtype N_VWrmsNormMask(N_Vector x, N_Vector w, N_Vector id);
realtype N_VMin(N_Vector x);
realtype N_VWL2Norm(N_Vector x, N_Vector w);
realtype N_VL1Norm(N_Vector x);
void N_VCompare(realtype c, N_Vector x, N_Vector z);
booleantype N_VInvTest(N_Vector x, N_Vector z);
booleantype N_VConstrMask(N_Vector c, N_Vector x, N_Vector m);
realtype N_VMinQuotient(N_Vector num, N_Vector denom);

N_Vector *N_VCloneEmptyVectorArray(int count, N_Vector w);
N_Vector *N_VCloneVectorArray(int count, N_Vector w);
void N_VDestroyVectorArray(N_Vector *vs, int count);

int CVDense(void *cvode_mem, int N);
int CVodeSetMaxOrd(void *cvode_mem, int maxord);
int CVodeSetUserData(void *cvode_mem, void *user_data);
int CVodeSetMaxNumSteps(void *cvode_mem, long int mxsteps);

/* CVDENSE linit, lsetup, lsolve, and lfree routines */

static int cvDenseInit(CVodeMem cv_mem);

static int cvDenseSetup(CVodeMem cv_mem, int convfail, N_Vector ypred,
                        N_Vector fpred, booleantype *jcurPtr, 
                        N_Vector vtemp1, N_Vector vtemp2, N_Vector vtemp3);

static int cvDenseSolve(CVodeMem cv_mem, N_Vector b, N_Vector weight,
                        N_Vector ycur, N_Vector fcur);

static void cvDenseFree(CVodeMem cv_mem);

int cvDlsDenseDQJac(int N, realtype t, N_Vector y, N_Vector fy, DlsMat Jac, void *data, N_Vector tmp1, N_Vector tmp2, N_Vector tmp3);

//-------------------------------------------------------------------//
// -- this is the main simulation procedure -- //
//-------------------------------------------------------------------//
simulation_data *run_coherence_simulation (int SIMULATION_TYPE, DESIGN *design, coherence_OP *options, VectorTable *pvt)
  {
  int i, j, k, l, q, number_of_cell_layers, *number_of_cells_in_layer;
  QCADCell ***sorted_cells = NULL ;
  int total_number_of_inputs = design->bus_layout->inputs->icUsed;
  unsigned long int number_samples;
  //number of points to record in simulation results //
  //simulations can have millions of points and there is no need to plot them all //
  unsigned long int number_recorded_samples = 3000;
  unsigned long int record_interval;
  double PEk = 0;
  gboolean stable;
  double old_lambda_x;
  double old_lambda_y;
  double old_lambda_z;
  double *energy;
  double average_power=0;
  time_t start_time, end_time;
  simulation_data *sim_data = NULL ;
  // for randomization
  int Nix, Nix1, idxCell1, idxCell2 ;
  QCADCell *swap = NULL ;
  BUS_LAYOUT_ITER bli ;
  double dPolarization = 2.0 ;
  int idxMasterBitOrder = -1.0 ;
			
  double **Ek_matrix = NULL;	  
	  double *Pol = NULL;
	  double *lss = NULL;
	  
	  //for Jacob's struct
	  Param user_data;
	  float **Ek_no_inputs;
	  float **Ekd;
	  int **K;
	  float *pd;
	  int *clock_value;
	  int num_normal;
	  //end for Jacob's struct
	  
	  //for CVODE
	  realtype reltol, t, tout;
	  N_Vector y, abstol;
	  void *cvode_mem;
	  int flag, flagr, iout;
	  
	  y = abstol = NULL;
	  cvode_mem = NULL;
	  //end for CVODE
	  
	  
	  
  STOP_SIMULATION = FALSE;

  // -- get the starting time for the simulation -- //
  if ((start_time = time (NULL)) < 0)
    fprintf (stderr, "Could not get start time\n");

  // determine the number of samples from the user options //
  number_samples = (unsigned long int)(ceil (options->duration/options->time_step));

  energy = malloc(number_samples*sizeof(double));

  // if the number of samples is larger then the number of recorded samples then change the
  // time step to ensure only number_recorded_samples is used //
  if (number_recorded_samples >= number_samples)
    {
    number_recorded_samples = number_samples;
    record_interval = 1;
    }
  else
    record_interval = (unsigned long int)ceil ((double)(number_samples - 1) / (double)(number_recorded_samples));

  //fill in some of the optimizations
  optimization_options.clock_prefactor = (options->clock_high - options->clock_low) * options->clock_amplitude_factor;
  optimization_options.clock_shift = (options->clock_high + options->clock_low) * 0.5;
  optimization_options.four_pi_over_number_samples = FOUR_PI / (double)number_samples;
  optimization_options.two_pi_over_number_samples = TWO_PI / (double)number_samples;
  optimization_options.hbar_over_kBT = hbar / (kB * options->T);
  optimization_options.over_kBT = 1 / (kB * options->T);

  // -- spit out some messages for possible debugging -- //
  command_history_message ("About to start the coherence vector simulation with %d samples\n", number_samples);
  command_history_message ("%d samples will be recorded for graphing.\n", number_recorded_samples);
  set_progress_bar_visible (TRUE) ;
  set_progress_bar_label ("Coherence vector simulation:") ;
  set_progress_bar_fraction (0.0) ;

  // Fill in the cell arrays necessary for conducting the simulation
  simulation_inproc_data_new (design, &number_of_cell_layers, &number_of_cells_in_layer, &sorted_cells) ;

	  
  // determine which cells are inputs and which are outputs //
  for(i = 0; i < number_of_cell_layers; i++)
    for(j = 0; j < number_of_cells_in_layer[i]; j++)
      {
      // attach the model parameters to each of the simulation cells //
      sorted_cells[i][j]->cell_model = g_malloc0 (sizeof(coherence_model));

      // -- Clear the model pointers so they are not dangling -- //
      ((coherence_model *)sorted_cells[i][j]->cell_model)->neighbours = NULL;
      ((coherence_model *)sorted_cells[i][j]->cell_model)->Ek = NULL;
      }

  // if we are performing a vector table simulation we consider only the activated inputs //
  if (VECTOR_TABLE == SIMULATION_TYPE)
    for (total_number_of_inputs = 0, Nix = 0 ; Nix < pvt->inputs->icUsed ; Nix++)
      {
      if (exp_array_index_1d (pvt->inputs, VT_INPUT, Nix).active_flag)
        total_number_of_inputs++ ;
      else
        // Kill the input flag for inactive inputs, so they may be correctly simulated
        exp_array_index_1d (pvt->inputs, VT_INPUT, Nix).input->cell_function = QCAD_CELL_NORMAL ;
      }

  // write message to the command history window //
  command_history_message ("Simulation found %d inputs %d outputs\n", total_number_of_inputs, design->bus_layout->outputs->icUsed) ;

  // -- Allocate memory to hold the simulation data -- //
  sim_data = g_malloc0 (sizeof(simulation_data)) ;

  // -- Initialize the simualtion data structure -- //
  sim_data->number_of_traces = design->bus_layout->inputs->icUsed + design->bus_layout->outputs->icUsed;

  // set the number of simulation samples to be the desired number of recorded samples //
  sim_data->number_samples = number_recorded_samples;

  // allocate the memory for each trace //
  sim_data->trace = g_malloc0 (sizeof (struct TRACEDATA) * sim_data->number_of_traces);

  // create and initialize the inputs into the sim data structure //
  for (i = 0; i < design->bus_layout->inputs->icUsed; i++)
    {
    sim_data->trace[i].data_labels = g_strdup (qcad_cell_get_label (exp_array_index_1d (design->bus_layout->inputs, BUS_LAYOUT_CELL, i).cell));
    sim_data->trace[i].drawtrace = TRUE;
    sim_data->trace[i].trace_function = QCAD_CELL_INPUT;
    sim_data->trace[i].data = g_malloc0 (sim_data->number_samples * sizeof (double));
    }

  // create and initialize the outputs into the sim data structure //
  for (i = 0; i < design->bus_layout->outputs->icUsed; i++)
    {
    sim_data->trace[i + total_number_of_inputs].data_labels = g_strdup (qcad_cell_get_label (exp_array_index_1d (design->bus_layout->outputs, BUS_LAYOUT_CELL, i).cell));
    sim_data->trace[i + total_number_of_inputs].drawtrace = TRUE;
    sim_data->trace[i + total_number_of_inputs].trace_function = QCAD_CELL_OUTPUT;
    sim_data->trace[i + total_number_of_inputs].data = g_malloc0 (sim_data->number_samples * sizeof (double));
    }

//  if (options->clocking == four_phase_clocking) {
  // create and initialize the clock data //
  sim_data->clock_data = g_malloc0 (sizeof (struct TRACEDATA) * 4);

  for (i = 0; i < 4; i++)
    {
    sim_data->clock_data[i].data_labels = g_strdup_printf ("CLOCK %d", i);
    sim_data->clock_data[i].drawtrace = 1;
    sim_data->clock_data[i].trace_function = QCAD_CELL_FIXED;
    if (NULL == (sim_data->clock_data[i].data = g_malloc0 (sim_data->number_samples * sizeof (double))))
      printf("Could not allocate memory for clock data\n");

    // fill in the clock data for the simulation results //
    for (j = 0; j<sim_data->number_samples; j++)
      //printf("j=%d, j*record_interval = %d\n",j,j*record_interval);
      sim_data->clock_data[i].data[j] = calculate_clock_value(i, j * record_interval, number_samples, total_number_of_inputs, options, SIMULATION_TYPE, pvt);
 //   }
  }
  // -- refresh all the kink energies and neighbours-- //
  coherence_refresh_all_Ek (number_of_cell_layers, number_of_cells_in_layer, sorted_cells, options);

  // -- sort the cells with respect to the neighbour count -- //
  // -- this is done so that majority gates are evalulated last -- //
  // -- to ensure that all the signals have arrived first -- //
  // -- kept getting wrong answers without this -- //

  // The following line causes a segfault when the design consists of a single cell
//  printf("The Ek to the first cells neighbour is %e [eV]\n",((coherence_model *)sorted_cells[0][0]->cell_model)->Ek[0]/1.602e-19);
	 
	  for (Nix = 0 ; Nix < number_of_cell_layers ; Nix++)
		  // ...perform as many swaps as there are cells therein
		  for (Nix1 = 0 ; Nix1 < number_of_cells_in_layer[Nix] ; Nix1++)
		  {
			  			  
			  idxCell1 = rand () % number_of_cells_in_layer[Nix] ;
			  idxCell2 = rand () % number_of_cells_in_layer[Nix] ;
			  
			  swap = sorted_cells[Nix][idxCell1] ;
			  sorted_cells[Nix][idxCell1] = sorted_cells[Nix][idxCell2] ;
			  sorted_cells[Nix][idxCell2] = swap ;
		  }
	   
	  
  if (EXHAUSTIVE_VERIFICATION == SIMULATION_TYPE)
    for (design_bus_layout_iter_first (design->bus_layout, &bli, QCAD_CELL_INPUT, &i) ; i > -1 ; design_bus_layout_iter_next (&bli, &i))
      qcad_cell_set_polarization (exp_array_index_1d (design->bus_layout->inputs, BUS_LAYOUT_CELL, i).cell, 
        sim_data->trace[i].data[0] = -1) ;
  else
//  if (VECTOR_TABLE == SIMULATION_TYPE)
    for (design_bus_layout_iter_first (design->bus_layout, &bli, QCAD_CELL_INPUT, &i) ; i > -1 ; design_bus_layout_iter_next (&bli, &i))
      if (exp_array_index_1d (pvt->inputs, VT_INPUT, i).active_flag)
        qcad_cell_set_polarization (exp_array_index_1d (pvt->inputs, VT_INPUT, i).input,
          sim_data->trace[i].data[0] = exp_array_index_2d (pvt->vectors, gboolean, 0, i) ? 1 : -1) ;

	  //Create Ek_matrix
	  Ek_matrix = (double**)malloc(number_of_cells_in_layer[0]*sizeof(double*));
	  for (i = 0; i < number_of_cells_in_layer[0]; i++) {
		  Ek_matrix[i] = (double*)malloc(number_of_cells_in_layer[0]*sizeof(double));
	  }
	  
	  for(i = 0; i < number_of_cells_in_layer[0]; i++) {
		  for(j = 0; j < number_of_cells_in_layer[0]; j++) {
			  if (i == j) {
				  Ek_matrix[i][j] = 0;
			  }
			  else {
				  Ek_matrix[i][j] = coherence_determine_Ek (sorted_cells[0][i], sorted_cells[0][j], 0, options);
			  }
		  }
	  }
	  	  
	  Pol = (double*)malloc(number_of_cells_in_layer[0]*sizeof(double));	  
	  lss = (double*)malloc(number_of_cells_in_layer[0]*sizeof(double));	
	  
	  
	  logic_sim (number_of_cell_layers, number_of_cells_in_layer, sorted_cells, Ek_matrix, total_number_of_inputs, options, SIMULATION_TYPE, pvt, Pol, lss);		
	  for (i = 0; i < number_of_cells_in_layer[0]; i++) {
		  printf("%f\n",lss[i]);
	  }
	  
	  double Ek_max = get_max(Ek_matrix, number_of_cells_in_layer[0]);
	  double over_Ek_max = 1/Ek_max;
	  //Set up Jacob's params
	  
	  //Find number of normal cells
	  num_normal = 0;
	  for (Nix = 0 ; Nix < number_of_cell_layers ; Nix++)
		  for (Nix1 = 0 ; Nix1 < number_of_cells_in_layer[Nix] ; Nix1++)
		  {
			  
			  if (((QCAD_CELL_INPUT == sorted_cells[Nix][Nix1]->cell_function) ||
				   (QCAD_CELL_FIXED == sorted_cells[Nix][Nix1]->cell_function)))
			  {
				  continue;
			  }
			  num_normal += 1;
		  }
	  
	  
	  //Allocate memory for Ek_no_inputs, Ekd, pd, and K
	  Ek_no_inputs = (float**)malloc(num_normal*sizeof(float*));
	  K = (int**)malloc(num_normal*sizeof(int*));
	  pd = (float*)malloc(total_number_of_inputs*sizeof(float));
	  clock_value = (int*)malloc(num_normal*sizeof(int));
	  Ekd = (float**)malloc(num_normal*sizeof(float));
	  for (i = 0; i < num_normal; i++) {
		  Ek_no_inputs[i] = (float*)malloc(num_normal*sizeof(float));
		  K[i] = (int*)malloc(num_normal*sizeof(int));
		  Ekd[i] = (float*)malloc(total_number_of_inputs*sizeof(float));
	  }
	  
	  //Fill in values for K
	  for(j = 1; j <= num_normal; j++){
		  for(k = 1; k <= num_normal; k++){
			  if( j > k ) {
				  K[j-1][k-1] = k + (j-1)*(3 + 6*(num_normal-1));
			  }
			  else { 
				  if ( j < k ) {
					  K[j-1][k-1] = k-1 + (j-1)*(3 + 6*(num_normal-1));
				  }
				  else {
					  K[j-1][k-1] = 0;
				  }
			  }
		  }
	  }
	  
	  //fill in values for pd, clock_value & Ek_no_inputs					  
	  k = -1;
	  l = 0;
	  q = 0;
	  for (i = 0; i < number_of_cells_in_layer[0]; i++) {
		  if (((QCAD_CELL_INPUT == sorted_cells[0][i]->cell_function) ||
			   (QCAD_CELL_FIXED == sorted_cells[0][i]->cell_function)))
		  {
			  pd[q] = qcad_cell_calculate_polarization (sorted_cells[0][i]);
			  q += 1;
			  continue;
		  }
		  k += 1;
		  l = 0;
		  for (j = 0; j < number_of_cells_in_layer[0]; j++) {
			  if (((QCAD_CELL_INPUT == sorted_cells[0][j]->cell_function) ||
				   (QCAD_CELL_FIXED == sorted_cells[0][j]->cell_function)))
			  {
				  continue;
			  }
			  if (i == j) {
				  Ek_no_inputs[k][l] = 0;
				  l +=1;
				  continue;
			  }			  
			  Ek_no_inputs[k][l] = over_Ek_max*coherence_determine_Ek (sorted_cells[0][i], sorted_cells[0][j], 0, options);
			  l += 1;
		  }
		  
		  clock_value[k] = sorted_cells[0][i]->cell_options.clock;
	  }
	  
	  
	  //fill in values for Ekd
	  k = 0;
	  l = 0;
	  for (i = 0; i < number_of_cells_in_layer[0]; i++) {
		  if (((QCAD_CELL_INPUT == sorted_cells[0][i]->cell_function) ||
			   (QCAD_CELL_FIXED == sorted_cells[0][i]->cell_function)))
		  {
			  for (j = 0; j < number_of_cells_in_layer[0]; j++) {
				  if (((QCAD_CELL_INPUT == sorted_cells[0][j]->cell_function) ||
					   (QCAD_CELL_FIXED == sorted_cells[0][j]->cell_function)))
				  {
					  continue;
				  }
				  else {
					  Ekd[k][l] =  over_Ek_max*coherence_determine_Ek (sorted_cells[0][i], sorted_cells[0][j], 0, options);
					  l += 1;
				  }
			  }
			  l = 0;
			  k += 1;
		  }
	  }
	  
	  
	  user_data.xx = 3;
	  user_data.xy = 3+(num_normal-1);
	  user_data.xz = 3+2*(num_normal-1);
	  user_data.yy = 3+3*(num_normal-1);
	  user_data.yz = 3+4*(num_normal-1);
	  user_data.zz = 3+5*(num_normal-1);
	  user_data.Ek = Ek_no_inputs;
	  user_data.pd = pd;
	  user_data.clock_value = clock_value;
	  user_data.K = K;
	  user_data.Ekd = Ekd;
	  user_data.n = num_normal;
	  user_data.nd = total_number_of_inputs; 
	  user_data.lss = lss;
	  user_data.options = options;
	  	  
	  //set up CVODE params
	  y = N_VNew_Serial(num_normal*(3+6*(num_normal-1)));
	  if (check_flag((void *)y, "N_VNew_Serial", 0)) return(1);
	  abstol = N_VNew_Serial(num_normal*(3+6*(num_normal-1))); 
	  if (check_flag((void *)abstol, "N_VNew_Serial", 0)) return(1);
	  
	  reltol = 1e-3;
	  
	  //Initialize y
	  gboolean stable0 = FALSE;
	  double EkP = 0;
	  int numel = num_normal*(3 + 6*(num_normal-1));
	  int numels = (3 + 6*(num_normal-1));
	  
	  N_Vector y_old = NULL;
	  y_old = N_VNew_Serial(num_normal*(3+6*(num_normal-1)));
	  if (check_flag((void *)y, "N_VNew_Serial", 0)) return(1);	  
	  
	  for (i = 1; i <= numel; i++) {
		  ((((N_VectorContent_Serial)(y->content))->data[i-1])) = 0;
		  ((((N_VectorContent_Serial)(y_old->content))->data[i-1])) = 0;
		  Ith(abstol,i) = 1e-3;
	  }
	    
	  int iter = 0;
	  int stable_flag = 1;
	  while (!stable0) {
		  stable_flag = 1;
		  for (i = 1; i <= num_normal; i++) {
			  EkP = 0;
			  for (j = 1; j <= num_normal; j++) {
				  EkP += -Ith(y,numel*(j-1)+3)*Ek_no_inputs[i-1][j-1];
			  }
			  for (j = 1; j <= total_number_of_inputs; j++) {
				  EkP += pd[j-1]*Ekd[j-1][i-1];
			  }
			  /*
			  Ith(y,numels*(i-1)+1) = lambda_ss_x(0, EkP, sim_data->clock_data[clock_value[i-1]].data[0], options); 
			  Ith(y,numels*(i-1)+2) = lambda_ss_y(0, EkP, sim_data->clock_data[clock_value[i-1]].data[0], options);
			  Ith(y,numels*(i-1)+3) = lambda_ss_z(0, EkP, sim_data->clock_data[clock_value[i-1]].data[0], options);
			  */
			  Ith(y,numels*(i-1)+1) = lambda_ss_x(0, EkP, 100, options); 
			  Ith(y,numels*(i-1)+2) = lambda_ss_y(0, EkP, 100, options);
			  Ith(y,numels*(i-1)+3) = lambda_ss_z(0, EkP, 100, options);
			   
			  if (stable_flag) {
				  stable0 =
				  !(fabs (Ith(y,numels*(i-1)+1) - Ith(y_old,numels*(i-1)+1)) > 1e-7 ||
					fabs (Ith(y,numels*(i-1)+2) - Ith(y_old,numels*(i-1)+2)) > 1e-7 ||
					fabs (Ith(y,numels*(i-1)+3) - Ith(y_old,numels*(i-1)+3)) > 1e-7) ;
				  
			  }
			  if (!stable0) {
				  stable_flag = 0;
			  }
			  Ith(y_old,numels*(i-1)+1) = Ith(y,numels*(i-1)+1);
			  Ith(y_old,numels*(i-1)+2) = Ith(y,numels*(i-1)+2);
			  Ith(y_old,numels*(i-1)+3) = Ith(y,numels*(i-1)+3);
		  }
		  iter += 1;  
	  }
	  
	  command_history_message ("It took %d iterations to converge the initial steady state polarization\n", iter);
	  
	  for (i = 1; i <= num_normal; i++) {
		  k = 1;
		  for (j = 4; j<= 2+num_normal; j++) {
			  if (k == i) {
				  k += 1;
			  }
			  Ith(y,numels*(i-1)+j) = Ith(y,numels*(i-1)+1)*Ith(y,numels*(k-1)+1); //Kxx
			  Ith(y,numels*(i-1)+j+(num_normal-1)) = Ith(y,numels*(i-1)+1)*Ith(y,numels*(k-1)+2); //Kxy
			  Ith(y,numels*(i-1)+j+2*(num_normal-1)) = Ith(y,numels*(i-1)+1)*Ith(y,numels*(k-1)+3); //Kxz
			  Ith(y,numels*(i-1)+j+3*(num_normal-1)) = Ith(y,numels*(i-1)+2)*Ith(y,numels*(k-1)+2); //Kyy
			  Ith(y,numels*(i-1)+j+4*(num_normal-1)) = Ith(y,numels*(i-1)+2)*Ith(y,numels*(k-1)+3); //Kyz
			  Ith(y,numels*(i-1)+j+5*(num_normal-1)) = Ith(y,numels*(i-1)+3)*Ith(y,numels*(k-1)+3); //Kzz
			  k += 1;
		  }
	  }
		  
	  cvode_mem = CVodeCreate(CV_BDF, CV_NEWTON);
	  if (check_flag((void *)cvode_mem, "CVodeCreate", 0)) return(1);
	  
	  flag = CVodeInit(cvode_mem, twopcorr, 0, y);
	  if (check_flag(&flag, "CVodeInit", 1)) return(1);
	  
	  flag = CVodeSVtolerances(cvode_mem, reltol, abstol);
	  if (check_flag(&flag, "CVodeSVtolerances", 1)) return(1);
	  
	  flag = CVDense(cvode_mem, numel);
	  if (check_flag(&flag, "CVDense", 1)) return(1);
	  
	  flag = CVodeSetMaxOrd(cvode_mem,1);
	  if (check_flag(&flag, "CVodeSetMaxOrd", 1)) return(1);
	  
	  flag = CVodeSetUserData(cvode_mem, &user_data);
	  if (check_flag(&flag, "CVodeSetUserData", 1)) return(1);
	  
	  flag = CVodeSetMaxNumSteps(cvode_mem, 1000);
	  if (check_flag(&flag, "CVodeSetMaxNumSteps", 1)) return(1);
	  
	  printf("All CVODE parameters set\n");

	  
	/*  
  // Converge the steady state coherence vector for each cell so that the simulation starts without any transients //
  stable = FALSE;
  k = 0;
  while (!stable)
    {
    stable = TRUE;

    for (i = 0; i < number_of_cell_layers; i++)
      for (j = 0; j < number_of_cells_in_layer[i]; j++)
        {
        if (((QCAD_CELL_INPUT == sorted_cells[i][j]->cell_function)||
             (QCAD_CELL_FIXED == sorted_cells[i][j]->cell_function)))
          {
          j++;
          continue;
          }

        PEk = 0;
        // Calculate the sum of neighboring polarizations * the kink energy between them//
        for (q = 0; q < ((coherence_model *)sorted_cells[i][j]->cell_model)->number_of_neighbours; q++)
          PEk += (qcad_cell_calculate_polarization (((coherence_model *)sorted_cells[i][j]->cell_model)->neighbours[q])) * ((coherence_model *)sorted_cells[i][j]->cell_model)->Ek[q];

        old_lambda_x = ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_x;
        old_lambda_y = ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_y;
        old_lambda_z = ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_z;

        ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_x = lambda_ss_x(0, PEk, sim_data->clock_data[sorted_cells[i][j]->cell_options.clock].data[0], options);
        ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_y = lambda_ss_y(0, PEk, sim_data->clock_data[sorted_cells[i][j]->cell_options.clock].data[0], options);
        ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_z = lambda_ss_z(0, PEk, sim_data->clock_data[sorted_cells[i][j]->cell_options.clock].data[0], options);

        qcad_cell_set_polarization(sorted_cells[i][j], ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_z);

        // if the lambda values are different by more then the tolerance then they have not converged //
        stable =
          !(fabs (((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_x - old_lambda_x) > 1e-7 ||
            fabs (((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_y - old_lambda_y) > 1e-7 ||
            fabs (((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_z - old_lambda_z) > 1e-7) ;
        }
    k++;
    }

  command_history_message ("It took %d iterations to converge the initial steady state polarization\n", k);

	 
  // perform the iterations over all samples //
  for (j = 0; j < number_samples; j++)
    {
    if (0 == j % 10000 || j == number_samples - 1)
      {
      // Update the progress bar
      set_progress_bar_fraction ((float) j / (float) number_samples) ;
      // redraw the design if the user wants it to appear animated or if this is the last sample //
	 */
#ifdef DESIGNER
      if(options->animate_simulation || j == number_samples - 1)
        {
        redraw_async(NULL);
        gdk_flush () ;
        }
#endif /* def DESIGNER */
      // -- for each of the inputs -- //

    if (EXHAUSTIVE_VERIFICATION == SIMULATION_TYPE)
      for (idxMasterBitOrder = 0, design_bus_layout_iter_first (design->bus_layout, &bli, QCAD_CELL_INPUT, &i) ; i > -1 ; design_bus_layout_iter_next (&bli, &i), idxMasterBitOrder++)
        {
        qcad_cell_set_polarization (exp_array_index_1d (design->bus_layout->inputs, BUS_LAYOUT_CELL, i).cell,
          dPolarization = (-sin (((double) (1 << idxMasterBitOrder)) * (double) j * optimization_options.four_pi_over_number_samples)) > 0 ? 1 : -1) ;
        if (0 == j % record_interval)
          sim_data->trace[i].data[j/record_interval] = dPolarization ;
        }
    else
//    if (VECTOR_TABLE == SIMULATION_TYPE)
      for (design_bus_layout_iter_first (design->bus_layout, &bli, QCAD_CELL_INPUT, &i) ; i > -1 ; design_bus_layout_iter_next (&bli, &i))
        if (exp_array_index_1d (pvt->inputs, VT_INPUT, i).active_flag)
          {
          qcad_cell_set_polarization (exp_array_index_1d (pvt->inputs, VT_INPUT, i).input,
            dPolarization = exp_array_index_2d (pvt->vectors, gboolean, (j*pvt->vectors->icUsed) / number_samples, i) ? 1 : -1) ;
          if (0 == j % record_interval)
            sim_data->trace[i].data[j/record_interval] = dPolarization ;
          }
/*
    if (0 == j % record_interval)
      {
      for (design_bus_layout_iter_first (design->bus_layout, &bli, QCAD_CELL_INPUT, &i) ; i > -1 ; design_bus_layout_iter_next (&bli, &i))
        sim_data->trace[i].data[j/record_interval] =
          qcad_cell_calculate_polarization (exp_array_index_1d (design->bus_layout->inputs, BUS_LAYOUT_CELL, i).cell) ;
      }
*/
		
    // -- run the iteration with the given clock value -- //
    //run_coherence_iteration (j, number_of_cell_layers, number_of_cells_in_layer, sorted_cells, total_number_of_inputs, number_samples, options, sim_data, SIMULATION_TYPE, pvt, energy);
iout = 0;
tout = options->time_step;
printf("Starting ODE Solver Now...\n");
while(1) {
	flag = CVode(cvode_mem, tout, y, &t, CV_NORMAL);
	for (design_bus_layout_iter_first (design->bus_layout, &bli, QCAD_CELL_OUTPUT, &i) ; i > -1 ; design_bus_layout_iter_next (&bli, &i))
        sim_data->trace[total_number_of_inputs + i].data[j/record_interval] = 1 ;
	
	if (TRUE == STOP_SIMULATION) return sim_data;
	
	if (check_flag(&flag, "CVode", 1)) {	
		break;
	}
    if (flag == CV_SUCCESS) {
		iout++;
		tout += options->time_step;
    }
	
	if (tout >= options->duration) {
		break;
	}
}

/*
	// -- Calculate Power -- //
	if(j>=1)
		//printf("%e\n",(energy[j]-energy[j-1])/options->time_step);
		//printf("%e\n", energy[j]);
		average_power+=(energy[j]-energy[j-1])/options->time_step;

    // -- Set the cell polarizations to the lambda_z value -- //
    for (k = 0; k < number_of_cell_layers; k++)
      for (l = 0; l < number_of_cells_in_layer[k]; l++)
        {
        // don't simulate the input and fixed cells //
        if (((QCAD_CELL_INPUT == sorted_cells[k][l]->cell_function) ||
             (QCAD_CELL_FIXED == sorted_cells[k][l]->cell_function)))
          continue;
        if (fabs (((coherence_model *)sorted_cells[k][l]->cell_model)->lambda_z) > 1.0)
          {
          command_history_message ("I had to abort the simulation at iteration %d because the polarization = %e was diverging.\nPossible cause is the time step is too large.\nAlternatively, you can decrease the relaxation time to reduce oscillations.\n",j, ((coherence_model *)sorted_cells[k][l]->cell_model)->lambda_z);
          command_history_message ("time step was set to %e\n", options->time_step);
          return sim_data;
          }
        qcad_cell_set_polarization (sorted_cells[k][l], -((coherence_model *)sorted_cells[k][l]->cell_model)->lambda_z);
        }

    // -- collect all the output data from the simulation -- //
    if (0 == j % record_interval)
      for (design_bus_layout_iter_first (design->bus_layout, &bli, QCAD_CELL_OUTPUT, &i) ; i > -1 ; design_bus_layout_iter_next (&bli, &i))
        sim_data->trace[total_number_of_inputs + i].data[j/record_interval] =
          qcad_cell_calculate_polarization (exp_array_index_1d (design->bus_layout->outputs, BUS_LAYOUT_CELL, i).cell) ;

  if (TRUE == STOP_SIMULATION) return sim_data;

  }//for number of samples

*/

  // Free the neigbours and Ek array introduced by this simulation//
  for (k = 0; k < number_of_cell_layers; k++)
    for (l = 0; l < number_of_cells_in_layer[k]; l++)
      {
      g_free (((coherence_model *)sorted_cells[k][l]->cell_model)->neighbours);
      g_free (((coherence_model *)sorted_cells[k][l]->cell_model)->neighbour_layer);
      g_free (((coherence_model *)sorted_cells[k][l]->cell_model)->Ek);
      }

  simulation_inproc_data_free (&number_of_cell_layers, &number_of_cells_in_layer, &sorted_cells) ;

  g_free(energy);

  // Restore the input flag for the inactive inputs
  if (VECTOR_TABLE == SIMULATION_TYPE)
    for (i = 0 ; i < pvt->inputs->icUsed ; i++)
      exp_array_index_1d (pvt->inputs, BUS_LAYOUT_CELL, i).cell->cell_function = QCAD_CELL_INPUT ;

  // -- get and print the total simulation time -- //
  if ((end_time = time (NULL)) < 0)
    fprintf (stderr, "Could not get end time\n");

  command_history_message (_("Total simulation time: %g s\n"), (double)(end_time - start_time));
  set_progress_bar_visible (FALSE) ;
  return sim_data;
  }//run_coherence

// -- completes one simulation iteration performs the approximations until the entire design has stabalized -- //
static void run_coherence_iteration (int sample_number, int number_of_cell_layers, int *number_of_cells_in_layer, QCADCell ***sorted_cells, int total_number_of_inputs, unsigned long int number_samples, const coherence_OP *options, simulation_data *sim_data, int SIMULATION_TYPE, VectorTable *pvt, double *energy)
  {
  unsigned int i,j,q;
  double lambda_x_new;
  double lambda_y_new;
  double lambda_z_new;
  double lambda_x;
  double lambda_y;
  double lambda_z;
  double PEk;
  double t;
  double clock_value;
  unsigned int num_neighbours;
  double Gamma[4][3];
	  unsigned long int number_recorded_samples = 3000;
	  unsigned long int record_interval;
	  	  
	  // if the number of samples is larger then the number of recorded samples then change the
	  // time step to ensure only number_recorded_samples is used //
	  if (number_recorded_samples >= number_samples)
	  {
		  number_recorded_samples = number_samples;
		  record_interval = 1;
	  }
	  else
		  record_interval = (unsigned long int)ceil ((double)(number_samples - 1) / (double)(number_recorded_samples));
	  
	  
  t = options->time_step * (double)sample_number;

	// precalculte the clock value for each of the four clocking zones
	Gamma[0][0] = calculate_clock_value(0, sample_number, number_samples, total_number_of_inputs, options, SIMULATION_TYPE, pvt);
	Gamma[1][0] = calculate_clock_value(1, sample_number, number_samples, total_number_of_inputs, options, SIMULATION_TYPE, pvt);
	Gamma[2][0] = calculate_clock_value(2, sample_number, number_samples, total_number_of_inputs, options, SIMULATION_TYPE, pvt);
	Gamma[3][0] = calculate_clock_value(3, sample_number, number_samples, total_number_of_inputs, options, SIMULATION_TYPE, pvt);
	
	if(options->algorithm == RUNGE_KUTTA){
		Gamma[0][2] = calculate_clock_value(0, sample_number+1, number_samples, total_number_of_inputs, options, SIMULATION_TYPE, pvt);
		Gamma[0][1] = (Gamma[0][0]+Gamma[0][2])/2.0;
	
		Gamma[1][2] = calculate_clock_value(1, sample_number+1, number_samples, total_number_of_inputs, options, SIMULATION_TYPE, pvt);
		Gamma[1][1] = (Gamma[1][0]+Gamma[1][2])/2.0;
	
		Gamma[2][2] = calculate_clock_value(2, sample_number+1, number_samples, total_number_of_inputs, options, SIMULATION_TYPE, pvt);
		Gamma[2][1] = (Gamma[2][0]+Gamma[2][2])/2.0;
	
		Gamma[3][2] = calculate_clock_value(3, sample_number+1, number_samples, total_number_of_inputs, options, SIMULATION_TYPE, pvt);
		Gamma[3][1] = (Gamma[3][0]+Gamma[3][2])/2.0;	
	}
	
  energy[sample_number]=0;
  // loop through all the cells in the design //
  for (i = 0 ; i < number_of_cell_layers ; i++)
    for (j = 0 ; j < number_of_cells_in_layer[i] ; j++)
      {
      // don't simulate the input and fixed cells //
      if (((QCAD_CELL_INPUT == sorted_cells[i][j]->cell_function) ||
           (QCAD_CELL_FIXED == sorted_cells[i][j]->cell_function)))
        continue;

 //Added by Faizal for cont. clocking	
	if(FOUR_PHASE_CLOCKING == options->clocking){
      clock_value = calculate_clock_value(sorted_cells[i][j]->cell_options.clock, sample_number, number_samples, total_number_of_inputs, options, SIMULATION_TYPE, pvt); 
    }
    else {  
	  clock_value = calculate_clock_value_cc(sorted_cells[i][j], sample_number, number_samples, total_number_of_inputs, options, SIMULATION_TYPE, pvt);
	}
//	  printf("%e\n", clock_value/1e-19);
//End added by Faizal
	  
      PEk = 0;
      // Calculate the sum of neighboring polarizations //
      num_neighbours = ((coherence_model *)sorted_cells[i][j]->cell_model)->number_of_neighbours;
      for (q = 0 ; q < num_neighbours ; q++)
        PEk += (qcad_cell_calculate_polarization (((coherence_model *)sorted_cells[i][j]->cell_model)->neighbours[q]))*((coherence_model *)sorted_cells[i][j]->cell_model)->Ek[q];

      lambda_x = ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_x;
      lambda_y = ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_y;
      lambda_z = ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_z;

      lambda_x_new = lambda_x_next (t, PEk, clock_value, lambda_x, lambda_y, lambda_z, options);
      lambda_y_new = lambda_y_next (t, PEk, clock_value, lambda_x, lambda_y, lambda_z, options);
      lambda_z_new = lambda_z_next (t, PEk, clock_value, lambda_x, lambda_y, lambda_z, options);
			  
		energy[sample_number] += (-Gamma[sorted_cells[i][j]->cell_options.clock][0]*lambda_x_new + PEk*lambda_z_new/2);

      ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_x = lambda_x_new;
      ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_y = lambda_y_new;
      ((coherence_model *)sorted_cells[i][j]->cell_model)->lambda_z = lambda_z_new;
      }
}//run_iteration

//-------------------------------------------------------------------//
// -- refreshes the array of Ek values for each cell in the design this is done to speed up the simulation
// since we can assume no design changes durring the simulation we can precompute all the Ek values then
// use them as necessary throughout the simulation -- //
static void coherence_refresh_all_Ek (int number_of_cell_layers, int *number_of_cells_in_layer, QCADCell ***sorted_cells, coherence_OP *options)
  {
  int icNeighbours = 0 ;
  coherence_model *cell_model = NULL ;
  int i,j,k;

  // calculate the Ek for each cell //
  for(i = 0 ; i < number_of_cell_layers ; i++)
    for(j = 0 ; j < number_of_cells_in_layer[i] ; j++)
      {
      // free up memory from previous simulations //
      g_free ((cell_model = (coherence_model *)sorted_cells[i][j]->cell_model)->neighbours);
      g_free (cell_model->Ek);
      g_free (cell_model->neighbour_layer);
      cell_model->neighbours = NULL;
      cell_model->neighbour_layer = NULL;
      cell_model->Ek = NULL;

      // select all neighbours within the provided radius //
      cell_model->number_of_neighbours = icNeighbours =
        select_cells_in_radius(sorted_cells, sorted_cells[i][j], ((coherence_OP *)options)->radius_of_effect, i, number_of_cell_layers, number_of_cells_in_layer,
             ((coherence_OP *)options)->layer_separation, &(cell_model->neighbours), (int **)&(cell_model->neighbour_layer));

      //printf("number of neighbors = %d\n", icNeighbours);

      if (icNeighbours > 0)
        {
        cell_model->Ek = g_malloc0 (sizeof (double) * icNeighbours);

        // ensure no memory allocation error has ocurred //
        if (((coherence_model *)sorted_cells[i][j]->cell_model)->neighbours == NULL ||
            ((coherence_model *)sorted_cells[i][j]->cell_model)->Ek == NULL)
          //printf ("memory allocation error in refresh_all_Ek()\n");
          exit (1);

        for (k = 0; k < icNeighbours; k++)
          //if(cell_model->neighbours[k]==NULL)printf("Null neighbour prior to passing into determine Ek for k = %d\n", k);
          // set the Ek of this cell and its neighbour //
          cell_model->Ek[k] = coherence_determine_Ek (sorted_cells[i][j], cell_model->neighbours[k], ABS(i-cell_model->neighbour_layer[k]), options);
          //printf("Ek = %e\n", cell_model->Ek[k]/1.602e-19);
        }
      }
  }//refresh_all_Ek

//-------------------------------------------------------------------//
// Determines the Kink energy of one cell with respect to another this is defined as the energy of those
// cells having opposite polarization minus the energy of those two cells having the same polarization -- //
static double coherence_determine_Ek (QCADCell * cell1, QCADCell * cell2, int layer_separation, coherence_OP *options)
  {
  int k;
  int j;

  double distance = 0;
  double Constant = 1 / (4 * PI * EPSILON * options->epsilonR);

  double charge1[4] = { -HALF_QCHARGE,  HALF_QCHARGE, -HALF_QCHARGE,  HALF_QCHARGE };
  double charge2[4] = {  HALF_QCHARGE, -HALF_QCHARGE,  HALF_QCHARGE, -HALF_QCHARGE };

  double EnergyDiff = 0;
  double EnergySame = 0;

  g_assert (cell1 != NULL);
  g_assert (cell2 != NULL);
  g_assert (cell1 != cell2);

  for (k = 0; k < cell1->number_of_dots; k++)
    for (j = 0; j < cell2->number_of_dots; j++)
      {
      // determine the distance between the dots //
      // printf("layer seperation = %d\n", layer_seperation);
      distance = determine_distance (cell1, cell2, k, j, (double)layer_separation * ((coherence_OP *)options)->layer_separation);
      g_assert (distance != 0);

      EnergyDiff += Constant * (charge1[k] * charge2[j]) / (distance*1e-9);
      EnergySame += Constant * (charge1[k] * charge1[j]) / (distance*1e-9);
      }//for other dots

  //printf("Ek = %e\n", (EnergyDiff - EnergySame)/ 1.602e-19);

  return EnergyDiff - EnergySame;
  }// coherence_determine_Ek

//-------------------------------------------------------------------//
//-------------------------------------------------------------------//
// Calculates the clock data at a particular sample
static inline double calculate_clock_value (unsigned int clock_num, unsigned long int sample, unsigned long int number_samples, int total_number_of_inputs, const coherence_OP *options, int SIMULATION_TYPE, VectorTable *pvt)
  {
  double clock = 0;
  int jitter_phases[4] = {options->jitter_phase_0, options->jitter_phase_1,
                          options->jitter_phase_2, options->jitter_phase_3} ;

//Added by Marco: phase shift included in (-PI/2, +P/2) with steps of (1/200)PI
//Edited by Konrad; Above is wrong, changed jitter to be actual phase shift shift = jitter/180*PI

  if (SIMULATION_TYPE == EXHAUSTIVE_VERIFICATION)
    {
    clock = optimization_options.clock_prefactor *
      cos (((double)(1 << total_number_of_inputs)) * (double)sample * optimization_options.four_pi_over_number_samples - (double)((jitter_phases[clock_num]) / 180.0) * PI  - PI * (double)clock_num * 0.5) + optimization_options.clock_shift + options->clock_shift;

    // Saturate the clock at the clock high and low values
    clock = CLAMP (clock, options->clock_low, options->clock_high) ;
    }
  else
  if (SIMULATION_TYPE == VECTOR_TABLE)
    {
    clock = optimization_options.clock_prefactor *
      cos (((double)pvt->vectors->icUsed) * (double)sample * optimization_options.two_pi_over_number_samples - (double)((jitter_phases[clock_num]) / 180.0) * PI  - PI * (double)clock_num * 0.5) + optimization_options.clock_shift + options->clock_shift;

    // Saturate the clock at the clock high and low values
    clock = CLAMP (clock, options->clock_low, options->clock_high) ;
    }
    
//End added by Marco
  return clock;
  }// calculate_clock_value


static inline double calculate_clock_value_cc (QCADCell *cell, unsigned long int sample, unsigned long int number_samples, int total_number_of_inputs, const coherence_OP *options, int SIMULATION_TYPE, VectorTable *pvt) //Added by Faizal for cont. clocking
  {
  double clock = 0;

  if (SIMULATION_TYPE == EXHAUSTIVE_VERIFICATION)
    {
    clock = optimization_options.clock_prefactor *
      cos (((double)(1 << total_number_of_inputs)) * (double)sample * optimization_options.four_pi_over_number_samples - options->wave_number_kx * QCAD_DESIGN_OBJECT (cell)->x - options->wave_number_ky * QCAD_DESIGN_OBJECT (cell)->y) + optimization_options.clock_shift + options->clock_shift;
	  
    // Saturate the clock at the clock high and low values
    clock = CLAMP (clock, options->clock_low, options->clock_high) ;
    }
  else
  if (SIMULATION_TYPE == VECTOR_TABLE)
    {
    clock = optimization_options.clock_prefactor *
      cos (((double)pvt->vectors->icUsed) * (double)sample * optimization_options.two_pi_over_number_samples - options->wave_number_kx * QCAD_DESIGN_OBJECT (cell)->x - options->wave_number_ky * QCAD_DESIGN_OBJECT (cell)->y) + optimization_options.clock_shift + options->clock_shift;

    // Saturate the clock at the clock high and low values
    clock = CLAMP (clock, options->clock_low, options->clock_high) ;
    }
    
  return clock;
  }// calculate_clock_value_cc


//-------------------------------------------------------------------//

// Next value of lambda x with choice of algorithm
static inline double lambda_x_next (double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_OP *options)
  {
  double k1 = options->time_step * slope_x (t, PEk, Gamma, lambda_x, lambda_y, lambda_z, options);
  double k2, k3, k4;

  if (RUNGE_KUTTA == options->algorithm)
    {
    k2 = options->time_step * slope_x (t, PEk, Gamma, lambda_x + k1/2, lambda_y, lambda_z, options);
    k3 = options->time_step * slope_x (t, PEk, Gamma, lambda_x + k2/2, lambda_y, lambda_z, options);
    k4 = options->time_step * slope_x (t, PEk, Gamma, lambda_x + k3,   lambda_y, lambda_z, options);
    return lambda_x + k1/6 + k2/3 + k3/3 + k4/6;
    }
  else
  if (EULER_METHOD == options->algorithm)
    return lambda_x + k1;
  else
    command_history_message ("coherence vector undefined algorithm\n");

  return 0;
  }

// Next value of lambda y with choice of algorithm
static inline double lambda_y_next (double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_OP *options)
  {
  double k1 = options->time_step * slope_y (t, PEk, Gamma, lambda_x, lambda_y, lambda_z, options);
  double k2, k3, k4;

  if (RUNGE_KUTTA == options->algorithm)
    {
    k2 = options->time_step * slope_y (t, PEk, Gamma, lambda_x, lambda_y + k1/2, lambda_z, options);
    k3 = options->time_step * slope_y (t, PEk, Gamma, lambda_x, lambda_y + k2/2, lambda_z, options);
    k4 = options->time_step * slope_y (t, PEk, Gamma, lambda_x, lambda_y + k3,   lambda_z, options);
    return lambda_y + k1/6 + k2/3 + k3/3 + k4/6;
    }
  else
  if (EULER_METHOD == options->algorithm)
    return lambda_y + k1;
  else
    command_history_message("coherence vector undefined algorithm\n");

  return 0;
  }

// Next value of lambda z with choice of algorithm
static inline double lambda_z_next (double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_OP *options)
  {
  double k1 = options->time_step * slope_z (t, PEk, Gamma, lambda_x, lambda_y, lambda_z, options);
  double k2, k3, k4;

  if (RUNGE_KUTTA == options->algorithm)
    {
    k2 = options->time_step * slope_z(t, PEk, Gamma, lambda_x, lambda_y, lambda_z + k1/2, options);
    k3 = options->time_step * slope_z(t, PEk, Gamma, lambda_x, lambda_y, lambda_z + k2/2, options);
    k4 = options->time_step * slope_z(t, PEk, Gamma, lambda_x, lambda_y, lambda_z + k3,   options);
    return lambda_z + k1/6 + k2/3 + k3/3 + k4/6;
    }
  else
  if (EULER_METHOD == options->algorithm)
    return lambda_z + k1;
  else
    command_history_message("coherence vector undefined algorithm\n");

  return 0;
  }

static inline double slope_x (double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_OP *options)
  {
  double mag = magnitude_energy_vector (PEk, Gamma);
  return ((2.0 * Gamma * over_hbar / mag * tanh (optimization_options.hbar_over_kBT * mag * 0.5) - lambda_x) / options->relaxation - (PEk * lambda_y * over_hbar));
  }

static inline double slope_y (double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_OP *options)
  {return (options->relaxation * (PEk * lambda_x + 2.0 * Gamma * lambda_z) - hbar * lambda_y) / (options->relaxation * hbar);}

static inline double slope_z (double t, double PEk, double Gamma, double lambda_x, double lambda_y, double lambda_z, const coherence_OP *options)
  {
  double mag = magnitude_energy_vector (PEk, Gamma);
  return (-PEk * tanh (optimization_options.hbar_over_kBT * mag * 0.5) - mag * (2.0 * Gamma * options->relaxation * lambda_y + hbar * lambda_z)) / (options->relaxation * hbar * mag);
  }

//-------------------------------------------------------------------------------------------------------------------------//

// Steady-State Coherence Vector X component
static inline double lambda_ss_x(double t, double PEk, double Gamma, const coherence_OP *options)
  {
  return 2.0 * Gamma * over_hbar / magnitude_energy_vector(PEk, Gamma) * tanh (temp_ratio (PEk, Gamma, options->T));}

// Steady-State Coherence Vector y component
static inline double lambda_ss_y (double t, double PEk, double Gamma, const coherence_OP *options)
  {return 0.0;}

// Steady-State Coherence Vector Z component
static inline double lambda_ss_z(double t, double PEk, double Gamma, const coherence_OP *options)
  {return - PEk * over_hbar / magnitude_energy_vector (PEk, Gamma) * tanh (temp_ratio (PEk, Gamma, options->T));}

static int compareCoherenceQCells (const void *p1, const void *p2)
  {
  return
    ((coherence_model *)((*((QCADCell **)(p1)))->cell_model))->number_of_neighbours >
    ((coherence_model *)((*((QCADCell **)(p2)))->cell_model))->number_of_neighbours ?  1 :
    ((coherence_model *)((*((QCADCell **)(p1)))->cell_model))->number_of_neighbours <
    ((coherence_model *)((*((QCADCell **)(p2)))->cell_model))->number_of_neighbours ? -1 : 0 ;
  }//compareSortStructs

static inline void logic_sim (int number_of_cell_layers, int *number_of_cells_in_layer, QCADCell ***sorted_cells, double **Ek_matrix, int total_number_of_inputs, const coherence_OP *options, int SIMULATION_TYPE, VectorTable *pvt, double *Pol, double *lss)
	{
		double Ek_max, Ek_min;
		int *fork;
		double *Pdriver;
		int *driver_ind;
		int done;
		
		int i, j, k;
		int num_NN, num_pol, NN_pols;
		int num_drivers = 0;
		int num_cells = number_of_cells_in_layer[0];
		int Pol_sum;
		int un_pol, un_Ek, done_ind, pol_val;
		
		Ek_max = get_max(Ek_matrix, num_cells);
		Ek_min = get_min(Ek_matrix, num_cells);
				
		fork = (int*)malloc(num_cells*sizeof(int));
		Pdriver = (double*)malloc(num_cells*sizeof(double));
		driver_ind = (int*)malloc(num_cells*sizeof(int));
		
		for(i = 0 ; i < number_of_cell_layers ; i++)
			for(j = 0 ; j < number_of_cells_in_layer[i] ; j++)
			{
				if (((QCAD_CELL_INPUT == sorted_cells[i][j]->cell_function) ||
					 (QCAD_CELL_FIXED == sorted_cells[i][j]->cell_function))) {
					Pol[j] = qcad_cell_calculate_polarization (sorted_cells[i][j]);
					Pdriver[j] = Pol[j];
					driver_ind[j] = j;
					num_drivers += 1;
				}										   
				else {
					Pol[j] = 2;
					Pdriver[j] = 0;
					driver_ind[j] = -1;
				}
				fork[j] = 0;
				lss[j] = 0;
			}
		
		done = 1;
		while (done) {
			calc_pol(Pol, Ek_matrix, Pdriver, driver_ind, num_drivers, fork, Ek_max, num_cells);
			calc_poli(Pol,Ek_matrix, Ek_min, Ek_max, num_cells, fork);
			
			Pol_sum = 0;
			
			for (i = 0; i < num_cells; i++) {
				if (fork[i] == 0) {
					continue;
				}
				else {
					num_NN = search_matrix_row(Ek_matrix, Ek_max, fork[i], num_cells);
					num_pol = find_array(Pol, 2.0, num_cells);
					NN_pols = num_NN - num_pol;
					if (NN_pols < num_pol) {
						continue;
					}
					else {
						for (j = 0; j < num_pol; j++) {
							un_pol = find_in_array(Pol,2,num_cells,j);
							Pol[un_pol] = 0;
						}
						for (j = 0; j < num_NN; j++) {
							pol_val = find_matrix_row(Ek_matrix,Ek_max,fork[i],num_cells,j);
							Pol_sum += Pol[pol_val];
						}
						
						if (Pol_sum == 0) {
							Pol[fork[i]] = 2;
							for (j = 0; j < num_pol; j++) {
								un_pol = find_in_array(Pol,0,num_cells,j);
								Pol[un_pol] = 2.0;
							}
						}
						else {
							for (j = 0; j < num_NN; j++) {
								un_Ek = find_matrix_row(Ek_matrix,Ek_max,fork[i],num_cells,j);
								Ek_matrix[fork[i]][un_Ek] = 0;
							}						
							if (Pol_sum >= 1) {
								Pol[fork[i]] = 1.0;
								for (j = 0; j < num_pol; j++) {
									un_pol = find_in_array(Pol,0,num_cells,j);
									Pol[un_pol] = 2.0;
									Ek_matrix[fork[i]][un_pol] = Ek_max;
								}
							}
							else {
								Pol[fork[i]] = -1;
								for (j = 0; j < num_pol; j++) {
									un_pol = find_in_array(Pol,0,num_cells,j);
									Pol[un_pol] = 2.0;
									Ek_matrix[fork[i]][un_pol] = Ek_max;
								}	
							}
						}
					}
				}
			}
			done_ind = search_array(Pol,2.0,num_cells);
			if (done_ind == -1) {
				done = 0;
			}
			else {
				num_drivers = 0;
				for (j = 0; j < num_cells; j++) {
					if (fork[j] == 0) {
						driver_ind[j] = 0;
						Pdriver[j] = 0;
					}
					else {
						num_drivers += 1;
						driver_ind[j] = fork[j];
						Pdriver[j] = Pol[fork[j]];
						fork[j] = 0;
					}
				}
			}
		}
		
		prob_pols (Pol, lss, Ek_matrix, Ek_max, Ek_min, num_cells, number_of_cell_layers, number_of_cells_in_layer, sorted_cells, options);
		
		free(Pdriver); Pdriver = NULL;
		free(driver_ind); driver_ind = NULL;
		free(fork); fork = NULL;
	}
	

static inline void calc_pol (double *Pol, double **Ek_matrix, double *Pdriver, int *driver_ind, int num_drivers, int *fork, double Ek_max, int num_cells)
	{
		int i, j, k;
		int num_NN;
		int neigh_ind;
		int fork_ind;
		
		int *new_ind = NULL;
		double *new_Pdriver = NULL;
		
		for (i = 0; i < num_cells; i++) {
			if (driver_ind[i] == -1) {
				continue;
			}
			num_NN = search_matrix_row(Ek_matrix, Ek_max, driver_ind[i], num_cells);
			
			new_ind = (int*)malloc(num_cells*sizeof(int));
			new_Pdriver = (double*)malloc(num_cells*sizeof(double));
			
			if (num_NN > 2) {
				Pol[driver_ind[i]] = 2;
				fork_ind = search_array_int(fork, driver_ind[i], num_cells);
				if (fork_ind == -1) {
					fork[i] = driver_ind[i];
				}
			}
			else {
				for (j = 0; j < num_cells; j++) {
					neigh_ind = find_matrix_row(Ek_matrix, Ek_max, driver_ind[i], num_cells, j);
					if (neigh_ind == -1) {
						new_ind[j] = -1;
						new_Pdriver[j] = 0;
					}
					if (fabs(Pol[neigh_ind]) > 1) {
						Pol[neigh_ind] = Pdriver[i];
						new_ind[j] = neigh_ind;
						new_Pdriver[j] = Pol[neigh_ind];
					}
					else {
						new_ind[j] = -1;
						new_Pdriver[j] = 0;
					}
				}
				calc_pol(Pol, Ek_matrix, new_Pdriver, new_ind, num_NN, fork, Ek_max, num_cells);
			}
				
		}
		
		free(new_ind); new_ind = NULL;
		free(new_Pdriver); new_Pdriver = NULL;
	}
				

static inline void calc_poli (double *Pol, double **Ek_matrix, double Ek_min, double Ek_max, int num_cells, int *fork) 
	{
		int i, j;
		int num_pos, num_neg, inv;
					
		int *new_ind;
		double *new_Pdriver;
		
		new_ind = (int*)malloc(sizeof(int));
		new_Pdriver = (double*)malloc(sizeof(double));
		
		for (i = 0; i < num_cells; i++) {
			if (fabs(Pol[i]) == 1) {
				num_pos = search_matrix_row(Ek_matrix, Ek_max, i, num_cells);
				num_neg = search_matrix_row(Ek_matrix, Ek_min, i, num_cells);
				if (num_pos + num_neg > 2) {
					continue;
				}
				else {
					if (num_neg > 0) {
						for (j = 0; j < num_neg; j++) {
							inv = find_matrix_row(Ek_matrix, Ek_min, i, num_cells, j);
							if (fabs(Pol[inv]) > 1) {
								Pol[inv] = -Pol[i];
								new_Pdriver[0] = (int)Pol[inv];
								new_ind[0] = inv;
								calc_pol(Pol,Ek_matrix,new_Pdriver,new_ind,1,fork,Ek_max,num_cells);
							}
						}
					}
				}
			}
		}
		free(new_ind); new_ind = NULL;
		free(new_Pdriver); new_Pdriver = NULL;
	}

static inline void prob_pols (double *Pol, double *lss, double **Ek_matrix, double Ek_max, double Ek_min, int num_elements, int number_of_cell_layers, int *number_of_cells_in_layer, QCADCell ***sorted_cells, const coherence_OP *options) 
{
	double *Pol_new;
	double *Pol_old;
	double *Pol_add;
	double *array_diff;
	double lambda = 0.25;
	int done = 1;
	
	int num_pos, num_neg;
	int pos_loc, neg_loc;
	int i,j,k;
	
	double E_sum = 0;
	double prob;
	int cmp;
	
	Pol_new = (double*)malloc(num_elements*sizeof(double));
	Pol_old = (double*)malloc(num_elements*sizeof(double));
	Pol_add = (double*)malloc(num_elements*sizeof(double));
	array_diff = (double*)malloc(num_elements*sizeof(double));
	
	array_copy(Pol,Pol_old,num_elements);
	
	while (done) {
		for(i = 0 ; i < number_of_cell_layers ; i++)
			for(j = 0 ; j < number_of_cells_in_layer[i] ; j++)
			{
				if (((QCAD_CELL_INPUT == sorted_cells[i][j]->cell_function) ||
					 (QCAD_CELL_FIXED == sorted_cells[i][j]->cell_function))) {
					Pol_new[j] = Pol[j];
				}
				else {
					E_sum = 0;
					num_pos = search_matrix_row(Ek_matrix, Ek_max, j, num_elements);
					num_neg = search_matrix_row(Ek_matrix, Ek_min, j, num_elements);
					
					for (k = 0; k < num_pos; k++) {
						pos_loc = find_matrix_row(Ek_matrix,Ek_max,j,num_elements,k);
						if (pos_loc != -1) {
							E_sum += Ek_max*Pol[pos_loc];
						}
					}
					for (k = 0; k < num_neg; k++) {
						neg_loc = find_matrix_row(Ek_matrix,Ek_min,j,num_elements,k);
						if (neg_loc != -1) {
							E_sum += Ek_min*Pol[neg_loc];
						}
					}
					E_sum *= -0.5;
					prob = exp(-E_sum*optimization_options.over_kBT)/(exp(E_sum*optimization_options.over_kBT) + exp(-E_sum*optimization_options.over_kBT));
					Pol_new[j] = 2*prob - 1;
				}
			}
		array_diff = compare_array(Pol,Pol_new,num_elements);
		cmp = search_array_thresh(array_diff, 1e-3, num_elements);
		
		if (cmp == -1) {
			done = 0;
		}
		else {
			mult_array_by_constant(Pol_new,lambda,num_elements);
			mult_array_by_constant(Pol_old,1-lambda,num_elements);
			add_array(Pol_new, Pol_old, num_elements, Pol_add);
			array_copy(Pol_add,Pol_old,num_elements);
		}
	}
	array_copy(Pol_new, lss, num_elements);
	
	free(Pol_new); Pol_new = NULL;
	free(Pol_old); Pol_old = NULL;
	free(Pol_add); Pol_add = NULL;
	free(array_diff); array_diff = NULL;
}
		

static int twopcorr(realtype t, N_Vector y, N_Vector ydot, void *user_data){

	//Optimize here by removing all local variables and just looking at struct?
	Param *par = (Param*)user_data;
	float clock_val = 0;
	register int nd = par->nd;
	float* pd = par->pd;
	float** Ek = par->Ek;
	float** Ekd = par->Ekd;  
	//find a way to not do this more than once
	register int xx = par->xx;
	register int xy = par->xy;
	register int xz = par->xz;
	register int yy = par->yy;
	register int yz = par->yz;
	register int zz = par->zz;
	register int n = par->n;
	int* clock_vals = par->clock_value;
	float *lss = par->lss;
	float tau = par->options->relaxation;
	
	float over_tau = 1/tau;
	
	int numel = n * (3 + 6*(n-1));
	
	int k;
	int i;
	int j;
	
	clock_val = 50+50*tanh(0.03*(300-t));
	
	for(i = 1; i <=  n; i++){
		int p = (numel / n)*(i-1);
		
		//lx
		dldt(1+p) = -driversum(par,i)*l(2+p); 	
		
		for(j = 1; j <= n; j++){
			dldt(1 + p) = dldt(1 + p) + Ek[i-1][j-1]*l(par->K[i-1][j-1] + yz);
		}
		dldt(1+p) = dldt(1+p) - over_tau*l(1+p);
		
		//ly
		dldt(2+p) = (driversum(par,i)*l(1+p) + 2*clock_val*l(3+p));
		for(j = 1; j <= n; j++){
			dldt(2+p) = dldt(2+p) - Ek[i-1][j-1]*l(par->K[i-1][j-1] + xz);
		}
		dldt(2+p) = dldt(2+p) - over_tau*l(2+p);
		
		//lz
		dldt(3+p) = -2*clock_val*l(2+p);	
		dldt(3+p) = dldt(3+p) - over_tau*(l(3+p)+lss[i+1]);
		
		//Ks
		for(j = 1; j <= n; j++){
			if( i != j ){
				
				//Kxz
				dldt(par->K[i-1][j-1] + xz) = (-2*clock_val*l(par->K[i-1][j-1] + xy) - driversum(par,i)*l(par->K[i-1][j-1] + yz) + Ek[i-1][j-1]*l(2+p));
				for(k = 1; k <= n; k++){
					if(k != j && k != i){
						dldt(par->K[i-1][j-1] + xz) += Ek[i-1][k-1]*Kyzz(par,i,j,k,y);
					}
				}
				dldt(par->K[i-1][j-1] + xz) = dldt(par->K[i-1][j-1] + xz) - over_tau*l(par->K[i-1][j-1] + xz);
				
				//Kxy
				dldt(par->K[i-1][j-1] + xy) = (2*clock_val*l(par->K[i-1][j-1] + xz) - driversum(par,i)*l(par->K[i-1][j-1] + yy) + driversum(par,j)*l(par->K[i-1][j-1] + xx));
				for(k = 1; k <= n; k++){
					if(k != j && k != i){
						dldt(par->K[i-1][j-1] + xy) += (Ek[i-1][k-1]*Kyyz(par,i,j,k,y) - Ek[j-1][k-1]*Kxxz(par,i,j,k,y));
					}
				}
				dldt(par->K[i-1][j-1] + xy) = dldt(par->K[i-1][j-1] + xy) - over_tau*l(par->K[i-1][j-1] + xy);
				
				//Kxx
				dldt(par->K[i-1][j-1] + xx) = (-driversum(par,i)*l(par->K[j-1][i-1] + xy) - driversum(par,j)*l(par->K[i-1][j-1] + xy));
				for(k = 1; k<=n;k++){
					if(k != j && k != i){
						dldt(par->K[i-1][j-1] + xx) += (Ek[i-1][k-1]*Kyxz(par,i,j,k,y) + Ek[j-1][k-1]*Kxyz(par,i,j,k,y));
					}
				}
				dldt(par->K[i-1][j-1] + xx) = dldt(par->K[i-1][j-1] + xx) - over_tau*l(par->K[i-1][j-1] + xx);
				
				//Kyy
				dldt(par->K[i-1][j-1] + yy) = (2*clock_val*(l(par->K[j-1][i-1] + yz) + l(par->K[i-1][j-1] + yz)) + driversum(par,i)*l(par->K[i-1][j-1] + xy) + driversum(par,j)*l(par->K[j-1][i-1] + xy));
				for(k = 1; k<=n;k++){
					if(k != j && k != i){
						dldt(par->K[i-1][j-1] + yy) += (-Ek[i-1][k-1]*Kxyz(par,i,j,k,y) - Ek[j-1][k-1]*Kyxz(par,i,j,k,y));
					}
				}
				dldt(par->K[i-1][j-1] + yy) = dldt(par->K[i-1][j-1] + yy) - over_tau*l(par->K[i-1][j-1] + yy);
				
				//Kyz
				dldt(par->K[i-1][j-1] + yz) = (2*clock_val*(l(par->K[i-1][j-1] + zz) - l(par->K[i-1][j-1] + yy)) - Ek[i-1][j-1]*l(p+1) + driversum(par,i)*l(par->K[i-1][j-1] + xz));
				for(k = 1; k <= n; k++){
					if(k != j && k != i){
						dldt(par->K[i-1][j-1] + yz) += (-Ek[i-1][k-1]*Kxzz(par,i,j,k,y));
					}
				}
				dldt(par->K[i-1][j-1] + yz) = dldt(par->K[i-1][j-1] + yz) - over_tau*l(par->K[i-1][j-1] + yz);
				
				//Kzz
				dldt(par->K[i-1][j-1] + zz) += -2*clock_val*(l(par->K[i-1][j-1] + yz) + l(par->K[j-1][i-1] + yz));
				dldt(par->K[i-1][j-1] + zz) = dldt(par->K[i-1][j-1] + zz) - over_tau*(l(par->K[i-1][j-1] + zz) - lss[i+1]*lss[j+1]);
			}
			
		}
	}
	return(0);
}


static inline int search_matrix_row(double **A, double cmp, int row, int num_elements)
{
	
	int i;
	int num_NN = 0;
	
	for (i = 0; i<num_elements; i++) {
		if (fabs(A[row][i] - cmp) < 1e-3*cmp) {
			num_NN += 1;
		}
	}
	return num_NN;	
}

static inline int find_matrix_row(double **A, double cmp, int row, int num_elements, int select)
{
	
	int i;
	int j = 0;
	
	for (i = 0; i<num_elements; i++) {
		if (fabs(A[row][i] - cmp) < 1e-3*cmp) {
			if (j == select) {
				return i;
			}
			else {
				j += 1;
			}
		}
	}
	return -1;
}

static inline int search_array(double *A, double cmp, int num_elements )
{
	
	int i;
	
	for (i = 0; i<num_elements; i++) {
		if (A[i] == cmp) {
			return i;
		}
	}
	return -1;	
}				

static inline int search_array_thresh(double *A, double cmp, int num_elements )
{
	
	int i;
	
	for (i = 0; i<num_elements; i++) {
		if (A[i] > cmp) {
			return i;
		}
	}
	return -1;	
}			
		
static inline int search_array_int(int *A, double cmp, int num_elements )
{
	
	int i;
	
	for (i = 0; i<num_elements; i++) {
		if (A[i] == cmp) {
			return i;
		}
	}
	return -1;	
}	

static inline int find_array(double *A, double cmp, int num_elements)
{
	
	int i;
	int j = 0;
	
	for (i = 0; i<num_elements; i++) {
		if (A[i] == cmp) {
			j += 1;
		}
	}
	return j;	
}	
					
static inline int find_in_array(double *A, double cmp, int num_elements, int select)
{
	
	int i;
	int j = 0;
	
	for (i = 0; i<num_elements; i++) {
		if (A[i] == cmp) {
			if (j == select) {
				return j;
			}
			else {
				j += 1;
			}
		}
	}
	return -1;
}						

static inline double get_max(double **A, int num_elements) {
	
	int i,j;
	double max = -32000;
	
	for (i = 0; i < num_elements; i++) {
		for (j = 0; j < num_elements; j++) {
			if (A[i][j] > max) {
				max = A[i][j];
			}
		}
	}
	return max;
}

static inline double get_min(double **A, int num_elements) {
	
	int i,j;
	double min = 32000;
	
	for (i = 0; i < num_elements; i++) {
		for (j = 0; j < num_elements; j++) {
			if (A[i][j] < min) {
				min = A[i][j];
			}
		}
	}
	return min;
}


inline float driversum(Param* par, int i){
	int j;
	float sum = 0;
	for(j = 1; j <= par->nd; j++){
		sum +=(par->pd[j-1])*(par->Ek[j-1][i-1]);
	}
	
	return sum;
}

inline float Kyzz(Param* par, int i, int j, int k, N_Vector y){
	int p = 3 + 6*(par->n-1);
	return l(par->K[i-1][j-1] + par->yz)*l(p*(k-1) + 3) + l(par->K[i-1][k-1] + par->yz)*l(p*(j-1)+3) 
	+ l(par->K[j-1][k-1] + par->zz)*l(p*(i-1) + 2) - 2*l(p*(i-1) + 2)*l(p*(j-1) + 3)*l(p*(k-1) + 3); 
	
	
}	

inline float Kxxz(Param* par, int i, int j, int k, N_Vector y){
	int p = 3 + 6*(par->n-1);
	return l(par->K[i-1][j-1] + par->xx)*l(p*(k-1) + 3) + l(par->K[j-1][k-1] + par->xz)*l(p*(i-1)+1)
	+ l(par->K[i-1][k-1] + par->xz)*l(p*(j-1) + 1) - 2*l(p*(i-1) + 1)*l(p*(j-1) + 1)*l(p*(k-1) + 3); 
	
	
}

inline float Kxyz(Param* par, int i, int j, int k, N_Vector y){
	int p = 3 + 6*(par->n-1);
	return l(par->K[i-1][j-1] + par->xy)*l(p*(k-1) + 3) + l(par->K[j-1][k-1] + par->yz)*l(p*(i-1)+1)
	+ l(par->K[i-1][k-1] + par->xz)*l(p*(j-1) + 2) - 2*l(p*(i-1) + 1)*l(p*(j-1) + 2)*l(p*(k-1) + 3); 
	
	
}

inline float Kxzz(Param* par, int i, int j, int k, N_Vector y){
	int p = 3 + 6*(par->n-1);
	return l(par->K[i-1][j-1] + par->xz)*l(p*(k-1) + 3) + l(par->K[j-1][k-1] + par->zz)*l(p*(i-1)+1)
	+ l(par->K[i-1][k-1] + par->xz)*l(p*(j-1) + 3) - 2*l(p*(i-1) + 1)*l(p*(j-1) + 3)*l(p*(k-1) + 3); 
	
	
}

inline float Kyxz(Param* par, int i, int j, int k, N_Vector y){
	int p = 3 + 6*(par->n-1);
	return l(par->K[j-1][i-1] + par->xy)*l(p*(k-1) + 3) + l(par->K[j-1][k-1] + par->xz)*l(p*(i-1)+2)
	+ l(par->K[i-1][k-1] + par->yz)*l(p*(j-1) + 1) - 2*l(p*(i-1) + 2)*l(p*(j-1) + 1)*l(p*(k-1) + 3); 
	
	
}

inline float Kyyz(Param* par, int i, int j, int k, N_Vector y){
	int p = 3 + 6*(par->n-1);
	return l(par->K[i-1][j-1] + par->yy)*l(p*(k-1) + 3) + l(par->K[j-1][k-1] + par->yz)*l(p*(i-1)+2)
	+ l(par->K[i-1][k-1] + par->yz)*l(p*(j-1) + 2) - 2*l(p*(i-1) + 2)*l(p*(j-1) + 2)*l(p*(k-1) + 3); 
	
	
}

double *compare_array(double *A, double *B, int length)
{
	int i;
	double *Out = NULL;
	
	Out = (double*)malloc(length*sizeof(double));
	
	for (i = 0; i < length; i++) {
		Out[i] = A[i] - B[i];
	}
	return Out;
}		

static inline void array_copy(double *Arr1, double *Arr0, int length)
{
	
	int i = 0;
	
	for(i = 0; i < length; i++) {
		Arr0[i] = Arr1[i];
	}
	
}

static inline void add_array(double *Arr1, double *Arr2, int length, double *Arr0)
{
	int i = 0;
	for (i = 0; i < length; i++) {
		Arr0[i] = Arr1[i] + Arr2[i];
	}
}

static inline void mult_array_by_constant(double *Arr1, double constant, int length)
{
	int i = 0;
	for (i = 0; i < length; i++) {
		Arr1[i] *= constant;
	}
}

static int check_flag(void *flagvalue, char *funcname, int opt)
{
	int *errflag;
	
	/* Check if SUNDIALS function returned NULL pointer - no memory allocated */
	if (opt == 0 && flagvalue == NULL) {
		fprintf(stderr, "\nSUNDIALS_ERROR: %s() failed - returned NULL pointer\n\n",
				funcname);
    return(1); }
	
	/* Check if flag < 0 */
	else if (opt == 1) {
		errflag = (int *) flagvalue;
		if (*errflag < 0) {
			fprintf(stderr, "\nSUNDIALS_ERROR: %s() failed with flag = %d\n\n",
					funcname, *errflag);
		return(1); }}
	
	/* Check if function returned NULL pointer - no memory allocated */
	else if (opt == 2 && flagvalue == NULL) {
		fprintf(stderr, "\nMEMORY_ERROR: %s() failed - returned NULL pointer\n\n",
				funcname);
    return(1); }
	
	return(0);
}


/*
 * -----------------------------------------------------------------
 * exported functions
 * -----------------------------------------------------------------
 */

/* ----------------------------------------------------------------------------
 * Function to create a new empty serial vector 
 */

N_Vector N_VNewEmpty_Serial(long int length)
{
	N_Vector v;
	N_Vector_Ops ops;
	N_VectorContent_Serial content;
	
	
	/* Create vector */
	v = NULL;
	v = (N_Vector) malloc(sizeof *v);
	if (v == NULL) {return(NULL);}
	
	
	/* Create vector operation structure */
	ops = NULL;
	ops = (N_Vector_Ops) malloc(sizeof(struct _generic_N_Vector_Ops));
	if (ops == NULL) { free(v); return(NULL); }
	
	ops->nvclone           = N_VClone_Serial;
	ops->nvcloneempty      = N_VCloneEmpty_Serial;
	ops->nvdestroy         = N_VDestroy_Serial;
	ops->nvspace           = N_VSpace_Serial;
	ops->nvgetarraypointer = N_VGetArrayPointer_Serial;
	ops->nvsetarraypointer = N_VSetArrayPointer_Serial;
	ops->nvlinearsum       = N_VLinearSum_Serial;
	ops->nvconst           = N_VConst_Serial;
	ops->nvprod            = N_VProd_Serial;
	ops->nvdiv             = N_VDiv_Serial;
	ops->nvscale           = N_VScale_Serial;
	ops->nvabs             = N_VAbs_Serial;
	ops->nvinv             = N_VInv_Serial;
	ops->nvaddconst        = N_VAddConst_Serial;
	ops->nvdotprod         = N_VDotProd_Serial;
	ops->nvmaxnorm         = N_VMaxNorm_Serial;
	ops->nvwrmsnormmask    = N_VWrmsNormMask_Serial;
	ops->nvwrmsnorm        = N_VWrmsNorm_Serial;
	ops->nvmin             = N_VMin_Serial;
	ops->nvwl2norm         = N_VWL2Norm_Serial;
	ops->nvl1norm          = N_VL1Norm_Serial;
	ops->nvcompare         = N_VCompare_Serial;
	ops->nvinvtest         = N_VInvTest_Serial;
	ops->nvconstrmask      = N_VConstrMask_Serial;
	ops->nvminquotient     = N_VMinQuotient_Serial;
	
	/* Create content */
	content = NULL;
	content = (N_VectorContent_Serial) malloc(sizeof(struct _N_VectorContent_Serial));
	if (content == NULL) { free(ops); free(v); return(NULL); }
	
	content->length   = length;
	content->own_data = FALSE;
	content->data     = NULL;
	
	/* Attach content and ops */
	v->content = content;
	v->ops     = ops;
	
	return(v);
}

/* ----------------------------------------------------------------------------
 * Function to create a new serial vector 
 */

N_Vector N_VNew_Serial(long int length)
{
	N_Vector v;
	realtype *data;
	
	v = NULL;
	v = N_VNewEmpty_Serial(length);
	if (v == NULL) return(NULL);
	
	/* Create data */
	if (length > 0) {
		
		/* Allocate memory */
		data = NULL;
		data = (realtype *) malloc(length * sizeof(realtype));
		if(data == NULL) { N_VDestroy_Serial(v); return(NULL); }
		
		/* Attach data */
		( ( (N_VectorContent_Serial)(v->content) )->own_data ) = TRUE;
		(((N_VectorContent_Serial)(v->content))->data)     = data;
		
	}
	
	return(v);
}

/* ----------------------------------------------------------------------------
 * Function to create a serial N_Vector with user data component 
 */

N_Vector N_VMake_Serial(long int length, realtype *v_data)
{
	N_Vector v;
	
	v = NULL;
	v = N_VNewEmpty_Serial(length);
	if (v == NULL) return(NULL);
	
	if (length > 0) {
		/* Attach data */
		( ( (N_VectorContent_Serial)(v->content) )->own_data ) = FALSE;
		(((N_VectorContent_Serial)(v->content))->data)     = v_data;
	}
	
	return(v);
}

/* ----------------------------------------------------------------------------
 * Function to create an array of new serial vectors. 
 */

N_Vector *N_VCloneVectorArray_Serial(int count, N_Vector w)
{
	N_Vector *vs;
	int j;
	
	if (count <= 0) return(NULL);
	
	vs = NULL;
	vs = (N_Vector *) malloc(count * sizeof(N_Vector));
	if(vs == NULL) return(NULL);
	
	for (j = 0; j < count; j++) {
		vs[j] = NULL;
		vs[j] = N_VClone_Serial(w);
		if (vs[j] == NULL) {
			N_VDestroyVectorArray_Serial(vs, j-1);
			return(NULL);
		}
	}
	
	return(vs);
}

/* ----------------------------------------------------------------------------
 * Function to create an array of new serial vectors with NULL data array. 
 */

N_Vector *N_VCloneVectorArrayEmpty_Serial(int count, N_Vector w)
{
	N_Vector *vs;
	int j;
	
	if (count <= 0) return(NULL);
	
	vs = NULL;
	vs = (N_Vector *) malloc(count * sizeof(N_Vector));
	if(vs == NULL) return(NULL);
	
	for (j = 0; j < count; j++) {
		vs[j] = NULL;
		vs[j] = N_VCloneEmpty_Serial(w);
		if (vs[j] == NULL) {
			N_VDestroyVectorArray_Serial(vs, j-1);
			return(NULL);
		}
	}
	
	return(vs);
}

/* ----------------------------------------------------------------------------
 * Function to free an array created with N_VCloneVectorArray_Serial
 */

void N_VDestroyVectorArray_Serial(N_Vector *vs, int count)
{
	int j;
	
	for (j = 0; j < count; j++) N_VDestroy_Serial(vs[j]);
	
	free(vs); vs = NULL;
	
	return;
}

/* ----------------------------------------------------------------------------
 * Function to print the a serial vector 
 */

void N_VPrint_Serial(N_Vector x)
{
	long int i, N;
	realtype *xd;
	
	xd = NULL;
	
	N  = NV_LENGTH_S(x);
	xd = NV_DATA_S(x);
	
	for (i = 0; i < N; i++) {
#if defined(SUNDIALS_EXTENDED_PRECISION)
		printf("%11.8Lg\n", xd[i]);
#elif defined(SUNDIALS_DOUBLE_PRECISION)
		printf("%11.8lg\n", xd[i]);
#else
		printf("%11.8g\n", xd[i]);
#endif
	}
	printf("\n");
	
	return;
}

/*
 * -----------------------------------------------------------------
 * implementation of vector operations
 * -----------------------------------------------------------------
 */

N_Vector N_VCloneEmpty_Serial(N_Vector w)
{
	N_Vector v;
	N_Vector_Ops ops;
	N_VectorContent_Serial content;
	
	if (w == NULL) return(NULL);
	
	/* Create vector */
	v = NULL;
	v = (N_Vector) malloc(sizeof *v);
	if (v == NULL) return(NULL);
	
	/* Create vector operation structure */
	ops = NULL;
	ops = (N_Vector_Ops) malloc(sizeof(struct _generic_N_Vector_Ops));
	if (ops == NULL) { free(v); return(NULL); }
	
	ops->nvclone           = w->ops->nvclone;
	ops->nvcloneempty      = w->ops->nvcloneempty;
	ops->nvdestroy         = w->ops->nvdestroy;
	ops->nvspace           = w->ops->nvspace;
	ops->nvgetarraypointer = w->ops->nvgetarraypointer;
	ops->nvsetarraypointer = w->ops->nvsetarraypointer;
	ops->nvlinearsum       = w->ops->nvlinearsum;
	ops->nvconst           = w->ops->nvconst;  
	ops->nvprod            = w->ops->nvprod;   
	ops->nvdiv             = w->ops->nvdiv;
	ops->nvscale           = w->ops->nvscale; 
	ops->nvabs             = w->ops->nvabs;
	ops->nvinv             = w->ops->nvinv;
	ops->nvaddconst        = w->ops->nvaddconst;
	ops->nvdotprod         = w->ops->nvdotprod;
	ops->nvmaxnorm         = w->ops->nvmaxnorm;
	ops->nvwrmsnormmask    = w->ops->nvwrmsnormmask;
	ops->nvwrmsnorm        = w->ops->nvwrmsnorm;
	ops->nvmin             = w->ops->nvmin;
	ops->nvwl2norm         = w->ops->nvwl2norm;
	ops->nvl1norm          = w->ops->nvl1norm;
	ops->nvcompare         = w->ops->nvcompare;    
	ops->nvinvtest         = w->ops->nvinvtest;
	ops->nvconstrmask      = w->ops->nvconstrmask;
	ops->nvminquotient     = w->ops->nvminquotient;
	
	/* Create content */
	content = NULL;
	content = (N_VectorContent_Serial) malloc(sizeof(struct _N_VectorContent_Serial));
	if (content == NULL) { free(ops); free(v); return(NULL); }
	
	content->length   = NV_LENGTH_S(w);
	content->own_data = FALSE;
	content->data     = NULL;
	
	/* Attach content and ops */
	v->content = content;
	v->ops     = ops;
	
	return(v);
}

N_Vector N_VClone_Serial(N_Vector w)
{
	N_Vector v;
	realtype *data;
	long int length;
	
	v = NULL;
	v = N_VCloneEmpty_Serial(w);
	if (v == NULL) return(NULL);
	
	length = NV_LENGTH_S(w);
	
	/* Create data */
	if (length > 0) {
		
		/* Allocate memory */
		data = NULL;
		data = (realtype *) malloc(length * sizeof(realtype));
		if(data == NULL) { N_VDestroy_Serial(v); return(NULL); }
		
		/* Attach data */
		( ( (N_VectorContent_Serial)(v->content) )->own_data ) = TRUE;
		(((N_VectorContent_Serial)(v->content))->data)     = data;
		
	}
	
	return(v);
}

void N_VDestroy_Serial(N_Vector v)
{
	if (( ( (N_VectorContent_Serial)(v->content) )->own_data ) == TRUE) {
		free((((N_VectorContent_Serial)(v->content))->data));
		(((N_VectorContent_Serial)(v->content))->data) = NULL;
	}
	free(v->content); v->content = NULL;
	free(v->ops); v->ops = NULL;
	free(v); v = NULL;
	
	return;
}

void N_VSpace_Serial(N_Vector v, long int *lrw, long int *liw)
{
	*lrw = NV_LENGTH_S(v);
	*liw = 1;
	
	return;
}

realtype *N_VGetArrayPointer_Serial(N_Vector v)
{
	return((realtype *) (((N_VectorContent_Serial)(v->content))->data));
}

void N_VSetArrayPointer_Serial(realtype *v_data, N_Vector v)
{
	if (NV_LENGTH_S(v) > 0) (((N_VectorContent_Serial)(v->content))->data) = v_data;
	
	return;
}

void N_VLinearSum_Serial(realtype a, N_Vector x, realtype b, N_Vector y, N_Vector z)
{
	long int i, N;
	realtype c, *xd, *yd, *zd;
	N_Vector v1, v2;
	booleantype test;
	
	xd = yd = zd = NULL;
	
	if ((b == ONE) && (z == y)) {    /* BLAS usage: axpy y <- ax+y */
		Vaxpy_Serial(a,x,y);
		return;
	}
	
	if ((a == ONE) && (z == x)) {    /* BLAS usage: axpy x <- by+x */
		Vaxpy_Serial(b,y,x);
		return;
	}
	
	/* Case: a == b == 1.0 */
	
	if ((a == ONE) && (b == ONE)) {
		VSum_Serial(x, y, z);
		return;
	}
	
	/* Cases: (1) a == 1.0, b = -1.0, (2) a == -1.0, b == 1.0 */
	
	if ((test = ((a == ONE) && (b == -ONE))) || ((a == -ONE) && (b == ONE))) {
		v1 = test ? y : x;
		v2 = test ? x : y;
		VDiff_Serial(v2, v1, z);
		return;
	}
	
	/* Cases: (1) a == 1.0, b == other or 0.0, (2) a == other or 0.0, b == 1.0 */
	/* if a or b is 0.0, then user should have called N_VScale */
	
	if ((test = (a == ONE)) || (b == ONE)) {
		c  = test ? b : a;
		v1 = test ? y : x;
		v2 = test ? x : y;
		VLin1_Serial(c, v1, v2, z);
		return;
	}
	
	/* Cases: (1) a == -1.0, b != 1.0, (2) a != 1.0, b == -1.0 */
	
	if ((test = (a == -ONE)) || (b == -ONE)) {
		c = test ? b : a;
		v1 = test ? y : x;
		v2 = test ? x : y;
		VLin2_Serial(c, v1, v2, z);
		return;
	}
	
	/* Case: a == b */
	/* catches case both a and b are 0.0 - user should have called N_VConst */
	
	if (a == b) {
		VScaleSum_Serial(a, x, y, z);
		return;
	}
	
	/* Case: a == -b */
	
	if (a == -b) {
		VScaleDiff_Serial(a, x, y, z);
		return;
	}
	
	/* Do all cases not handled above:
     (1) a == other, b == 0.0 - user should have called N_VScale
     (2) a == 0.0, b == other - user should have called N_VScale
     (3) a,b == other, a !=b, a != -b */
	
	N  = NV_LENGTH_S(x);
	xd = NV_DATA_S(x);
	yd = NV_DATA_S(y);
	zd = NV_DATA_S(z);
	
	for (i = 0; i < N; i++)
		zd[i] = (a*xd[i])+(b*yd[i]);
	
	return;
}

void N_VConst_Serial(realtype c, N_Vector z)
{
	long int i, N;
	realtype *zd;
	
	zd = NULL;
	
	N  = NV_LENGTH_S(z);
	zd = NV_DATA_S(z);
	
	for (i = 0; i < N; i++) zd[i] = c;
	
	return;
}

void N_VProd_Serial(N_Vector x, N_Vector y, N_Vector z)
{
	long int i, N;
	realtype *xd, *yd, *zd;
	
	xd = yd = zd = NULL;
	
	N  = NV_LENGTH_S(x);
	xd = NV_DATA_S(x);
	yd = NV_DATA_S(y);
	zd = NV_DATA_S(z);
	
	for (i = 0; i < N; i++)
		zd[i] = xd[i]*yd[i];
	
	return;
}

void N_VDiv_Serial(N_Vector x, N_Vector y, N_Vector z)
{
	long int i, N;
	realtype *xd, *yd, *zd;
	
	xd = yd = zd = NULL;
	
	N  = NV_LENGTH_S(x);
	xd = NV_DATA_S(x);
	yd = NV_DATA_S(y);
	zd = NV_DATA_S(z);
	
	for (i = 0; i < N; i++)
		zd[i] = xd[i]/yd[i];
	
	return;
}

void N_VScale_Serial(realtype c, N_Vector x, N_Vector z)
{
	long int i, N;
	realtype *xd, *zd;
	
	xd = zd = NULL;
	
	if (z == x) {  /* BLAS usage: scale x <- cx */
		VScaleBy_Serial(c, x);
		return;
	}
	
	if (c == ONE) {
		VCopy_Serial(x, z);
	} else if (c == -ONE) {
		VNeg_Serial(x, z);
	} else {
		N  = NV_LENGTH_S(x);
		xd = NV_DATA_S(x);
		zd = NV_DATA_S(z);
		for (i = 0; i < N; i++) 
			zd[i] = c*xd[i];
	}
	
	return;
}

void N_VAbs_Serial(N_Vector x, N_Vector z)
{
	long int i, N;
	realtype *xd, *zd;
	
	xd = zd = NULL;
	
	N  = NV_LENGTH_S(x);
	xd = NV_DATA_S(x);
	zd = NV_DATA_S(z);
	
	for (i = 0; i < N; i++)
		zd[i] = ABS(xd[i]);
	
	return;
}

void N_VInv_Serial(N_Vector x, N_Vector z)
{
	long int i, N;
	realtype *xd, *zd;
	
	xd = zd = NULL;
	
	N  = NV_LENGTH_S(x);
	xd = NV_DATA_S(x);
	zd = NV_DATA_S(z);
	
	for (i = 0; i < N; i++)
		zd[i] = ONE/xd[i];
	
	return;
}

void N_VAddConst_Serial(N_Vector x, realtype b, N_Vector z)
{
	long int i, N;
	realtype *xd, *zd;
	
	xd = zd = NULL;
	
	N  = NV_LENGTH_S(x);
	xd = NV_DATA_S(x);
	zd = NV_DATA_S(z);
	
	for (i = 0; i < N; i++) 
		zd[i] = xd[i]+b;
	
	return;
}

realtype N_VDotProd_Serial(N_Vector x, N_Vector y)
{
	long int i, N;
	realtype sum, *xd, *yd;
	
	sum = ZERO;
	xd = yd = NULL;
	
	N  = NV_LENGTH_S(x);
	xd = NV_DATA_S(x);
	yd = NV_DATA_S(y);
	
	for (i = 0; i < N; i++)
		sum += xd[i]*yd[i];
	
	return(sum);
}

realtype N_VMaxNorm_Serial(N_Vector x)
{
	long int i, N;
	realtype max, *xd;
	
	max = ZERO;
	xd = NULL;
	
	N  = NV_LENGTH_S(x);
	xd = NV_DATA_S(x);
	
	for (i = 0; i < N; i++) {
		if (ABS(xd[i]) > max) max = ABS(xd[i]);
	}
	
	return(max);
}

realtype N_VWrmsNorm_Serial(N_Vector x, N_Vector w)
{
	long int i, N;
	realtype sum, prodi, *xd, *wd;
	
	sum = ZERO;
	xd = wd = NULL;
	
	N  = NV_LENGTH_S(x);
	xd = NV_DATA_S(x);
	wd = NV_DATA_S(w);
	
	for (i = 0; i < N; i++) {
		prodi = xd[i]*wd[i];
		sum += SQR(prodi);
	}
	
	return(RSqrt(sum/N));
}

realtype N_VWrmsNormMask_Serial(N_Vector x, N_Vector w, N_Vector id)
{
	long int i, N;
	realtype sum, prodi, *xd, *wd, *idd;
	
	sum = ZERO;
	xd = wd = idd = NULL;
	
	N  = NV_LENGTH_S(x);
	xd  = NV_DATA_S(x);
	wd  = NV_DATA_S(w);
	idd = NV_DATA_S(id);
	
	for (i = 0; i < N; i++) {
		if (idd[i] > ZERO) {
			prodi = xd[i]*wd[i];
			sum += SQR(prodi);
		}
	}
	
	return(RSqrt(sum / N));
}

realtype N_VMin_Serial(N_Vector x)
{
	long int i, N;
	realtype min, *xd;
	
	xd = NULL;
	
	N  = NV_LENGTH_S(x);
	xd = NV_DATA_S(x);
	
	min = xd[0];
	
	for (i = 1; i < N; i++) {
		if (xd[i] < min) min = xd[i];
	}
	
	return(min);
}

realtype N_VWL2Norm_Serial(N_Vector x, N_Vector w)
{
	long int i, N;
	realtype sum, prodi, *xd, *wd;
	
	sum = ZERO;
	xd = wd = NULL;
	
	N  = NV_LENGTH_S(x);
	xd = NV_DATA_S(x);
	wd = NV_DATA_S(w);
	
	for (i = 0; i < N; i++) {
		prodi = xd[i]*wd[i];
		sum += SQR(prodi);
	}
	
	return(RSqrt(sum));
}

realtype N_VL1Norm_Serial(N_Vector x)
{
	long int i, N;
	realtype sum, *xd;
	
	sum = ZERO;
	xd = NULL;
	
	N  = NV_LENGTH_S(x);
	xd = NV_DATA_S(x);
	
	for (i = 0; i<N; i++)  
		sum += ABS(xd[i]);
	
	return(sum);
}

void N_VCompare_Serial(realtype c, N_Vector x, N_Vector z)
{
	long int i, N;
	realtype *xd, *zd;
	
	xd = zd = NULL;
	
	N  = NV_LENGTH_S(x);
	xd = NV_DATA_S(x);
	zd = NV_DATA_S(z);
	
	for (i = 0; i < N; i++) {
		zd[i] = (ABS(xd[i]) >= c) ? ONE : ZERO;
	}
	
	return;
}

booleantype N_VInvTest_Serial(N_Vector x, N_Vector z)
{
	long int i, N;
	realtype *xd, *zd;
	
	xd = zd = NULL;
	
	N  = NV_LENGTH_S(x);
	xd = NV_DATA_S(x);
	zd = NV_DATA_S(z);
	
	for (i = 0; i < N; i++) {
		if (xd[i] == ZERO) return(FALSE);
		zd[i] = ONE/xd[i];
	}
	
	return(TRUE);
}

booleantype N_VConstrMask_Serial(N_Vector c, N_Vector x, N_Vector m)
{
	long int i, N;
	booleantype test;
	realtype *cd, *xd, *md;
	
	cd = xd = md = NULL;
	
	N  = NV_LENGTH_S(x);
	xd = NV_DATA_S(x);
	cd = NV_DATA_S(c);
	md = NV_DATA_S(m);
	
	test = TRUE;
	
	for (i = 0; i < N; i++) {
		md[i] = ZERO;
		if (cd[i] == ZERO) continue;
		if (cd[i] > ONEPT5 || cd[i] < -ONEPT5) {
			if ( xd[i]*cd[i] <= ZERO) { test = FALSE; md[i] = ONE; }
			continue;
		}
		if ( cd[i] > HALF || cd[i] < -HALF) {
			if (xd[i]*cd[i] < ZERO ) { test = FALSE; md[i] = ONE; }
		}
	}
	
	return(test);
}

realtype N_VMinQuotient_Serial(N_Vector num, N_Vector denom)
{
	booleantype notEvenOnce;
	long int i, N;
	realtype *nd, *dd, min;
	
	nd = dd = NULL;
	
	N  = NV_LENGTH_S(num);
	nd = NV_DATA_S(num);
	dd = NV_DATA_S(denom);
	
	notEvenOnce = TRUE;
	min = BIG_REAL;
	
	for (i = 0; i < N; i++) {
		if (dd[i] == ZERO) continue;
		else {
			if (!notEvenOnce) min = MIN(min, nd[i]/dd[i]);
			else {
				min = nd[i]/dd[i];
				notEvenOnce = FALSE;
			}
		}
	}
	
	return(min);
}

/*
 * -----------------------------------------------------------------
 * private functions
 * -----------------------------------------------------------------
 */

static void VCopy_Serial(N_Vector x, N_Vector z)
{
	long int i, N;
	realtype *xd, *zd;
	
	xd = zd = NULL;
	
	N  = NV_LENGTH_S(x);
	xd = NV_DATA_S(x);
	zd = NV_DATA_S(z);
	
	for (i = 0; i < N; i++)
		zd[i] = xd[i]; 
	
	return;
}

static void VSum_Serial(N_Vector x, N_Vector y, N_Vector z)
{
	long int i, N;
	realtype *xd, *yd, *zd;
	
	xd = yd = zd = NULL;
	
	N  = NV_LENGTH_S(x);
	xd = NV_DATA_S(x);
	yd = NV_DATA_S(y);
	zd = NV_DATA_S(z);
	
	for (i = 0; i < N; i++)
		zd[i] = xd[i]+yd[i];
	
	return;
}

static void VDiff_Serial(N_Vector x, N_Vector y, N_Vector z)
{
	long int i, N;
	realtype *xd, *yd, *zd;
	
	xd = yd = zd = NULL;
	
	N  = NV_LENGTH_S(x);
	xd = NV_DATA_S(x);
	yd = NV_DATA_S(y);
	zd = NV_DATA_S(z);
	
	for (i = 0; i < N; i++)
		zd[i] = xd[i]-yd[i];
	
	return;
}

static void VNeg_Serial(N_Vector x, N_Vector z)
{
	long int i, N;
	realtype *xd, *zd;
	
	xd = zd = NULL;
	
	N  = NV_LENGTH_S(x);
	xd = NV_DATA_S(x);
	zd = NV_DATA_S(z);
	
	for (i = 0; i < N; i++)
		zd[i] = -xd[i];
	
	return;
}

static void VScaleSum_Serial(realtype c, N_Vector x, N_Vector y, N_Vector z)
{
	long int i, N;
	realtype *xd, *yd, *zd;
	
	xd = yd = zd = NULL;
	
	N  = NV_LENGTH_S(x);
	xd = NV_DATA_S(x);
	yd = NV_DATA_S(y);
	zd = NV_DATA_S(z);
	
	for (i = 0; i < N; i++)
		zd[i] = c*(xd[i]+yd[i]);
	
	return;
}

static void VScaleDiff_Serial(realtype c, N_Vector x, N_Vector y, N_Vector z)
{
	long int i, N;
	realtype *xd, *yd, *zd;
	
	xd = yd = zd = NULL;
	
	N  = NV_LENGTH_S(x);
	xd = NV_DATA_S(x);
	yd = NV_DATA_S(y);
	zd = NV_DATA_S(z);
	
	for (i = 0; i < N; i++)
		zd[i] = c*(xd[i]-yd[i]);
	
	return;
}

static void VLin1_Serial(realtype a, N_Vector x, N_Vector y, N_Vector z)
{
	long int i, N;
	realtype *xd, *yd, *zd;
	
	xd = yd = zd = NULL;
	
	N  = NV_LENGTH_S(x);
	xd = NV_DATA_S(x);
	yd = NV_DATA_S(y);
	zd = NV_DATA_S(z);
	
	for (i = 0; i < N; i++)
		zd[i] = (a*xd[i])+yd[i];
	
	return;
}

static void VLin2_Serial(realtype a, N_Vector x, N_Vector y, N_Vector z)
{
	long int i, N;
	realtype *xd, *yd, *zd;
	
	xd = yd = zd = NULL;
	
	N  = NV_LENGTH_S(x);
	xd = NV_DATA_S(x);
	yd = NV_DATA_S(y);
	zd = NV_DATA_S(z);
	
	for (i = 0; i < N; i++)
		zd[i] = (a*xd[i])-yd[i];
	
	return;
}

static void Vaxpy_Serial(realtype a, N_Vector x, N_Vector y)
{
	long int i, N;
	realtype *xd, *yd;
	
	xd = yd = NULL;
	
	N  = NV_LENGTH_S(x);
	xd = NV_DATA_S(x);
	yd = NV_DATA_S(y);
	
	if (a == ONE) {
		for (i = 0; i < N; i++)
			yd[i] += xd[i];
		return;
	}
	
	if (a == -ONE) {
		for (i = 0; i < N; i++)
			yd[i] -= xd[i];
		return;
	}    
	
	for (i = 0; i < N; i++)
		yd[i] += a*xd[i];
	
	return;
}

static void VScaleBy_Serial(realtype a, N_Vector x)
{
	long int i, N;
	realtype *xd;
	
	xd = NULL;
	
	N  = NV_LENGTH_S(x);
	xd = NV_DATA_S(x);
	
	for (i = 0; i < N; i++)
		xd[i] *= a;
	
	return;
}

realtype RSqrt(realtype x)
{
	if (x <= ZERO) return(ZERO);
	
#if defined(SUNDIALS_USE_GENERIC_MATH)
	return((realtype) sqrt((double) x));
#elif defined(SUNDIALS_DOUBLE_PRECISION)
	return(sqrt(x));
#elif defined(SUNDIALS_SINGLE_PRECISION)
	return(sqrtf(x));
#elif defined(SUNDIALS_EXTENDED_PRECISION)
	return(sqrtl(x));
#endif
}

realtype RPowerI(realtype base, int exponent)
{
	int i, expt;
	realtype prod;
	
	prod = ONE;
	expt = abs(exponent);
	for(i = 1; i <= expt; i++) prod *= base;
	if (exponent < 0) prod = ONE/prod;
	return(prod);
}

realtype RPowerR(realtype base, realtype exponent)
{
	if (base <= ZERO) return(ZERO);
	
#if defined(SUNDIALS_USE_GENERIC_MATH)
	return((realtype) pow((double) base, (double) exponent));
#elif defined(SUNDIALS_DOUBLE_PRECISION)
	return(pow(base, exponent));
#elif defined(SUNDIALS_SINGLE_PRECISION)
	return(powf(base, exponent));
#elif defined(SUNDIALS_EXTENDED_PRECISION)
	return(powl(base, exponent));
#endif
}



/* 
 * =================================================================
 * EXPORTED FUNCTIONS IMPLEMENTATION
 * =================================================================
 */

/* 
 * CVodeCreate
 *
 * CVodeCreate creates an internal memory block for a problem to 
 * be solved by CVODE.
 * If successful, CVodeCreate returns a pointer to the problem memory. 
 * This pointer should be passed to CVodeInit.  
 * If an initialization error occurs, CVodeCreate prints an error 
 * message to standard err and returns NULL. 
 */

void *CVodeCreate(int lmm, int iter)
{
	int maxord;
	CVodeMem cv_mem;
	
	/* Test inputs */
	
	if ((lmm != CV_ADAMS) && (lmm != CV_BDF)) {
		CVProcessError(NULL, 0, "CVODE", "CVodeCreate", MSGCV_BAD_LMM);
		return(NULL);
	}
	
	if ((iter != CV_FUNCTIONAL) && (iter != CV_NEWTON)) {
		CVProcessError(NULL, 0, "CVODE", "CVodeCreate", MSGCV_BAD_ITER);
		return(NULL);
	}
	
	cv_mem = NULL;
	cv_mem = (CVodeMem) malloc(sizeof(struct CVodeMemRec));
	if (cv_mem == NULL) {
		CVProcessError(NULL, 0, "CVODE", "CVodeCreate", MSGCV_CVMEM_FAIL);
		return(NULL);
	}
	
	maxord = (lmm == CV_ADAMS) ? ADAMS_Q_MAX : BDF_Q_MAX;
	
	/* copy input parameters into cv_mem */
	cv_mem->cv_lmm  = lmm;
	cv_mem->cv_iter = iter;
	
	/* Set uround */
	cv_mem->cv_uround = UNIT_ROUNDOFF;
	
	/* Set default values for integrator optional inputs */
	cv_mem->cv_f          = NULL;
	cv_mem->cv_user_data  = NULL;
	cv_mem->cv_itol       = CV_NN;
	cv_mem->cv_user_efun  = FALSE;
	cv_mem->cv_efun       = NULL;
	cv_mem->cv_e_data     = NULL;
	cv_mem->cv_ehfun      = CVErrHandler;
	cv_mem->cv_eh_data    = cv_mem;
	cv_mem->cv_errfp      = stderr;
	cv_mem->cv_qmax       = maxord;
	cv_mem->cv_mxstep     = MXSTEP_DEFAULT;
	cv_mem->cv_mxhnil     = MXHNIL_DEFAULT;
	cv_mem->cv_sldeton    = FALSE;
	cv_mem->cv_hin        = ZERO;
	cv_mem->cv_hmin       = HMIN_DEFAULT;
	cv_mem->cv_hmax_inv   = HMAX_INV_DEFAULT;
	cv_mem->cv_tstopset   = FALSE;
	cv_mem->cv_maxcor     = NLS_MAXCOR;
	cv_mem->cv_maxnef     = MXNEF;
	cv_mem->cv_maxncf     = MXNCF;
	cv_mem->cv_nlscoef    = CORTES;
	
	/* Initialize root finding variables */
	
	cv_mem->cv_glo        = NULL;
	cv_mem->cv_ghi        = NULL;
	cv_mem->cv_grout      = NULL;
	cv_mem->cv_iroots     = NULL;
	cv_mem->cv_rootdir    = NULL;
	cv_mem->cv_gfun       = NULL;
	cv_mem->cv_nrtfn      = 0;
	cv_mem->cv_gactive    = NULL;
	cv_mem->cv_mxgnull    = 1;
	
	/* Set the saved value qmax_alloc */
	
	cv_mem->cv_qmax_alloc = maxord;
	
	/* Initialize lrw and liw */
	
	cv_mem->cv_lrw = 58 + 2*L_MAX + NUM_TESTS;
	cv_mem->cv_liw = 40;
	
	/* No mallocs have been done yet */
	
	cv_mem->cv_VabstolMallocDone = FALSE;
	cv_mem->cv_MallocDone        = FALSE;
	
	/* Return pointer to CVODE memory block */
	
	return((void *)cv_mem);
}

/*-----------------------------------------------------------------*/

#define iter (cv_mem->cv_iter)  
#define lmm  (cv_mem->cv_lmm) 
#define lrw  (cv_mem->cv_lrw)
#define liw  (cv_mem->cv_liw)

/*-----------------------------------------------------------------*/

/*
 * CVodeInit
 * 
 * CVodeInit allocates and initializes memory for a problem. All 
 * problem inputs are checked for errors. If any error occurs during 
 * initialization, it is reported to the file whose file pointer is 
 * errfp and an error flag is returned. Otherwise, it returns CV_SUCCESS
 */

int CVodeInit(void *cvode_mem, CVRhsFn f, realtype t0, N_Vector y0)
{
	CVodeMem cv_mem;
	booleantype nvectorOK, allocOK;
	long int lrw1, liw1;
	int i,k;
	
	/* Check cvode_mem */
	
	if (cvode_mem==NULL) {
		CVProcessError(NULL, CV_MEM_NULL, "CVODE", "CVodeInit", MSGCV_NO_MEM);
		return(CV_MEM_NULL);
	}
	cv_mem = (CVodeMem) cvode_mem;
	
	/* Check for legal input parameters */
	
	if (y0==NULL) {
		CVProcessError(cv_mem, CV_ILL_INPUT, "CVODE", "CVodeInit", MSGCV_NULL_Y0);
		return(CV_ILL_INPUT);
	}
	
	if (f == NULL) {
		CVProcessError(cv_mem, CV_ILL_INPUT, "CVODE", "CVodeInit", MSGCV_NULL_F);
		return(CV_ILL_INPUT);
	}
	
	/* Test if all required vector operations are implemented */
	
	nvectorOK = CVCheckNvector(y0);
	if(!nvectorOK) {
		CVProcessError(cv_mem, CV_ILL_INPUT, "CVODE", "CVodeInit", MSGCV_BAD_NVECTOR);
		return(CV_ILL_INPUT);
	}
	
	/* Set space requirements for one N_Vector */
	
	if (y0->ops->nvspace != NULL) {
		N_VSpace_Serial(y0, &lrw1, &liw1);
	} else {
		lrw1 = 0;
		liw1 = 0;
	}
	cv_mem->cv_lrw1 = lrw1;
	cv_mem->cv_liw1 = liw1;
	
	/* Allocate the vectors (using y0 as a template) */
	
	allocOK = CVAllocVectors(cv_mem, y0);
	if (!allocOK) {
		CVProcessError(cv_mem, CV_MEM_FAIL, "CVODE", "CVodeInit", MSGCV_MEM_FAIL);
		return(CV_MEM_FAIL);
	}
	
	/* All error checking is complete at this point */
	
	/* Copy the input parameters into CVODE state */
	
	cv_mem->cv_f  = f;
	cv_mem->cv_tn = t0;
	
	/* Set step parameters */
	
	cv_mem->cv_q      = 1;
	cv_mem->cv_L      = 2;
	cv_mem->cv_qwait  = cv_mem->cv_L;
	cv_mem->cv_etamax = ETAMX1;
	
	cv_mem->cv_qu    = 0;
	cv_mem->cv_hu    = ZERO;
	cv_mem->cv_tolsf = ONE;
	
	/* Set the linear solver addresses to NULL.
     (We check != NULL later, in CVode, if using CV_NEWTON.) */
	
	cv_mem->cv_linit  = NULL;
	cv_mem->cv_lsetup = NULL;
	cv_mem->cv_lsolve = NULL;
	cv_mem->cv_lfree  = NULL;
	cv_mem->cv_lmem   = NULL;
	
	/* Initialize zn[0] in the history array */
	
	N_VScale(ONE, y0, cv_mem->cv_zn[0]);
	
	/* Initialize all the counters */
	
	cv_mem->cv_nst     = 0;
	cv_mem->cv_nfe     = 0;
	cv_mem->cv_ncfn    = 0;
	cv_mem->cv_netf    = 0;
	cv_mem->cv_nni     = 0;
	cv_mem->cv_nsetups = 0;
	cv_mem->cv_nhnil   = 0;
	cv_mem->cv_nstlp   = 0;
	cv_mem->cv_nscon   = 0;
	cv_mem->cv_nge     = 0;
	
	cv_mem->cv_irfnd   = 0;
	
	/* Initialize other integrator optional outputs */
	
	cv_mem->cv_h0u      = ZERO;
	cv_mem->cv_next_h   = ZERO;
	cv_mem->cv_next_q   = 0;
	
	/* Initialize Stablilty Limit Detection data */
	/* NOTE: We do this even if stab lim det was not
     turned on yet. This way, the user can turn it
     on at any time */
	
	cv_mem->cv_nor = 0;
	for (i = 1; i <= 5; i++)
		for (k = 1; k <= 3; k++) 
			cv_mem->cv_ssdat[i-1][k-1] = ZERO;
	
	/* Problem has been successfully initialized */
	
	cv_mem->cv_MallocDone = TRUE;
	
	return(CV_SUCCESS);
}

/*-----------------------------------------------------------------*/

#define lrw1 (cv_mem->cv_lrw1)
#define liw1 (cv_mem->cv_liw1)

/*-----------------------------------------------------------------*/

/*
 * CVodeReInit
 *
 * CVodeReInit re-initializes CVODE's memory for a problem, assuming
 * it has already been allocated in a prior CVodeInit call.
 * All problem specification inputs are checked for errors.
 * If any error occurs during initialization, it is reported to the
 * file whose file pointer is errfp.
 * The return value is CV_SUCCESS = 0 if no errors occurred, or
 * a negative value otherwise.
 */

int CVodeReInit(void *cvode_mem, realtype t0, N_Vector y0)
{
	CVodeMem cv_mem;
	int i,k;
	
	/* Check cvode_mem */
	
	if (cvode_mem==NULL) {
		CVProcessError(NULL, CV_MEM_NULL, "CVODE", "CVodeReInit", MSGCV_NO_MEM);
		return(CV_MEM_NULL);
	}
	cv_mem = (CVodeMem) cvode_mem;
	
	/* Check if cvode_mem was allocated */
	
	if (cv_mem->cv_MallocDone == FALSE) {
		CVProcessError(cv_mem, CV_NO_MALLOC, "CVODE", "CVodeReInit", MSGCV_NO_MALLOC);
		return(CV_NO_MALLOC);
	}
	
	/* Check for legal input parameters */
	
	if (y0 == NULL) {
		CVProcessError(cv_mem, CV_ILL_INPUT, "CVODE", "CVodeReInit", MSGCV_NULL_Y0);
		return(CV_ILL_INPUT);
	}
	
	/* Copy the input parameters into CVODE state */
	
	cv_mem->cv_tn = t0;
	
	/* Set step parameters */
	
	cv_mem->cv_q      = 1;
	cv_mem->cv_L      = 2;
	cv_mem->cv_qwait  = cv_mem->cv_L;
	cv_mem->cv_etamax = ETAMX1;
	
	cv_mem->cv_qu    = 0;
	cv_mem->cv_hu    = ZERO;
	cv_mem->cv_tolsf = ONE;
	
	/* Initialize zn[0] in the history array */
	
	N_VScale(ONE, y0, cv_mem->cv_zn[0]);
	
	/* Initialize all the counters */
	
	cv_mem->cv_nst     = 0;
	cv_mem->cv_nfe     = 0;
	cv_mem->cv_ncfn    = 0;
	cv_mem->cv_netf    = 0;
	cv_mem->cv_nni     = 0;
	cv_mem->cv_nsetups = 0;
	cv_mem->cv_nhnil   = 0;
	cv_mem->cv_nstlp   = 0;
	cv_mem->cv_nscon   = 0;
	cv_mem->cv_nge     = 0;
	
	cv_mem->cv_irfnd   = 0;
	
	/* Initialize other integrator optional outputs */
	
	cv_mem->cv_h0u      = ZERO;
	cv_mem->cv_next_h   = ZERO;
	cv_mem->cv_next_q   = 0;
	
	/* Initialize Stablilty Limit Detection data */
	
	cv_mem->cv_nor = 0;
	for (i = 1; i <= 5; i++)
		for (k = 1; k <= 3; k++) 
			cv_mem->cv_ssdat[i-1][k-1] = ZERO;
	
	/* Problem has been successfully re-initialized */
	
	return(CV_SUCCESS);
}

/*-----------------------------------------------------------------*/

/*
 * CVodeSStolerances
 * CVodeSVtolerances
 * CVodeWFtolerances
 *
 * These functions specify the integration tolerances. One of them
 * MUST be called before the first call to CVode.
 *
 * CVodeSStolerances specifies scalar relative and absolute tolerances.
 * CVodeSVtolerances specifies scalar relative tolerance and a vector
 *   absolute tolerance (a potentially different absolute tolerance 
 *   for each vector component).
 * CVodeWFtolerances specifies a user-provides function (of type CVEwtFn)
 *   which will be called to set the error weight vector.
 */

int CVodeSStolerances(void *cvode_mem, realtype reltol, realtype abstol)
{
	CVodeMem cv_mem;
	
	if (cvode_mem==NULL) {
		CVProcessError(NULL, CV_MEM_NULL, "CVODE", "CVodeSStolerances", MSGCV_NO_MEM);
		return(CV_MEM_NULL);
	}
	cv_mem = (CVodeMem) cvode_mem;
	
	if (cv_mem->cv_MallocDone == FALSE) {
		CVProcessError(cv_mem, CV_NO_MALLOC, "CVODE", "CVodeSStolerances", MSGCV_NO_MALLOC);
		return(CV_NO_MALLOC);
	}
	
	/* Check inputs */
	
	if (reltol < ZERO) {
		CVProcessError(cv_mem, CV_ILL_INPUT, "CVODE", "CVodeSStolerances", MSGCV_BAD_RELTOL);
		return(CV_ILL_INPUT);
	}
	
	if (abstol < ZERO) {
		CVProcessError(cv_mem, CV_ILL_INPUT, "CVODE", "CVodeSStolerances", MSGCV_BAD_ABSTOL);
		return(CV_ILL_INPUT);
	}
	
	/* Copy tolerances into memory */
	
	cv_mem->cv_reltol = reltol;
	cv_mem->cv_Sabstol = abstol;
	
	cv_mem->cv_itol = CV_SS;
	
	cv_mem->cv_user_efun = FALSE;
	cv_mem->cv_efun = CVEwtSet;
	cv_mem->cv_e_data = NULL; /* will be set to cvode_mem in InitialSetup */
	
	return(CV_SUCCESS);
}


int CVodeSVtolerances(void *cvode_mem, realtype reltol, N_Vector abstol)
{
	CVodeMem cv_mem;
	
	if (cvode_mem==NULL) {
		CVProcessError(NULL, CV_MEM_NULL, "CVODE", "CVodeSVtolerances", MSGCV_NO_MEM);
		return(CV_MEM_NULL);
	}
	cv_mem = (CVodeMem) cvode_mem;
	
	if (cv_mem->cv_MallocDone == FALSE) {
		CVProcessError(cv_mem, CV_NO_MALLOC, "CVODE", "CVodeSVtolerances", MSGCV_NO_MALLOC);
		return(CV_NO_MALLOC);
	}
	
	/* Check inputs */
	
	if (reltol < ZERO) {
		CVProcessError(cv_mem, CV_ILL_INPUT, "CVODE", "CVodeSVtolerances", MSGCV_BAD_RELTOL);
		return(CV_ILL_INPUT);
	}
	
	if (N_VMin(abstol) < ZERO) {
		CVProcessError(cv_mem, CV_ILL_INPUT, "CVODE", "CVodeSVtolerances", MSGCV_BAD_ABSTOL);
		return(CV_ILL_INPUT);
	}
	
	/* Copy tolerances into memory */
	
	if ( !(cv_mem->cv_VabstolMallocDone) ) {
		cv_mem->cv_Vabstol = N_VClone(cv_mem->cv_ewt);
		lrw += lrw1;
		liw += liw1;
		cv_mem->cv_VabstolMallocDone = TRUE;
	}
	
	cv_mem->cv_reltol = reltol;
	N_VScale(ONE, abstol, cv_mem->cv_Vabstol);
	
	cv_mem->cv_itol = CV_SV;
	
	cv_mem->cv_user_efun = FALSE;
	cv_mem->cv_efun = CVEwtSet;
	cv_mem->cv_e_data = NULL; /* will be set to cvode_mem in InitialSetup */
	
	return(CV_SUCCESS);
}


int CVodeWFtolerances(void *cvode_mem, CVEwtFn efun)
{
	CVodeMem cv_mem;
	
	if (cvode_mem==NULL) {
		CVProcessError(NULL, CV_MEM_NULL, "CVODE", "CVodeWFtolerances", MSGCV_NO_MEM);
		return(CV_MEM_NULL);
	}
	cv_mem = (CVodeMem) cvode_mem;
	
	if (cv_mem->cv_MallocDone == FALSE) {
		CVProcessError(cv_mem, CV_NO_MALLOC, "CVODE", "CVodeWFtolerances", MSGCV_NO_MALLOC);
		return(CV_NO_MALLOC);
	}
	
	cv_mem->cv_itol = CV_WF;
	
	cv_mem->cv_user_efun = TRUE;
	cv_mem->cv_efun = efun;
	cv_mem->cv_e_data = NULL; /* will be set to user_data in InitialSetup */
	
	return(CV_SUCCESS);
}

/*-----------------------------------------------------------------*/

#define gfun    (cv_mem->cv_gfun)
#define glo     (cv_mem->cv_glo)
#define ghi     (cv_mem->cv_ghi)
#define grout   (cv_mem->cv_grout)
#define iroots  (cv_mem->cv_iroots)
#define rootdir (cv_mem->cv_rootdir)
#define gactive (cv_mem->cv_gactive)

/*-----------------------------------------------------------------*/

/*
 * CVodeRootInit
 *
 * CVodeRootInit initializes a rootfinding problem to be solved
 * during the integration of the ODE system.  It loads the root
 * function pointer and the number of root functions, and allocates
 * workspace memory.  The return value is CV_SUCCESS = 0 if no errors
 * occurred, or a negative value otherwise.
 */

int CVodeRootInit(void *cvode_mem, int nrtfn, CVRootFn g)
{
	CVodeMem cv_mem;
	int i, nrt;
	
	/* Check cvode_mem pointer */
	if (cvode_mem == NULL) {
		CVProcessError(NULL, CV_MEM_NULL, "CVODE", "CVodeRootInit", MSGCV_NO_MEM);
		return(CV_MEM_NULL);
	}
	cv_mem = (CVodeMem) cvode_mem;
	
	nrt = (nrtfn < 0) ? 0 : nrtfn;
	
	/* If rerunning CVodeRootInit() with a different number of root
     functions (changing number of gfun components), then free
     currently held memory resources */
	if ((nrt != cv_mem->cv_nrtfn) && (cv_mem->cv_nrtfn > 0)) {
		free(glo); glo = NULL;
		free(ghi); ghi = NULL;
		free(grout); grout = NULL;
		free(iroots); iroots = NULL;
		free(rootdir); rootdir = NULL;
		free(gactive); gactive = NULL;
		
		lrw -= 3 * (cv_mem->cv_nrtfn);
		liw -= 3 * (cv_mem->cv_nrtfn);
	}
	
	/* If CVodeRootInit() was called with nrtfn == 0, then set cv_nrtfn to
     zero and cv_gfun to NULL before returning */
	if (nrt == 0) {
		cv_mem->cv_nrtfn = nrt;
		gfun = NULL;
		return(CV_SUCCESS);
	}
	
	/* If rerunning CVodeRootInit() with the same number of root functions
     (not changing number of gfun components), then check if the root
     function argument has changed */
	/* If g != NULL then return as currently reserved memory resources
     will suffice */
	if (nrt == cv_mem->cv_nrtfn) {
		if (g != gfun) {
			if (g == NULL) {
				free(glo); glo = NULL;
				free(ghi); ghi = NULL;
				free(grout); grout = NULL;
				free(iroots); iroots = NULL;
				free(rootdir); rootdir = NULL;
				free(gactive); gactive = NULL;
				
				lrw -= 3*nrt;
				liw -= 3*nrt;
				
				CVProcessError(cv_mem, CV_ILL_INPUT, "CVODE", "CVodeRootInit", MSGCV_NULL_G);
				return(CV_ILL_INPUT);
			}
			else {
				gfun = g;
				return(CV_SUCCESS);
			}
		}
		else return(CV_SUCCESS);
	}
	
	/* Set variable values in CVode memory block */
	cv_mem->cv_nrtfn = nrt;
	if (g == NULL) {
		CVProcessError(cv_mem, CV_ILL_INPUT, "CVODE", "CVodeRootInit", MSGCV_NULL_G);
		return(CV_ILL_INPUT);
	}
	else gfun = g;
	
	/* Allocate necessary memory and return */
	glo = NULL;
	glo = (realtype *) malloc(nrt*sizeof(realtype));
	if (glo == NULL) {
		CVProcessError(cv_mem, CV_MEM_FAIL, "CVODE", "CVodeRootInit", MSGCV_MEM_FAIL);
		return(CV_MEM_FAIL);
	}
	
	ghi = NULL;
	ghi = (realtype *) malloc(nrt*sizeof(realtype));
	if (ghi == NULL) {
		free(glo); glo = NULL;
		CVProcessError(cv_mem, CV_MEM_FAIL, "CVODE", "CVodeRootInit", MSGCV_MEM_FAIL);
		return(CV_MEM_FAIL);
	}
	
	grout = NULL;
	grout = (realtype *) malloc(nrt*sizeof(realtype));
	if (grout == NULL) {
		free(glo); glo = NULL;
		free(ghi); ghi = NULL;
		CVProcessError(cv_mem, CV_MEM_FAIL, "CVODE", "CVodeRootInit", MSGCV_MEM_FAIL);
		return(CV_MEM_FAIL);
	}
	
	iroots = NULL;
	iroots = (int *) malloc(nrt*sizeof(int));
	if (iroots == NULL) {
		free(glo); glo = NULL; 
		free(ghi); ghi = NULL;
		free(grout); grout = NULL;
		CVProcessError(cv_mem, CV_MEM_FAIL, "CVODE", "CVodeRootInit", MSGCV_MEM_FAIL);
		return(CV_MEM_FAIL);
	}
	
	rootdir = NULL;
	rootdir = (int *) malloc(nrt*sizeof(int));
	if (rootdir == NULL) {
		free(glo); glo = NULL; 
		free(ghi); ghi = NULL;
		free(grout); grout = NULL;
		free(iroots); iroots = NULL;
		CVProcessError(cv_mem, CV_MEM_FAIL, "CVODE", "CVodeRootInit", MSGCV_MEM_FAIL);
		return(CV_MEM_FAIL);
	}
	
	gactive = NULL;
	gactive = (booleantype *) malloc(nrt*sizeof(booleantype));
	if (gactive == NULL) {
		free(glo); glo = NULL; 
		free(ghi); ghi = NULL;
		free(grout); grout = NULL;
		free(iroots); iroots = NULL;
		free(rootdir); rootdir = NULL;
		CVProcessError(cv_mem, CV_MEM_FAIL, "CVODES", "CVodeRootInit", MSGCV_MEM_FAIL);
		return(CV_MEM_FAIL);
	}
	
	/* Set default values for rootdir (both directions) */
	for(i=0; i<nrt; i++) rootdir[i] = 0;
	
	/* Set default values for gactive (all active) */
	for(i=0; i<nrt; i++) gactive[i] = TRUE;
	
	lrw += 3*nrt;
	liw += 3*nrt;
	
	return(CV_SUCCESS);
}

/* 
 * =================================================================
 * Readibility Constants
 * =================================================================
 */

#define f              (cv_mem->cv_f)      
#define cv_user_data   (cv_mem->cv_user_data) 
#define efun           (cv_mem->cv_efun)
#define e_data         (cv_mem->cv_e_data) 
#define qmax           (cv_mem->cv_qmax)
#define mxstep         (cv_mem->cv_mxstep)
#define mxhnil         (cv_mem->cv_mxhnil)
#define sldeton        (cv_mem->cv_sldeton)
#define hin            (cv_mem->cv_hin)
#define hmin           (cv_mem->cv_hmin)
#define hmax_inv       (cv_mem->cv_hmax_inv)
#define tstop          (cv_mem->cv_tstop)
#define tstopset       (cv_mem->cv_tstopset)
#define maxnef         (cv_mem->cv_maxnef)
#define maxncf         (cv_mem->cv_maxncf)
#define maxcor         (cv_mem->cv_maxcor)
#define nlscoef        (cv_mem->cv_nlscoef)
#define itol           (cv_mem->cv_itol)         
#define cv_reltol      (cv_mem->cv_reltol)       
#define Sabstol        (cv_mem->cv_Sabstol)
#define Vabstol        (cv_mem->cv_Vabstol)

#define uround         (cv_mem->cv_uround)  
#define zn             (cv_mem->cv_zn) 
#define ewt            (cv_mem->cv_ewt)  
#define cv_y           (cv_mem->cv_y)
#define acor           (cv_mem->cv_acor)
#define tempv          (cv_mem->cv_tempv)
#define ftemp          (cv_mem->cv_ftemp) 
#define cv_q           (cv_mem->cv_q)
#define qprime         (cv_mem->cv_qprime)
#define next_q         (cv_mem->cv_next_q)
#define qwait          (cv_mem->cv_qwait)
#define L              (cv_mem->cv_L)
#define h              (cv_mem->cv_h)
#define hprime         (cv_mem->cv_hprime)
#define next_h         (cv_mem->cv_next_h)
#define eta            (cv_mem->cv_eta) 
#define etaqm1         (cv_mem->cv_etaqm1) 
#define etaq           (cv_mem->cv_etaq) 
#define etaqp1         (cv_mem->cv_etaqp1) 
#define nscon          (cv_mem->cv_nscon)
#define hscale         (cv_mem->cv_hscale)
#define tn             (cv_mem->cv_tn)
#define cv_tau         (cv_mem->cv_tau)
#define tq             (cv_mem->cv_tq)
#define cv_l           (cv_mem->cv_l)
#define rl1            (cv_mem->cv_rl1)
#define cv_gamma       (cv_mem->cv_gamma) 
#define gammap         (cv_mem->cv_gammap) 
#define gamrat         (cv_mem->cv_gamrat)
#define crate          (cv_mem->cv_crate)
#define acnrm          (cv_mem->cv_acnrm)
#define mnewt          (cv_mem->cv_mnewt)
#define etamax         (cv_mem->cv_etamax)
#define nst            (cv_mem->cv_nst)
#define nfe            (cv_mem->cv_nfe)
#define ncfn           (cv_mem->cv_ncfn)
#define netf           (cv_mem->cv_netf)
#define nni            (cv_mem->cv_nni)
#define nsetups        (cv_mem->cv_nsetups)
#define nhnil          (cv_mem->cv_nhnil)
#define linit          (cv_mem->cv_linit)
#define lsetup         (cv_mem->cv_lsetup)
#define lsolve         (cv_mem->cv_lsolve) 
#define lfree          (cv_mem->cv_lfree) 
#define lmem           (cv_mem->cv_lmem) 
#define qu             (cv_mem->cv_qu)          
#define nstlp          (cv_mem->cv_nstlp)  
#define h0u            (cv_mem->cv_h0u)
#define hu             (cv_mem->cv_hu)         
#define saved_tq5      (cv_mem->cv_saved_tq5)  
#define indx_acor      (cv_mem->cv_indx_acor)
#define jcur           (cv_mem->cv_jcur)         
#define tolsf          (cv_mem->cv_tolsf)      
#define setupNonNull   (cv_mem->cv_setupNonNull) 
#define nor            (cv_mem->cv_nor)
#define ssdat          (cv_mem->cv_ssdat)

#define nrtfn          (cv_mem->cv_nrtfn)
#define tlo            (cv_mem->cv_tlo)
#define thi            (cv_mem->cv_thi)
#define tretlast       (cv_mem->cv_tretlast)
#define toutc          (cv_mem->cv_toutc)
#define trout          (cv_mem->cv_trout)
#define ttol           (cv_mem->cv_ttol)
#define taskc          (cv_mem->cv_taskc)
#define irfnd          (cv_mem->cv_irfnd)
#define nge            (cv_mem->cv_nge)


/*-----------------------------------------------------------------*/

/*
 * CVode
 *
 * This routine is the main driver of the CVODE package. 
 *
 * It integrates over a time interval defined by the user, by calling
 * CVStep to do internal time steps.
 *
 * The first time that CVode is called for a successfully initialized
 * problem, it computes a tentative initial step size h.
 *
 * CVode supports two modes, specified by itask: CV_NORMAL, CV_ONE_STEP.
 * In the CV_NORMAL mode, the solver steps until it reaches or passes tout
 * and then interpolates to obtain y(tout).
 * In the CV_ONE_STEP mode, it takes one internal step and returns.
 */

int CVode(void *cvode_mem, realtype tout, N_Vector yout, 
          realtype *tret, int itask)
{
	CVodeMem cv_mem;
	long int nstloc;
	int retval, hflag, kflag, istate, ir, ier, irfndp;
	int ewtsetOK;
	realtype troundoff, tout_hin, rh, nrm;
	booleantype inactive_roots;
	
	/*
	 * -------------------------------------
	 * 1. Check and process inputs
	 * -------------------------------------
	 */
	
	/* Check if cvode_mem exists */
	if (cvode_mem == NULL) {
		CVProcessError(NULL, CV_MEM_NULL, "CVODE", "CVode", MSGCV_NO_MEM);
		return(CV_MEM_NULL);
	}
	cv_mem = (CVodeMem) cvode_mem;
	
	/* Check if cvode_mem was allocated */
	if (cv_mem->cv_MallocDone == FALSE) {
		CVProcessError(cv_mem, CV_NO_MALLOC, "CVODE", "CVode", MSGCV_NO_MALLOC);
		return(CV_NO_MALLOC);
	}
	
	/* Check for yout != NULL */
	if ((cv_y = yout) == NULL) {
		CVProcessError(cv_mem, CV_ILL_INPUT, "CVODE", "CVode", MSGCV_YOUT_NULL);
		return(CV_ILL_INPUT);
	}
	
	/* Check for tret != NULL */
	if (tret == NULL) {
		CVProcessError(cv_mem, CV_ILL_INPUT, "CVODE", "CVode", MSGCV_TRET_NULL);
		return(CV_ILL_INPUT);
	}
	
	/* Check for valid itask */
	if ( (itask != CV_NORMAL) && (itask != CV_ONE_STEP) ) {
		CVProcessError(cv_mem, CV_ILL_INPUT, "CVODE", "CVode", MSGCV_BAD_ITASK);
		return(CV_ILL_INPUT);
	}
	
	if (itask == CV_NORMAL) toutc = tout;
	taskc = itask;
	
	/*
	 * ----------------------------------------
	 * 2. Initializations performed only at
	 *    the first step (nst=0):
	 *    - initial setup
	 *    - initialize Nordsieck history array
	 *    - compute initial step size
	 *    - check for approach to tstop
	 *    - check for approach to a root
	 * ----------------------------------------
	 */
	
	if (nst == 0) {
		
		ier = CVInitialSetup(cv_mem);
		if (ier!= CV_SUCCESS) return(ier);
		
		/* Call f at (t0,y0), set zn[1] = y'(t0), 
		 set initial h (from H0 or CVHin), and scale zn[1] by h.
		 Also check for zeros of root function g at and near t0.    */
		
		retval = f(tn, zn[0], zn[1], cv_user_data); 
		nfe++;
		if (retval < 0) {
			CVProcessError(cv_mem, CV_RHSFUNC_FAIL, "CVODE", "CVode", MSGCV_RHSFUNC_FAILED, tn);
			return(CV_RHSFUNC_FAIL);
		}
		if (retval > 0) {
			CVProcessError(cv_mem, CV_FIRST_RHSFUNC_ERR, "CVODE", "CVode", MSGCV_RHSFUNC_FIRST);
			return(CV_FIRST_RHSFUNC_ERR);
		}
		
		/* Set initial h (from H0 or CVHin). */
		
		h = hin;
		if ( (h != ZERO) && ((tout-tn)*h < ZERO) ) {
			CVProcessError(cv_mem, CV_ILL_INPUT, "CVODE", "CVode", MSGCV_BAD_H0);
			return(CV_ILL_INPUT);
		}
		if (h == ZERO) {
			tout_hin = tout;
			if ( tstopset && (tout-tn)*(tout-tstop) > 0 ) tout_hin = tstop; 
			hflag = CVHin(cv_mem, tout_hin);
			if (hflag != CV_SUCCESS) {
				istate = CVHandleFailure(cv_mem, hflag);
				return(istate);
			}
		}
		rh = ABS(h)*hmax_inv;
		if (rh > ONE) h /= rh;
		if (ABS(h) < hmin) h *= hmin/ABS(h);
		
		/* Check for approach to tstop */
		
		if (tstopset) {
			if ( (tstop - tn)*h < ZERO ) {
				CVProcessError(cv_mem, CV_ILL_INPUT, "CVODE", "CVode", MSGCV_BAD_TSTOP, tstop, tn);
				return(CV_ILL_INPUT);
			}
			if ( (tn + h - tstop)*h > ZERO ) 
				h = (tstop - tn)*(ONE-FOUR*uround);
		}
		
		/* Scale zn[1] by h.*/
		
		hscale = h; 
		h0u    = h;
		hprime = h;
		
		N_VScale(h, zn[1], zn[1]);
		
		/* Check for zeros of root function g at and near t0. */
		
		if (nrtfn > 0) {
			
			retval = CVRcheck1(cv_mem);
			
			if (retval == CV_RTFUNC_FAIL) {
				CVProcessError(cv_mem, CV_RTFUNC_FAIL, "CVODE", "CVRcheck1", MSGCV_RTFUNC_FAILED, tn);
				return(CV_RTFUNC_FAIL);
			}
			
		}
		
	} /* end of first call block */
	
	/*
	 * ------------------------------------------------------
	 * 3. At following steps, perform stop tests:
	 *    - check for root in last step
	 *    - check if we passed tstop
	 *    - check if we passed tout (NORMAL mode)
	 *    - check if current tn was returned (ONE_STEP mode)
	 *    - check if we are close to tstop
	 *      (adjust step size if needed)
	 * -------------------------------------------------------
	 */
	
	if (nst > 0) {
		
		/* Estimate an infinitesimal time interval to be used as
		 a roundoff for time quantities (based on current time 
		 and step size) */
		troundoff = FUZZ_FACTOR*uround*(ABS(tn) + ABS(h));
		
		/* First, check for a root in the last step taken, other than the
		 last root found, if any.  If itask = CV_ONE_STEP and y(tn) was not
		 returned because of an intervening root, return y(tn) now.     */
		if (nrtfn > 0) {
			
			irfndp = irfnd;
			
			retval = CVRcheck2(cv_mem);
			
			if (retval == CLOSERT) {
				CVProcessError(cv_mem, CV_ILL_INPUT, "CVODE", "CVRcheck2", MSGCV_CLOSE_ROOTS, tlo);
				return(CV_ILL_INPUT);
			} else if (retval == CV_RTFUNC_FAIL) {
				CVProcessError(cv_mem, CV_RTFUNC_FAIL, "CVODE", "CVRcheck2", MSGCV_RTFUNC_FAILED, tlo);
				return(CV_RTFUNC_FAIL);
			} else if (retval == RTFOUND) {
				tretlast = *tret = tlo;
				return(CV_ROOT_RETURN);
			}
			
			/* If tn is distinct from tretlast (within roundoff),
			 check remaining interval for roots */
			if ( ABS(tn - tretlast) > troundoff ) {
				
				retval = CVRcheck3(cv_mem);
				
				if (retval == CV_SUCCESS) {     /* no root found */
					irfnd = 0;
					if ((irfndp == 1) && (itask == CV_ONE_STEP)) {
						tretlast = *tret = tn;
						N_VScale(ONE, zn[0], yout);
						return(CV_SUCCESS);
					}
				} else if (retval == RTFOUND) {  /* a new root was found */
					irfnd = 1;
					tretlast = *tret = tlo;
					return(CV_ROOT_RETURN);
				} else if (retval == CV_RTFUNC_FAIL) {  /* g failed */
					CVProcessError(cv_mem, CV_RTFUNC_FAIL, "CVODE", "CVRcheck3", MSGCV_RTFUNC_FAILED, tlo);
					return(CV_RTFUNC_FAIL);
				}
				
			}
			
		} /* end of root stop check */
		
		/* In CV_NORMAL mode, test if tout was reached */
		if ( (itask == CV_NORMAL) && ((tn-tout)*h >= ZERO) ) {
			tretlast = *tret = tout;
			ier =  CVodeGetDky(cv_mem, tout, 0, yout);
			if (ier != CV_SUCCESS) {
				CVProcessError(cv_mem, CV_ILL_INPUT, "CVODE", "CVode", MSGCV_BAD_TOUT, tout);
				return(CV_ILL_INPUT);
			}
			return(CV_SUCCESS);
		}
		
		/* In CV_ONE_STEP mode, test if tn was returned */
		if ( itask == CV_ONE_STEP && ABS(tn - tretlast) > troundoff ) {
			tretlast = *tret = tn;
			N_VScale(ONE, zn[0], yout);
			return(CV_SUCCESS);
		}
		
		/* Test for tn at tstop or near tstop */
		if ( tstopset ) {
			
			if ( ABS(tn - tstop) <= troundoff) {
				ier =  CVodeGetDky(cv_mem, tstop, 0, yout);
				if (ier != CV_SUCCESS) {
					CVProcessError(cv_mem, CV_ILL_INPUT, "CVODE", "CVode", MSGCV_BAD_TSTOP, tstop, tn);
					return(CV_ILL_INPUT);
				}
				tretlast = *tret = tstop;
				tstopset = FALSE;
				return(CV_TSTOP_RETURN);
			}
			
			/* If next step would overtake tstop, adjust stepsize */
			if ( (tn + hprime - tstop)*h > ZERO ) {
				hprime = (tstop - tn)*(ONE-FOUR*uround);
				eta = hprime/h;
			}
			
		}
		
	} /* end stopping tests block */  
	
	/*
	 * --------------------------------------------------
	 * 4. Looping point for internal steps
	 *
	 *    4.1. check for errors (too many steps, too much
	 *         accuracy requested, step size too small)
	 *    4.2. take a new step (call CVStep)
	 *    4.3. stop on error 
	 *    4.4. perform stop tests:
	 *         - check for root in last step
	 *         - check if tout was passed
	 *         - check if close to tstop
	 *         - check if in ONE_STEP mode (must return)
	 * --------------------------------------------------
	 */
	
	nstloc = 0;
	loop {
		
		next_h = h;
		next_q = cv_q;
		
		/* Reset and check ewt */
		if (nst > 0) {
			
			ewtsetOK = efun(zn[0], ewt, e_data);
			
			if (ewtsetOK != 0) {
				
				if (itol == CV_WF) 
					CVProcessError(cv_mem, CV_ILL_INPUT, "CVODE", "CVode", MSGCV_EWT_NOW_FAIL, tn);
				else 
					CVProcessError(cv_mem, CV_ILL_INPUT, "CVODE", "CVode", MSGCV_EWT_NOW_BAD, tn);
				
				istate = CV_ILL_INPUT;
				tretlast = *tret = tn;
				N_VScale(ONE, zn[0], yout);
				break;
				
			}
		}
		
		/* Check for too many steps */
		if ( (mxstep>0) && (nstloc >= mxstep) ) {
			CVProcessError(cv_mem, CV_TOO_MUCH_WORK, "CVODE", "CVode", MSGCV_MAX_STEPS, tn);
			istate = CV_TOO_MUCH_WORK;
			tretlast = *tret = tn;
			N_VScale(ONE, zn[0], yout);
			break;
		}
		
		/* Check for too much accuracy requested */
		nrm = N_VWrmsNorm(zn[0], ewt);
		tolsf = uround * nrm;
		if (tolsf > ONE) {
			CVProcessError(cv_mem, CV_TOO_MUCH_ACC, "CVODE", "CVode", MSGCV_TOO_MUCH_ACC, tn);
			istate = CV_TOO_MUCH_ACC;
			tretlast = *tret = tn;
			N_VScale(ONE, zn[0], yout);
			tolsf *= TWO;
			break;
		} else {
			tolsf = ONE;
		}
		
		/* Check for h below roundoff level in tn */
		if (tn + h == tn) {
			nhnil++;
			if (nhnil <= mxhnil) 
				CVProcessError(cv_mem, CV_WARNING, "CVODE", "CVode", MSGCV_HNIL, tn, h);
			if (nhnil == mxhnil) 
				CVProcessError(cv_mem, CV_WARNING, "CVODE", "CVode", MSGCV_HNIL_DONE);
		}
		
		/* Call CVStep to take a step */
		kflag = CVStep(cv_mem);
		
		/* Process failed step cases, and exit loop */
		if (kflag != CV_SUCCESS) {
			istate = CVHandleFailure(cv_mem, kflag);
			tretlast = *tret = tn;
			N_VScale(ONE, zn[0], yout);
			break;
		}
		
		nstloc++;
		
		/* Check for root in last step taken. */
		if (nrtfn > 0) {
			
			retval = CVRcheck3(cv_mem);
			
			if (retval == RTFOUND) {  /* A new root was found */
				irfnd = 1;
				istate = CV_ROOT_RETURN;
				tretlast = *tret = tlo;
				break;
			} else if (retval == CV_RTFUNC_FAIL) { /* g failed */
				CVProcessError(cv_mem, CV_RTFUNC_FAIL, "CVODE", "CVRcheck3", MSGCV_RTFUNC_FAILED, tlo);
				istate = CV_RTFUNC_FAIL;
				break;
			}
			
			/* If we are at the end of the first step and we still have
			 * some event functions that are inactive, issue a warning
			 * as this may indicate a user error in the implementation
			 * of the root function. */
			
			if (nst==1) {
				inactive_roots = FALSE;
				for (ir=0; ir<nrtfn; ir++) { 
					if (!gactive[ir]) {
						inactive_roots = TRUE;
						break;
					}
				}
				if ((cv_mem->cv_mxgnull > 0) && inactive_roots) {
					CVProcessError(cv_mem, CV_WARNING, "CVODES", "CVode", MSGCV_INACTIVE_ROOTS);
				}
			}
			
		}
		
		/* In NORMAL mode, check if tout reached */
		if ( (itask == CV_NORMAL) &&  (tn-tout)*h >= ZERO ) {
			istate = CV_SUCCESS;
			tretlast = *tret = tout;
			(void) CVodeGetDky(cv_mem, tout, 0, yout);
			next_q = qprime;
			next_h = hprime;
			break;
		}
		
		/* Check if tn is at tstop or near tstop */
		if ( tstopset ) {
			
			troundoff = FUZZ_FACTOR*uround*(ABS(tn) + ABS(h));
			if ( ABS(tn - tstop) <= troundoff) {
				(void) CVodeGetDky(cv_mem, tstop, 0, yout);
				tretlast = *tret = tstop;
				tstopset = FALSE;
				istate = CV_TSTOP_RETURN;
				break;
			}
			
			if ( (tn + hprime - tstop)*h > ZERO ) {
				hprime = (tstop - tn)*(ONE-FOUR*uround);
				eta = hprime/h;
			}
			
		}
		
		/* In ONE_STEP mode, copy y and exit loop */
		if (itask == CV_ONE_STEP) {
			istate = CV_SUCCESS;
			tretlast = *tret = tn;
			N_VScale(ONE, zn[0], yout);
			next_q = qprime;
			next_h = hprime;
			break;
		}
		
	} /* end looping for internal steps */
	
	return(istate);
}

/*-----------------------------------------------------------------*/

/*
 * CVodeGetDky
 *
 * This routine computes the k-th derivative of the interpolating
 * polynomial at the time t and stores the result in the vector dky.
 * The formula is:
 *         q 
 *  dky = SUM c(j,k) * (t - tn)^(j-k) * h^(-j) * zn[j] , 
 *        j=k 
 * where c(j,k) = j*(j-1)*...*(j-k+1), q is the current order, and
 * zn[j] is the j-th column of the Nordsieck history array.
 *
 * This function is called by CVode with k = 0 and t = tout, but
 * may also be called directly by the user.
 */

int CVodeGetDky(void *cvode_mem, realtype t, int k, N_Vector dky)
{
	realtype s, c, r;
	realtype tfuzz, tp, tn1;
	int i, j;
	CVodeMem cv_mem;
	
	/* Check all inputs for legality */
	
	if (cvode_mem == NULL) {
		CVProcessError(NULL, CV_MEM_NULL, "CVODE", "CVodeGetDky", MSGCV_NO_MEM);
		return(CV_MEM_NULL);
	}
	cv_mem = (CVodeMem) cvode_mem;
	
	if (dky == NULL) {
		CVProcessError(cv_mem, CV_BAD_DKY, "CVODE", "CVodeGetDky", MSGCV_NULL_DKY);
		return(CV_BAD_DKY);
	}
	
	if ((k < 0) || (k > cv_q)) {
		CVProcessError(cv_mem, CV_BAD_K, "CVODE", "CVodeGetDky", MSGCV_BAD_K);
		return(CV_BAD_K);
	}
	
	/* Allow for some slack */
	tfuzz = FUZZ_FACTOR * uround * (ABS(tn) + ABS(hu));
	if (hu < ZERO) tfuzz = -tfuzz;
	tp = tn - hu - tfuzz;
	tn1 = tn + tfuzz;
	if ((t-tp)*(t-tn1) > ZERO) {
		CVProcessError(cv_mem, CV_BAD_T, "CVODE", "CVodeGetDky", MSGCV_BAD_T, t, tn-hu, tn);
		return(CV_BAD_T);
	}
	
	/* Sum the differentiated interpolating polynomial */
	
	s = (t - tn) / h;
	for (j=cv_q; j >= k; j--) {
		c = ONE;
		for (i=j; i >= j-k+1; i--) c *= i;
		if (j == cv_q) {
			N_VScale(c, zn[cv_q], dky);
		} else {
			N_VLinearSum_Serial(c, zn[j], s, dky, dky);
		}
	}
	if (k == 0) return(CV_SUCCESS);
	r = RPowerI(h,-k);
	N_VScale(r, dky, dky);
	return(CV_SUCCESS);
}

/*
 * CVodeFree
 *
 * This routine frees the problem memory allocated by CVodeInit.
 * Such memory includes all the vectors allocated by CVAllocVectors,
 * and the memory lmem for the linear solver (deallocated by a call
 * to lfree).
 */

void CVodeFree(void **cvode_mem)
{
	CVodeMem cv_mem;
	
	if (*cvode_mem == NULL) return;
	
	cv_mem = (CVodeMem) (*cvode_mem);
	
	CVFreeVectors(cv_mem);
	
	if (iter == CV_NEWTON && lfree != NULL) lfree(cv_mem);
	
	if (nrtfn > 0) {
		free(glo); glo = NULL;
		free(ghi); ghi = NULL;
		free(grout); grout = NULL;
		free(iroots); iroots = NULL;
		free(rootdir); rootdir = NULL;
		free(gactive); gactive = NULL;
	}
	
	free(*cvode_mem);
	*cvode_mem = NULL;
}

/* 
 * =================================================================
 *  Private Functions Implementation
 * =================================================================
 */

/*
 * CVCheckNvector
 * This routine checks if all required vector operations are present.
 * If any of them is missing it returns FALSE.
 */

static booleantype CVCheckNvector(N_Vector tmpl)
{
	if((tmpl->ops->nvclone     == NULL) ||
	   (tmpl->ops->nvdestroy   == NULL) ||
	   (tmpl->ops->nvlinearsum == NULL) ||
	   (tmpl->ops->nvconst     == NULL) ||
	   (tmpl->ops->nvprod      == NULL) ||
	   (tmpl->ops->nvdiv       == NULL) ||
	   (tmpl->ops->nvscale     == NULL) ||
	   (tmpl->ops->nvabs       == NULL) ||
	   (tmpl->ops->nvinv       == NULL) ||
	   (tmpl->ops->nvaddconst  == NULL) ||
	   (tmpl->ops->nvmaxnorm   == NULL) ||
	   (tmpl->ops->nvwrmsnorm  == NULL) ||
	   (tmpl->ops->nvmin       == NULL))
		return(FALSE);
	else
		return(TRUE);
}

/*
 * CVAllocVectors
 *
 * This routine allocates the CVODE vectors ewt, acor, tempv, ftemp, and
 * zn[0], ..., zn[maxord].
 * If all memory allocations are successful, CVAllocVectors returns TRUE. 
 * Otherwise all allocated memory is freed and CVAllocVectors returns FALSE.
 * This routine also sets the optional outputs lrw and liw, which are
 * (respectively) the lengths of the real and integer work spaces
 * allocated here.
 */

static booleantype CVAllocVectors(CVodeMem cv_mem, N_Vector tmpl)
{
	int i, j;
	
	/* Allocate ewt, acor, tempv, ftemp */
	
	ewt = N_VClone(tmpl);
	if (ewt == NULL) return(FALSE);
	
	acor = N_VClone(tmpl);
	if (acor == NULL) {
		N_VDestroy(ewt);
		return(FALSE);
	}
	
	tempv = N_VClone(tmpl);
	if (tempv == NULL) {
		N_VDestroy(ewt);
		N_VDestroy(acor);
		return(FALSE);
	}
	
	ftemp = N_VClone(tmpl);
	if (ftemp == NULL) {
		N_VDestroy(tempv);
		N_VDestroy(ewt);
		N_VDestroy(acor);
		return(FALSE);
	}
	
	/* Allocate zn[0] ... zn[qmax] */
	
	for (j=0; j <= qmax; j++) {
		zn[j] = N_VClone(tmpl);
		if (zn[j] == NULL) {
			N_VDestroy(ewt);
			N_VDestroy(acor);
			N_VDestroy(tempv);
			N_VDestroy(ftemp);
			for (i=0; i < j; i++) N_VDestroy(zn[i]);
			return(FALSE);
		}
	}
	
	/* Update solver workspace lengths  */
	lrw += (qmax + 5)*lrw1;
	liw += (qmax + 5)*liw1;
	
	/* Store the value of qmax used here */
	cv_mem->cv_qmax_alloc = qmax;
	
	return(TRUE);
}

/*  
 * CVFreeVectors
 *
 * This routine frees the CVODE vectors allocated in CVAllocVectors.
 */

static void CVFreeVectors(CVodeMem cv_mem)
{
	int j, maxord;
	
	maxord = cv_mem->cv_qmax_alloc;
	
	N_VDestroy(ewt);
	N_VDestroy(acor);
	N_VDestroy(tempv);
	N_VDestroy(ftemp);
	for(j=0; j <= maxord; j++) N_VDestroy(zn[j]);
	
	lrw -= (maxord + 5)*lrw1;
	liw -= (maxord + 5)*liw1;
	
	if (cv_mem->cv_VabstolMallocDone) {
		N_VDestroy(Vabstol);
		lrw -= lrw1;
		liw -= liw1;
	}
}

/*  
 * CVInitialSetup
 *
 * This routine performs input consistency checks at the first step.
 * If needed, it also checks the linear solver module and calls the
 * linear solver initialization routine.
 */

static int CVInitialSetup(CVodeMem cv_mem)
{
	int ier;
	
	/* Did the user specify tolerances? */
	if (itol == CV_NN) {
		CVProcessError(cv_mem, CV_ILL_INPUT, "CVODE", "CVInitialSetup", MSGCV_NO_TOLS);
		return(CV_ILL_INPUT);
	}
	
	/* Set data for efun */
	if (cv_mem->cv_user_efun) e_data = cv_user_data;
	else                      e_data = cv_mem;
	
	/* Load initial error weights */
	ier = efun(zn[0], ewt, e_data);
	if (ier != 0) {
		if (itol == CV_WF) 
			CVProcessError(cv_mem, CV_ILL_INPUT, "CVODE", "CVInitialSetup", MSGCV_EWT_FAIL);
		else 
			CVProcessError(cv_mem, CV_ILL_INPUT, "CVODE", "CVInitialSetup", MSGCV_BAD_EWT);
		return(CV_ILL_INPUT);
	}
	
	/* Check if lsolve function exists (if needed) and call linit function (if it exists) */
	if (iter == CV_NEWTON) {
		if (lsolve == NULL) {
			CVProcessError(cv_mem, CV_ILL_INPUT, "CVODE", "CVInitialSetup", MSGCV_LSOLVE_NULL);
			return(CV_ILL_INPUT);
		}
		if (linit != NULL) {
			ier = linit(cv_mem);
			if (ier != 0) {
				CVProcessError(cv_mem, CV_LINIT_FAIL, "CVODE", "CVInitialSetup", MSGCV_LINIT_FAIL);
				return(CV_LINIT_FAIL);
			}
		}
	}
	
	return(CV_SUCCESS);
}

/* 
 * -----------------------------------------------------------------
 * PRIVATE FUNCTIONS FOR CVODE
 * -----------------------------------------------------------------
 */

/*
 * CVHin
 *
 * This routine computes a tentative initial step size h0. 
 * If tout is too close to tn (= t0), then CVHin returns CV_TOO_CLOSE
 * and h remains uninitialized. Note that here tout is either the value
 * passed to CVode at the first call or the value of tstop (if tstop is 
 * enabled and it is closer to t0=tn than tout).
 * If the RHS function fails unrecoverably, CVHin returns CV_RHSFUNC_FAIL.
 * If the RHS function fails recoverably too many times and recovery is
 * not possible, CVHin returns CV_REPTD_RHSFUNC_ERR.
 * Otherwise, CVHin sets h to the chosen value h0 and returns CV_SUCCESS.
 *
 * The algorithm used seeks to find h0 as a solution of
 *       (WRMS norm of (h0^2 ydd / 2)) = 1, 
 * where ydd = estimated second derivative of y.
 *
 * We start with an initial estimate equal to the geometric mean of the
 * lower and upper bounds on the step size.
 *
 * Loop up to MAX_ITERS times to find h0.
 * Stop if new and previous values differ by a factor < 2.
 * Stop if hnew/hg > 2 after one iteration, as this probably means
 * that the ydd value is bad because of cancellation error.        
 *  
 * For each new proposed hg, we allow MAX_ITERS attempts to
 * resolve a possible recoverable failure from f() by reducing
 * the proposed stepsize by a factor of 0.2. If a legal stepsize
 * still cannot be found, fall back on a previous value if possible,
 * or else return CV_REPTD_RHSFUNC_ERR.
 *
 * Finally, we apply a bias (0.5) and verify that h0 is within bounds.
 */

static int CVHin(CVodeMem cv_mem, realtype tout)
{
	int retval, sign, count1, count2;
	realtype tdiff, tdist, tround, hlb, hub;
	realtype hg, hgs, hs, hnew, hrat, h0, yddnrm;
	booleantype hgOK, hnewOK;
	
	/* If tout is too close to tn, give up */
	
	if ((tdiff = tout-tn) == ZERO) return(CV_TOO_CLOSE);
	
	sign = (tdiff > ZERO) ? 1 : -1;
	tdist = ABS(tdiff);
	tround = uround * MAX(ABS(tn), ABS(tout));
	
	if (tdist < TWO*tround) return(CV_TOO_CLOSE);
	
	/* 
     Set lower and upper bounds on h0, and take geometric mean 
     as first trial value.
     Exit with this value if the bounds cross each other.
	 */
	
	hlb = HLB_FACTOR * tround;
	hub = CVUpperBoundH0(cv_mem, tdist);
	
	hg  = RSqrt(hlb*hub);
	
	if (hub < hlb) {
		if (sign == -1) h = -hg;
		else            h =  hg;
		return(CV_SUCCESS);
	}
	
	/* Outer loop */
	
	hnewOK = FALSE;
	hs = hg;         /* safeguard against 'uninitialized variable' warning */
	
	for(count1 = 1; count1 <= MAX_ITERS; count1++) {
		
		/* Attempts to estimate ydd */
		
		hgOK = FALSE;
		
		for (count2 = 1; count2 <= MAX_ITERS; count2++) {
			hgs = hg*sign;
			retval = CVYddNorm(cv_mem, hgs, &yddnrm);
			/* If f() failed unrecoverably, give up */
			if (retval < 0) return(CV_RHSFUNC_FAIL);
			/* If successful, we can use ydd */
			if (retval == CV_SUCCESS) {hgOK = TRUE; break;}
			/* f() failed recoverably; cut step size and test it again */
			hg *= POINT2;
		}
		
		/* If f() failed recoverably MAX_ITERS times */
		
		if (!hgOK) {
			/* Exit if this is the first or second pass. No recovery possible */
			if (count1 <= 2) return(CV_REPTD_RHSFUNC_ERR);
			/* We have a fall-back option. The value hs is a previous hnew which
			 passed through f(). Use it and break */
			hnew = hs;
			break;
		}
		
		/* The proposed step size is feasible. Save it. */
		hs = hg;
		
		/* If the stopping criteria was met, or if this is the last pass, stop */
		if ( (hnewOK) || (count1 == MAX_ITERS))  {hnew = hg; break;}
		
		/* Propose new step size */
		hnew = (yddnrm*hub*hub > TWO) ? RSqrt(TWO/yddnrm) : RSqrt(hg*hub);
		hrat = hnew/hg;
		
		/* Accept hnew if it does not differ from hg by more than a factor of 2 */
		if ((hrat > HALF) && (hrat < TWO)) {
			hnewOK = TRUE;
		}
		
		/* After one pass, if ydd seems to be bad, use fall-back value. */
		if ((count1 > 1) && (hrat > TWO)) {
			hnew = hg;
			hnewOK = TRUE;
		}
		
		/* Send this value back through f() */
		hg = hnew;
		
	}
	
	/* Apply bounds, bias factor, and attach sign */
	
	h0 = H_BIAS*hnew;
	if (h0 < hlb) h0 = hlb;
	if (h0 > hub) h0 = hub;
	if (sign == -1) h0 = -h0;
	h = h0;
	
	return(CV_SUCCESS);
}

/*
 * CVUpperBoundH0
 *
 * This routine sets an upper bound on abs(h0) based on
 * tdist = tn - t0 and the values of y[i]/y'[i].
 */

static realtype CVUpperBoundH0(CVodeMem cv_mem, realtype tdist)
{
	realtype hub_inv, hub;
	N_Vector temp1, temp2;
	
	/* 
	 * Bound based on |y0|/|y0'| -- allow at most an increase of
	 * HUB_FACTOR in y0 (based on a forward Euler step). The weight 
	 * factor is used as a safeguard against zero components in y0. 
	 */
	
	temp1 = tempv;
	temp2 = acor;
	
	N_VAbs(zn[0], temp2);
	efun(zn[0], temp1, e_data);
	N_VInv(temp1, temp1);
	N_VLinearSum_Serial(HUB_FACTOR, temp2, ONE, temp1, temp1);
	
	N_VAbs(zn[1], temp2);
	
	N_VDiv_Serial(temp2, temp1, temp1);
	hub_inv = N_VMaxNorm(temp1);
	
	/*
	 * bound based on tdist -- allow at most a step of magnitude
	 * HUB_FACTOR * tdist
	 */
	
	hub = HUB_FACTOR*tdist;
	
	/* Use the smaler of the two */
	
	if (hub*hub_inv > ONE) hub = ONE/hub_inv;
	
	return(hub);
}

/*
 * CVYddNorm
 *
 * This routine computes an estimate of the second derivative of y
 * using a difference quotient, and returns its WRMS norm.
 */

static int CVYddNorm(CVodeMem cv_mem, realtype hg, realtype *yddnrm)
{
	int retval;
	
	N_VLinearSum_Serial(hg, zn[1], ONE, zn[0], cv_y);
	retval = f(tn+hg, cv_y, tempv, cv_user_data);
	nfe++;
	if (retval < 0) return(CV_RHSFUNC_FAIL);
	if (retval > 0) return(RHSFUNC_RECVR);
	
	N_VLinearSum_Serial(ONE, tempv, -ONE, zn[1], tempv);
	N_VScale(ONE/hg, tempv, tempv);
	
	*yddnrm = N_VWrmsNorm(tempv, ewt);
	
	return(CV_SUCCESS);
}

/* 
 * CVStep
 *
 * This routine performs one internal cvode step, from tn to tn + h.
 * It calls other routines to do all the work.
 *
 * The main operations done here are as follows:
 * - preliminary adjustments if a new step size was chosen;
 * - prediction of the Nordsieck history array zn at tn + h;
 * - setting of multistep method coefficients and test quantities;
 * - solution of the nonlinear system;
 * - testing the local error;
 * - updating zn and other state data if successful;
 * - resetting stepsize and order for the next step.
 * - if SLDET is on, check for stability, reduce order if necessary.
 * On a failure in the nonlinear system solution or error test, the
 * step may be reattempted, depending on the nature of the failure.
 */

static int CVStep(CVodeMem cv_mem)
{
	realtype saved_t, dsm;
	int ncf, nef;
	int nflag, kflag, eflag;
	
	saved_t = tn;
	ncf = nef = 0;
	nflag = FIRST_CALL;
	
	if ((nst > 0) && (hprime != h)) CVAdjustParams(cv_mem);
	
	/* Looping point for attempts to take a step */
	loop {  
		
		CVPredict(cv_mem);  
		CVSet(cv_mem);
		
		nflag = CVNls(cv_mem, nflag);
		kflag = CVHandleNFlag(cv_mem, &nflag, saved_t, &ncf);
		
		/* Go back in loop if we need to predict again (nflag=PREV_CONV_FAIL)*/
		if (kflag == PREDICT_AGAIN) continue;
		
		/* Return if nonlinear solve failed and recovery not possible. */
		if (kflag != DO_ERROR_TEST) return(kflag);
		
		/* Perform error test (nflag=CV_SUCCESS) */
		eflag = CVDoErrorTest(cv_mem, &nflag, saved_t, &nef, &dsm);
		
		/* Go back in loop if we need to predict again (nflag=PREV_ERR_FAIL) */
		if (eflag == TRY_AGAIN)  continue;
		
		/* Return if error test failed and recovery not possible. */
		if (eflag != CV_SUCCESS) return(eflag);
		
		/* Error test passed (eflag=CV_SUCCESS), break from loop */
		break;
		
	}
	
	/* Nonlinear system solve and error test were both successful.
     Update data, and consider change of step and/or order.       */
	
	CVCompleteStep(cv_mem); 
	
	CVPrepareNextStep(cv_mem, dsm); 
	
	/* If Stablilty Limit Detection is turned on, call stability limit
     detection routine for possible order reduction. */
	
	if (sldeton) CVBDFStab(cv_mem);
	
	etamax = (nst <= SMALL_NST) ? ETAMX2 : ETAMX3;
	
	/*  Finally, we rescale the acor array to be the 
	 estimated local error vector. */
	
	N_VScale(tq[2], acor, acor);
	return(CV_SUCCESS);
	
}

/*
 * CVAdjustParams
 *
 * This routine is called when a change in step size was decided upon,
 * and it handles the required adjustments to the history array zn.
 * If there is to be a change in order, we call CVAdjustOrder and reset
 * q, L = q+1, and qwait.  Then in any case, we call CVRescale, which
 * resets h and rescales the Nordsieck array.
 */

static void CVAdjustParams(CVodeMem cv_mem)
{
	if (qprime != cv_q) {
		CVAdjustOrder(cv_mem, qprime-cv_q);
		cv_q = qprime;
		L = cv_q+1;
		qwait = L;
	}
	CVRescale(cv_mem);
}

/*
 * CVAdjustOrder
 *
 * This routine is a high level routine which handles an order
 * change by an amount deltaq (= +1 or -1). If a decrease in order
 * is requested and q==2, then the routine returns immediately.
 * Otherwise CVAdjustAdams or CVAdjustBDF is called to handle the
 * order change (depending on the value of lmm).
 */

static void CVAdjustOrder(CVodeMem cv_mem, int deltaq)
{
	if ((cv_q==2) && (deltaq != 1)) return;
	
	switch(lmm){
		case CV_ADAMS: 
			CVAdjustAdams(cv_mem, deltaq);
			break;
		case CV_BDF:   
			CVAdjustBDF(cv_mem, deltaq);
			break;
	}
}

/*
 * CVAdjustAdams
 *
 * This routine adjusts the history array on a change of order q by
 * deltaq, in the case that lmm == CV_ADAMS.
 */

static void CVAdjustAdams(CVodeMem cv_mem, int deltaq)
{
	int i, j;
	realtype xi, hsum;
	
	/* On an order increase, set new column of zn to zero and return */
	
	if (deltaq==1) {
		N_VConst(ZERO, zn[L]);
		return;
	}
	
	/*
	 * On an order decrease, each zn[j] is adjusted by a multiple of zn[q].
	 * The coeffs. in the adjustment are the coeffs. of the polynomial:
	 *        x
	 * q * INT { u * ( u + xi_1 ) * ... * ( u + xi_{q-2} ) } du 
	 *        0
	 * where xi_j = [t_n - t_(n-j)]/h => xi_0 = 0
	 */
	
	for (i=0; i <= qmax; i++) cv_l[i] = ZERO;
	cv_l[1] = ONE;
	hsum = ZERO;
	for (j=1; j <= cv_q-2; j++) {
		hsum += cv_tau[j];
		xi = hsum / hscale;
		for (i=j+1; i >= 1; i--) cv_l[i] = cv_l[i]*xi + cv_l[i-1];
	}
	
	for (j=1; j <= cv_q-2; j++) cv_l[j+1] = cv_q * (cv_l[j] / (j+1));
	
	for (j=2; j < cv_q; j++)
		N_VLinearSum_Serial(-cv_l[j], zn[cv_q], ONE, zn[j], zn[j]);
}

/*
 * CVAdjustBDF
 *
 * This is a high level routine which handles adjustments to the
 * history array on a change of order by deltaq in the case that 
 * lmm == CV_BDF.  CVAdjustBDF calls CVIncreaseBDF if deltaq = +1 and 
 * CVDecreaseBDF if deltaq = -1 to do the actual work.
 */

static void CVAdjustBDF(CVodeMem cv_mem, int deltaq)
{
	switch(deltaq) {
		case 1 : 
			CVIncreaseBDF(cv_mem);
			return;
		case -1: 
			CVDecreaseBDF(cv_mem);
			return;
	}
}

/*
 * CVIncreaseBDF
 *
 * This routine adjusts the history array on an increase in the 
 * order q in the case that lmm == CV_BDF.  
 * A new column zn[q+1] is set equal to a multiple of the saved 
 * vector (= acor) in zn[indx_acor].  Then each zn[j] is adjusted by
 * a multiple of zn[q+1].  The coefficients in the adjustment are the 
 * coefficients of the polynomial x*x*(x+xi_1)*...*(x+xi_j),
 * where xi_j = [t_n - t_(n-j)]/h.
 */

static void CVIncreaseBDF(CVodeMem cv_mem)
{
	realtype alpha0, alpha1, prod, xi, xiold, hsum, A1;
	int i, j;
	
	for (i=0; i <= qmax; i++) cv_l[i] = ZERO;
	cv_l[2] = alpha1 = prod = xiold = ONE;
	alpha0 = -ONE;
	hsum = hscale;
	if (cv_q > 1) {
		for (j=1; j < cv_q; j++) {
			hsum += cv_tau[j+1];
			xi = hsum / hscale;
			prod *= xi;
			alpha0 -= ONE / (j+1);
			alpha1 += ONE / xi;
			for (i=j+2; i >= 2; i--) cv_l[i] = cv_l[i]*xiold + cv_l[i-1];
			xiold = xi;
		}
	}
	A1 = (-alpha0 - alpha1) / prod;
	N_VScale(A1, zn[indx_acor], zn[L]);
	for (j=2; j <= cv_q; j++) {
		N_VLinearSum_Serial(cv_l[j], zn[L], ONE, zn[j], zn[j]);
	}  
}

/*
 * CVDecreaseBDF
 *
 * This routine adjusts the history array on a decrease in the 
 * order q in the case that lmm == CV_BDF.  
 * Each zn[j] is adjusted by a multiple of zn[q].  The coefficients
 * in the adjustment are the coefficients of the polynomial
 *   x*x*(x+xi_1)*...*(x+xi_j), where xi_j = [t_n - t_(n-j)]/h.
 */

static void CVDecreaseBDF(CVodeMem cv_mem)
{
	realtype hsum, xi;
	int i, j;
	
	for (i=0; i <= qmax; i++) cv_l[i] = ZERO;
	cv_l[2] = ONE;
	hsum = ZERO;
	for(j=1; j <= cv_q-2; j++) {
		hsum += cv_tau[j];
		xi = hsum /hscale;
		for (i=j+2; i >= 2; i--) cv_l[i] = cv_l[i]*xi + cv_l[i-1];
	}
	
	for(j=2; j < cv_q; j++)
		N_VLinearSum_Serial(-cv_l[j], zn[cv_q], ONE, zn[j], zn[j]);
}

/*
 * CVRescale
 *
 * This routine rescales the Nordsieck array by multiplying the
 * jth column zn[j] by eta^j, j = 1, ..., q.  Then the value of
 * h is rescaled by eta, and hscale is reset to h.
 */

static void CVRescale(CVodeMem cv_mem)
{
	int j;
	realtype factor;
	
	factor = eta;
	for (j=1; j <= cv_q; j++) {
		N_VScale(factor, zn[j], zn[j]);
		factor *= eta;
	}
	h = hscale * eta;
	next_h = h;
	hscale = h;
	nscon = 0;
}

/*
 * CVPredict
 *
 * This routine advances tn by the tentative step size h, and computes
 * the predicted array z_n(0), which is overwritten on zn.  The
 * prediction of zn is done by repeated additions.
 * If tstop is enabled, it is possible for tn + h to be past tstop by roundoff,
 * and in that case, we reset tn (after incrementing by h) to tstop.
 */

static void CVPredict(CVodeMem cv_mem)
{
	int j, k;
	
	tn += h;
	if (tstopset) {
		if ((tn - tstop)*h > ZERO) tn = tstop;
	}
	for (k = 1; k <= cv_q; k++)
		for (j = cv_q; j >= k; j--) 
			N_VLinearSum_Serial(ONE, zn[j-1], ONE, zn[j], zn[j-1]); 
}

/*
 * CVSet
 *
 * This routine is a high level routine which calls CVSetAdams or
 * CVSetBDF to set the polynomial l, the test quantity array tq, 
 * and the related variables  rl1, gamma, and gamrat.
 *
 * The array tq is loaded with constants used in the control of estimated
 * local errors and in the nonlinear convergence test.  Specifically, while
 * running at order q, the components of tq are as follows:
 *   tq[1] = a coefficient used to get the est. local error at order q-1
 *   tq[2] = a coefficient used to get the est. local error at order q
 *   tq[3] = a coefficient used to get the est. local error at order q+1
 *   tq[4] = constant used in nonlinear iteration convergence test
 *   tq[5] = coefficient used to get the order q+2 derivative vector used in
 *           the est. local error at order q+1
 */

static void CVSet(CVodeMem cv_mem)
{
	switch(lmm) {
		case CV_ADAMS: 
			CVSetAdams(cv_mem);
			break;
		case CV_BDF:
			CVSetBDF(cv_mem);
			break;
	}
	rl1 = ONE / cv_l[1];
	cv_gamma = h * rl1;
	if (nst == 0) gammap = cv_gamma;
	gamrat = (nst > 0) ? cv_gamma / gammap : ONE;  /* protect x / x != 1.0 */
}

/*
 * CVSetAdams
 *
 * This routine handles the computation of l and tq for the
 * case lmm == CV_ADAMS.
 *
 * The components of the array l are the coefficients of a
 * polynomial Lambda(x) = l_0 + l_1 x + ... + l_q x^q, given by
 *                          q-1
 * (d/dx) Lambda(x) = c * PRODUCT (1 + x / xi_i) , where
 *                          i=1
 *  Lambda(-1) = 0, Lambda(0) = 1, and c is a normalization factor.
 * Here xi_i = [t_n - t_(n-i)] / h.
 *
 * The array tq is set to test quantities used in the convergence
 * test, the error test, and the selection of h at a new order.
 */

static void CVSetAdams(CVodeMem cv_mem)
{
	realtype m[L_MAX], M[3], hsum;
	
	if (cv_q == 1) {
		cv_l[0] = cv_l[1] = tq[1] = tq[5] = ONE;
		tq[2] = HALF;
		tq[3] = ONE/TWELVE;
		tq[4] = nlscoef / tq[2];       /* = 0.1 / tq[2] */
		return;
	}
	
	hsum = CVAdamsStart(cv_mem, m);
	
	M[0] = CVAltSum(cv_q-1, m, 1);
	M[1] = CVAltSum(cv_q-1, m, 2);
	
	CVAdamsFinish(cv_mem, m, M, hsum);
}

/*
 * CVAdamsStart
 *
 * This routine generates in m[] the coefficients of the product
 * polynomial needed for the Adams l and tq coefficients for q > 1.
 */

static realtype CVAdamsStart(CVodeMem cv_mem, realtype m[])
{
	realtype hsum, xi_inv, sum;
	int i, j;
	
	hsum = h;
	m[0] = ONE;
	for (i=1; i <= cv_q; i++) m[i] = ZERO;
	for (j=1; j < cv_q; j++) {
		if ((j==cv_q-1) && (qwait == 1)) {
			sum = CVAltSum(cv_q-2, m, 2);
			tq[1] = cv_q * sum / m[cv_q-2];
		}
		xi_inv = h / hsum;
		for (i=j; i >= 1; i--) m[i] += m[i-1] * xi_inv;
		hsum += cv_tau[j];
		/* The m[i] are coefficients of product(1 to j) (1 + x/xi_i) */
	}
	return(hsum);
}

/*
 * CVAdamsFinish
 *
 * This routine completes the calculation of the Adams l and tq.
 */

static void CVAdamsFinish(CVodeMem cv_mem, realtype m[], realtype M[], realtype hsum)
{
	int i;
	realtype M0_inv, xi, xi_inv;
	
	M0_inv = ONE / M[0];
	
	cv_l[0] = ONE;
	for (i=1; i <= cv_q; i++) cv_l[i] = M0_inv * (m[i-1] / i);
	xi = hsum / h;
	xi_inv = ONE / xi;
	
	tq[2] = M[1] * M0_inv / xi;
	tq[5] = xi / cv_l[cv_q];
	
	if (qwait == 1) {
		for (i=cv_q; i >= 1; i--) m[i] += m[i-1] * xi_inv;
		M[2] = CVAltSum(cv_q, m, 2);
		tq[3] = M[2] * M0_inv / L;
	}
	
	tq[4] = nlscoef / tq[2];
}

/*  
 * CVAltSum
 *
 * CVAltSum returns the value of the alternating sum
 *   sum (i= 0 ... iend) [ (-1)^i * (a[i] / (i + k)) ].
 * If iend < 0 then CVAltSum returns 0.
 * This operation is needed to compute the integral, from -1 to 0,
 * of a polynomial x^(k-1) M(x) given the coefficients of M(x).
 */

static realtype CVAltSum(int iend, realtype a[], int k)
{
	int i, sign;
	realtype sum;
	
	if (iend < 0) return(ZERO);
	
	sum = ZERO;
	sign = 1;
	for (i=0; i <= iend; i++) {
		sum += sign * (a[i] / (i+k));
		sign = -sign;
	}
	return(sum);
}

/*
 * CVSetBDF
 *
 * This routine computes the coefficients l and tq in the case
 * lmm == CV_BDF.  CVSetBDF calls CVSetTqBDF to set the test
 * quantity array tq. 
 * 
 * The components of the array l are the coefficients of a
 * polynomial Lambda(x) = l_0 + l_1 x + ... + l_q x^q, given by
 *                                 q-1
 * Lambda(x) = (1 + x / xi*_q) * PRODUCT (1 + x / xi_i) , where
 *                                 i=1
 *  xi_i = [t_n - t_(n-i)] / h.
 *
 * The array tq is set to test quantities used in the convergence
 * test, the error test, and the selection of h at a new order.
 */

static void CVSetBDF(CVodeMem cv_mem)
{
	realtype alpha0, alpha0_hat, xi_inv, xistar_inv, hsum;
	int i,j;
	
	cv_l[0] = cv_l[1] = xi_inv = xistar_inv = ONE;
	for (i=2; i <= cv_q; i++) cv_l[i] = ZERO;
	alpha0 = alpha0_hat = -ONE;
	hsum = h;
	if (cv_q > 1) {
		for (j=2; j < cv_q; j++) {
			hsum += cv_tau[j-1];
			xi_inv = h / hsum;
			alpha0 -= ONE / j;
			for(i=j; i >= 1; i--) cv_l[i] += cv_l[i-1]*xi_inv;
			/* The cv_l[i] are coefficients of product(1 to j) (1 + x/xi_i) */
		}
		
		/* j = q */
		alpha0 -= ONE / cv_q;
		xistar_inv = -cv_l[1] - alpha0;
		hsum += cv_tau[cv_q-1];
		xi_inv = h / hsum;
		alpha0_hat = -cv_l[1] - xi_inv;
		for (i=cv_q; i >= 1; i--) cv_l[i] += cv_l[i-1]*xistar_inv;
	}
	
	CVSetTqBDF(cv_mem, hsum, alpha0, alpha0_hat, xi_inv, xistar_inv);
}

/*
 * CVSetTqBDF
 *
 * This routine sets the test quantity array tq in the case
 * lmm == CV_BDF.
 */

static void CVSetTqBDF(CVodeMem cv_mem, realtype hsum, realtype alpha0,
                       realtype alpha0_hat, realtype xi_inv, realtype xistar_inv)
{
	realtype A1, A2, A3, A4, A5, A6;
	realtype C, Cpinv, Cppinv;
	
	A1 = ONE - alpha0_hat + alpha0;
	A2 = ONE + cv_q * A1;
	tq[2] = ABS(A1 / (alpha0 * A2));
	tq[5] = ABS(A2 * xistar_inv / (cv_l[cv_q] * xi_inv));
	if (qwait == 1) {
		C = xistar_inv / cv_l[cv_q];
		A3 = alpha0 + ONE / cv_q;
		A4 = alpha0_hat + xi_inv;
		Cpinv = (ONE - A4 + A3) / A3;
		tq[1] = ABS(C * Cpinv);
		hsum += cv_tau[cv_q];
		xi_inv = h / hsum;
		A5 = alpha0 - (ONE / (cv_q+1));
		A6 = alpha0_hat - xi_inv;
		Cppinv = (ONE - A6 + A5) / A2;
		tq[3] = ABS(Cppinv / (xi_inv * (cv_q+2) * A5));
	}
	tq[4] = nlscoef / tq[2];
}

/*
 * CVNls
 *
 * This routine attempts to solve the nonlinear system associated
 * with a single implicit step of the linear multistep method.
 * Depending on iter, it calls CVNlsFunctional or CVNlsNewton
 * to do the work.
 */

static int CVNls(CVodeMem cv_mem, int nflag)
{
	int flag = CV_SUCCESS;
	
	switch(iter) {
		case CV_FUNCTIONAL: 
			flag = CVNlsFunctional(cv_mem);
			break;
		case CV_NEWTON:
			flag = CVNlsNewton(cv_mem, nflag);
			break;
	}
	
	return(flag);
}

/*
 * CVNlsFunctional
 *
 * This routine attempts to solve the nonlinear system using 
 * functional iteration (no matrices involved).
 *
 * Possible return values are:
 *
 *   CV_SUCCESS      --->  continue with error test
 *
 *   CV_RHSFUNC_FAIL --->  halt the integration
 *
 *   CONV_FAIL       -+
 *   RHSFUNC_RECVR   -+->  predict again or stop if too many
 *
 */

static int CVNlsFunctional(CVodeMem cv_mem)
{
	int retval, m;
	realtype del, delp, dcon;
	
	/* Initialize counter and evaluate f at predicted y */
	
	crate = ONE;
	m = 0;
	
	retval = f(tn, zn[0], tempv, cv_user_data);
	nfe++;
	if (retval < 0) return(CV_RHSFUNC_FAIL);
	if (retval > 0) return(RHSFUNC_RECVR);
	
	N_VConst(ZERO, acor);
	
	/* Initialize delp to avoid compiler warning message */
	del = delp = ZERO;
	
	/* Loop until convergence; accumulate corrections in acor */
	
	loop {
		
		nni++;
		
		/* Correct y directly from the last f value */
		N_VLinearSum_Serial(h, tempv, -ONE, zn[1], tempv);
		N_VScale(rl1, tempv, tempv);
		N_VLinearSum_Serial(ONE, zn[0], ONE, tempv, cv_y);
		/* Get WRMS norm of current correction to use in convergence test */
		N_VLinearSum_Serial(ONE, tempv, -ONE, acor, acor);
		del = N_VWrmsNorm(acor, ewt);
		N_VScale(ONE, tempv, acor);
		
		/* Test for convergence.  If m > 0, an estimate of the convergence
		 rate constant is stored in crate, and used in the test.        */
		if (m > 0) crate = MAX(CRDOWN * crate, del / delp);
		dcon = del * MIN(ONE, crate) / tq[4];
		if (dcon <= ONE) {
			acnrm = (m == 0) ? del : N_VWrmsNorm(acor, ewt);
			return(CV_SUCCESS);  /* Convergence achieved */
		}
		
		/* Stop at maxcor iterations or if iter. seems to be diverging */
		m++;
		if ((m==maxcor) || ((m >= 2) && (del > RDIV * delp))) return(CONV_FAIL);
		
		/* Save norm of correction, evaluate f, and loop again */
		delp = del;
		
		retval = f(tn, cv_y, tempv, cv_user_data);
		nfe++;
		if (retval < 0) return(CV_RHSFUNC_FAIL);
		if (retval > 0) return(RHSFUNC_RECVR);
		
	}
}

/*
 * CVNlsNewton
 *
 * This routine handles the Newton iteration. It calls lsetup if 
 * indicated, calls CVNewtonIteration to perform the iteration, and 
 * retries a failed attempt at Newton iteration if that is indicated.
 *
 * Possible return values:
 *
 *   CV_SUCCESS       ---> continue with error test
 *
 *   CV_RHSFUNC_FAIL  -+  
 *   CV_LSETUP_FAIL    |-> halt the integration 
 *   CV_LSOLVE_FAIL   -+
 *
 *   CONV_FAIL        -+
 *   RHSFUNC_RECVR    -+-> predict again or stop if too many
 *
 */

static int CVNlsNewton(CVodeMem cv_mem, int nflag)
{
	N_Vector vtemp1, vtemp2, vtemp3;
	int convfail, retval, ier;
	booleantype callSetup;
	
	vtemp1 = acor;  /* rename acor as vtemp1 for readability  */
	vtemp2 = cv_y;     /* rename y as vtemp2 for readability     */
	vtemp3 = tempv; /* rename tempv as vtemp3 for readability */
	
	/* Set flag convfail, input to lsetup for its evaluation decision */
	convfail = ((nflag == FIRST_CALL) || (nflag == PREV_ERR_FAIL)) ?
    CV_NO_FAILURES : CV_FAIL_OTHER;
	
	/* Decide whether or not to call setup routine (if one exists) */
	if (setupNonNull) {      
		callSetup = (nflag == PREV_CONV_FAIL) || (nflag == PREV_ERR_FAIL) ||
		(nst == 0) || (nst >= nstlp + MSBP) || (ABS(gamrat-ONE) > DGMAX);
	} else {  
		crate = ONE;
		callSetup = FALSE;
	}
	
	/* Looping point for the solution of the nonlinear system.
     Evaluate f at the predicted y, call lsetup if indicated, and
     call CVNewtonIteration for the Newton iteration itself.      */
	
	loop {
		
		retval = f(tn, zn[0], ftemp, cv_user_data);
		nfe++; 
		if (retval < 0) return(CV_RHSFUNC_FAIL);
		if (retval > 0) return(RHSFUNC_RECVR);
		
		if (callSetup) {
			ier = lsetup(cv_mem, convfail, zn[0], ftemp, &jcur, 
						 vtemp1, vtemp2, vtemp3);
			nsetups++;
			callSetup = FALSE;
			gamrat = crate = ONE; 
			gammap = cv_gamma;
			nstlp = nst;
			/* Return if lsetup failed */
			if (ier < 0) return(CV_LSETUP_FAIL);
			if (ier > 0) return(CONV_FAIL);
		}
		
		/* Set acor to zero and load prediction into y vector */
		N_VConst(ZERO, acor);
		N_VScale(ONE, zn[0], cv_y);
		
		/* Do the Newton iteration */
		ier = CVNewtonIteration(cv_mem);
		
		/* If there is a convergence failure and the Jacobian-related 
		 data appears not to be current, loop again with a call to lsetup
		 in which convfail=CV_FAIL_BAD_J.  Otherwise return.                 */
		if (ier != TRY_AGAIN) return(ier);
		
		callSetup = TRUE;
		convfail = CV_FAIL_BAD_J;
	}
}

/*
 * CVNewtonIteration
 *
 * This routine performs the Newton iteration. If the iteration succeeds,
 * it returns the value CV_SUCCESS. If not, it may signal the CVNlsNewton 
 * routine to call lsetup again and reattempt the iteration, by
 * returning the value TRY_AGAIN. (In this case, CVNlsNewton must set 
 * convfail to CV_FAIL_BAD_J before calling setup again). 
 * Otherwise, this routine returns one of the appropriate values 
 * CV_LSOLVE_FAIL, CV_RHSFUNC_FAIL, CONV_FAIL, or RHSFUNC_RECVR back 
 * to CVNlsNewton.
 */

static int CVNewtonIteration(CVodeMem cv_mem)
{
	int m, retval;
	realtype del, delp, dcon;
	N_Vector b;
	
	mnewt = m = 0;
	
	/* Initialize delp to avoid compiler warning message */
	del = delp = ZERO;
	
	/* Looping point for Newton iteration */
	loop {
		
		/* Evaluate the residual of the nonlinear system*/
		N_VLinearSum_Serial(rl1, zn[1], ONE, acor, tempv);
		N_VLinearSum_Serial(cv_gamma, ftemp, -ONE, tempv, tempv);
		
		/* Call the lsolve function */
		b = tempv;
		retval = lsolve(cv_mem, b, ewt, cv_y, ftemp); 
		nni++;
		
		if (retval < 0) return(CV_LSOLVE_FAIL);
		
		/* If lsolve had a recoverable failure and Jacobian data is
		 not current, signal to try the solution again            */
		if (retval > 0) { 
			if ((!jcur) && (setupNonNull)) return(TRY_AGAIN);
			else                           return(CONV_FAIL);
		}
		
		/* Get WRMS norm of correction; add correction to acor and y */
		del = N_VWrmsNorm(b, ewt);
		N_VLinearSum_Serial(ONE, acor, ONE, b, acor);
		N_VLinearSum_Serial(ONE, zn[0], ONE, acor, cv_y);
		
		/* Test for convergence.  If m > 0, an estimate of the convergence
		 rate constant is stored in crate, and used in the test.        */
		if (m > 0) {
			crate = MAX(CRDOWN * crate, del/delp);
		}
		dcon = del * MIN(ONE, crate) / tq[4];
		
		if (dcon <= ONE) {
			acnrm = (m==0) ? del : N_VWrmsNorm(acor, ewt);
			jcur = FALSE;
			return(CV_SUCCESS); /* Nonlinear system was solved successfully */
		}
		
		mnewt = ++m;
		
		/* Stop at maxcor iterations or if iter. seems to be diverging.
		 If still not converged and Jacobian data is not current, 
		 signal to try the solution again                            */
		if ((m == maxcor) || ((m >= 2) && (del > RDIV*delp))) {
			if ((!jcur) && (setupNonNull)) return(TRY_AGAIN);
			else                           return(CONV_FAIL);
		}
		
		/* Save norm of correction, evaluate f, and loop again */
		delp = del;
		retval = f(tn, cv_y, ftemp, cv_user_data);
		nfe++;
		if (retval < 0) return(CV_RHSFUNC_FAIL);
		if (retval > 0) {
			if ((!jcur) && (setupNonNull)) return(TRY_AGAIN);
			else                           return(RHSFUNC_RECVR);
		}
		
	} /* end loop */
}

/*
 * CVHandleFlag
 *
 * This routine takes action on the return value nflag = *nflagPtr
 * returned by CVNls, as follows:
 *
 * If CVNls succeeded in solving the nonlinear system, then
 * CVHandleNFlag returns the constant DO_ERROR_TEST, which tells CVStep
 * to perform the error test.
 *
 * If the nonlinear system was not solved successfully, then ncfn and
 * ncf = *ncfPtr are incremented and Nordsieck array zn is restored.
 *
 * If the solution of the nonlinear system failed due to an
 * unrecoverable failure by setup, we return the value CV_LSETUP_FAIL.
 * 
 * If it failed due to an unrecoverable failure in solve, then we return
 * the value CV_LSOLVE_FAIL.
 *
 * If it failed due to an unrecoverable failure in rhs, then we return
 * the value CV_RHSFUNC_FAIL.
 *
 * Otherwise, a recoverable failure occurred when solving the 
 * nonlinear system (CVNls returned nflag == CONV_FAIL or RHSFUNC_RECVR). 
 * In this case, if ncf is now equal to maxncf or |h| = hmin, 
 * we return the value CV_CONV_FAILURE (if nflag=CONV_FAIL) or
 * CV_REPTD_RHSFUNC_ERR (if nflag=RHSFUNC_RECVR).
 * If not, we set *nflagPtr = PREV_CONV_FAIL and return the value
 * PREDICT_AGAIN, telling CVStep to reattempt the step.
 *
 */

static int CVHandleNFlag(CVodeMem cv_mem, int *nflagPtr, realtype saved_t,
                         int *ncfPtr)
{
	int nflag;
	
	nflag = *nflagPtr;
	
	if (nflag == CV_SUCCESS) return(DO_ERROR_TEST);
	
	/* The nonlinear soln. failed; increment ncfn and restore zn */
	ncfn++;
	CVRestore(cv_mem, saved_t);
	
	/* Return if lsetup, lsolve, or rhs failed unrecoverably */
	if (nflag == CV_LSETUP_FAIL)  return(CV_LSETUP_FAIL);
	if (nflag == CV_LSOLVE_FAIL)  return(CV_LSOLVE_FAIL);
	if (nflag == CV_RHSFUNC_FAIL) return(CV_RHSFUNC_FAIL);
	
	/* At this point, nflag = CONV_FAIL or RHSFUNC_RECVR; increment ncf */
	
	(*ncfPtr)++;
	etamax = ONE;
	
	/* If we had maxncf failures or |h| = hmin, 
     return CV_CONV_FAILURE or CV_REPTD_RHSFUNC_ERR. */
	
	if ((ABS(h) <= hmin*ONEPSM) || (*ncfPtr == maxncf)) {
		if (nflag == CONV_FAIL)     return(CV_CONV_FAILURE);
		if (nflag == RHSFUNC_RECVR) return(CV_REPTD_RHSFUNC_ERR);    
	}
	
	/* Reduce step size; return to reattempt the step */
	
	eta = MAX(ETACF, hmin / ABS(h));
	*nflagPtr = PREV_CONV_FAIL;
	CVRescale(cv_mem);
	
	return(PREDICT_AGAIN);
}

/*
 * CVRestore
 *
 * This routine restores the value of tn to saved_t and undoes the
 * prediction.  After execution of CVRestore, the Nordsieck array zn has
 * the same values as before the call to CVPredict.
 */

static void CVRestore(CVodeMem cv_mem, realtype saved_t)
{
	int j, k;
	
	tn = saved_t;
	for (k = 1; k <= cv_q; k++)
		for (j = cv_q; j >= k; j--)
			N_VLinearSum_Serial(ONE, zn[j-1], -ONE, zn[j], zn[j-1]);
}

/*
 * CVDoErrorTest
 *
 * This routine performs the local error test. 
 * The weighted local error norm dsm is loaded into *dsmPtr, and 
 * the test dsm ?<= 1 is made.
 *
 * If the test passes, CVDoErrorTest returns CV_SUCCESS. 
 *
 * If the test fails, we undo the step just taken (call CVRestore) and 
 *
 *   - if maxnef error test failures have occurred or if ABS(h) = hmin,
 *     we return CV_ERR_FAILURE.
 *
 *   - if more than MXNEF1 error test failures have occurred, an order
 *     reduction is forced. If already at order 1, restart by reloading 
 *     zn from scratch. If f() fails we return either CV_RHSFUNC_FAIL
 *     or CV_UNREC_RHSFUNC_ERR (no recovery is possible at this stage).
 *
 *   - otherwise, set *nflagPtr to PREV_ERR_FAIL, and return TRY_AGAIN. 
 *
 */

static booleantype CVDoErrorTest(CVodeMem cv_mem, int *nflagPtr,
								 realtype saved_t, int *nefPtr, realtype *dsmPtr)
{
	realtype dsm;
	int retval;
	
	dsm = acnrm * tq[2];
	
	/* If est. local error norm dsm passes test, return CV_SUCCESS */  
	*dsmPtr = dsm; 
	if (dsm <= ONE) return(CV_SUCCESS);
	
	/* Test failed; increment counters, set nflag, and restore zn array */
	(*nefPtr)++;
	netf++;
	*nflagPtr = PREV_ERR_FAIL;
	CVRestore(cv_mem, saved_t);
	
	/* At maxnef failures or |h| = hmin, return CV_ERR_FAILURE */
	if ((ABS(h) <= hmin*ONEPSM) || (*nefPtr == maxnef)) return(CV_ERR_FAILURE);
	
	/* Set etamax = 1 to prevent step size increase at end of this step */
	etamax = ONE;
	
	/* Set h ratio eta from dsm, rescale, and return for retry of step */
	if (*nefPtr <= MXNEF1) {
		eta = ONE / (RPowerR(BIAS2*dsm,ONE/L) + ADDON);
		eta = MAX(ETAMIN, MAX(eta, hmin / ABS(h)));
		if (*nefPtr >= SMALL_NEF) eta = MIN(eta, ETAMXF);
		CVRescale(cv_mem);
		return(TRY_AGAIN);
	}
	
	/* After MXNEF1 failures, force an order reduction and retry step */
	if (cv_q > 1) {
		eta = MAX(ETAMIN, hmin / ABS(h));
		CVAdjustOrder(cv_mem,-1);
		L = cv_q;
		cv_q--;
		qwait = L;
		CVRescale(cv_mem);
		return(TRY_AGAIN);
	}
	
	/* If already at order 1, restart: reload zn from scratch */
	
	eta = MAX(ETAMIN, hmin / ABS(h));
	h *= eta;
	next_h = h;
	hscale = h;
	qwait = LONG_WAIT;
	nscon = 0;
	
	retval = f(tn, zn[0], tempv, cv_user_data);
	nfe++;
	if (retval < 0)  return(CV_RHSFUNC_FAIL);
	if (retval > 0)  return(CV_UNREC_RHSFUNC_ERR);
	
	N_VScale(h, tempv, zn[1]);
	
	return(TRY_AGAIN);
}

/* 
 * =================================================================
 *  Private Functions Implementation after succesful step
 * =================================================================
 */

/*
 * CVCompleteStep
 *
 * This routine performs various update operations when the solution
 * to the nonlinear system has passed the local error test. 
 * We increment the step counter nst, record the values hu and qu,
 * update the tau array, and apply the corrections to the zn array.
 * The cv_tau[i] are the last q values of h, with cv_tau[1] the most recent.
 * The counter qwait is decremented, and if qwait == 1 (and q < qmax)
 * we save acor and tq[5] for a possible order increase.
 */

static void CVCompleteStep(CVodeMem cv_mem)
{
	int i, j;
	
	nst++;
	nscon++;
	hu = h;
	qu = cv_q;
	
	for (i=cv_q; i >= 2; i--)  cv_tau[i] = cv_tau[i-1];
	if ((cv_q==1) && (nst > 1)) cv_tau[2] = cv_tau[1];
	cv_tau[1] = h;
	
	for (j=0; j <= cv_q; j++) 
		N_VLinearSum_Serial(cv_l[j], acor, ONE, zn[j], zn[j]);
	qwait--;
	if ((qwait == 1) && (cv_q != qmax)) {
		N_VScale(ONE, acor, zn[qmax]);
		saved_tq5 = tq[5];
		indx_acor = qmax;
	}
}

/*
 * CVprepareNextStep
 *
 * This routine handles the setting of stepsize and order for the
 * next step -- hprime and qprime.  Along with hprime, it sets the
 * ratio eta = hprime/h.  It also updates other state variables 
 * related to a change of step size or order. 
 */

static void CVPrepareNextStep(CVodeMem cv_mem, realtype dsm)
{
	/* If etamax = 1, defer step size or order changes */
	if (etamax == ONE) {
		qwait = MAX(qwait, 2);
		qprime = cv_q;
		hprime = h;
		eta = ONE;
		return;
	}
	
	/* etaq is the ratio of new to old h at the current order */  
	etaq = ONE /(RPowerR(BIAS2*dsm,ONE/L) + ADDON);
	
	/* If no order change, adjust eta and acor in CVSetEta and return */
	if (qwait != 0) {
		eta = etaq;
		qprime = cv_q;
		CVSetEta(cv_mem);
		return;
	}
	
	/* If qwait = 0, consider an order change.   etaqm1 and etaqp1 are 
     the ratios of new to old h at orders q-1 and q+1, respectively.
     CVChooseEta selects the largest; CVSetEta adjusts eta and acor */
	qwait = 2;
	etaqm1 = CVComputeEtaqm1(cv_mem);
	etaqp1 = CVComputeEtaqp1(cv_mem);  
	CVChooseEta(cv_mem); 
	CVSetEta(cv_mem);
}

/*
 * CVsetEta
 *
 * This routine adjusts the value of eta according to the various
 * heuristic limits and the optional input hmax.  It also resets
 * etamax to be the estimated local error vector.
 */

static void CVSetEta(CVodeMem cv_mem)
{
	
	/* If eta below the threshhold THRESH, reject a change of step size */
	if (eta < THRESH) {
		eta = ONE;
		hprime = h;
	} else {
		/* Limit eta by etamax and hmax, then set hprime */
		eta = MIN(eta, etamax);
		eta /= MAX(ONE, ABS(h)*hmax_inv*eta);
		hprime = h * eta;
		if (qprime < cv_q) nscon = 0;
	}
	
	/* Reset etamax for the next step size change, and scale acor */
}

/*
 * CVComputeEtaqm1
 *
 * This routine computes and returns the value of etaqm1 for a
 * possible decrease in order by 1.
 */

static realtype CVComputeEtaqm1(CVodeMem cv_mem)
{
	realtype ddn;
	
	etaqm1 = ZERO;
	if (cv_q > 1) {
		ddn = N_VWrmsNorm(zn[cv_q], ewt) * tq[1];
		etaqm1 = ONE/(RPowerR(BIAS1*ddn, ONE/cv_q) + ADDON);
	}
	return(etaqm1);
}

/*
 * CVComputeEtaqp1
 *
 * This routine computes and returns the value of etaqp1 for a
 * possible increase in order by 1.
 */

static realtype CVComputeEtaqp1(CVodeMem cv_mem)
{
	realtype dup, cquot;
	
	etaqp1 = ZERO;
	if (cv_q != qmax) {
		if (saved_tq5 == ZERO) return(etaqp1);
		cquot = (tq[5] / saved_tq5) * RPowerI(h/cv_tau[2], L);
		N_VLinearSum_Serial(-cquot, zn[qmax], ONE, acor, tempv);
		dup = N_VWrmsNorm(tempv, ewt) * tq[3];
		etaqp1 = ONE / (RPowerR(BIAS3*dup, ONE/(L+1)) + ADDON);
	}
	return(etaqp1);
}

/*
 * CVChooseEta
 * Given etaqm1, etaq, etaqp1 (the values of eta for qprime =
 * q - 1, q, or q + 1, respectively), this routine chooses the 
 * maximum eta value, sets eta to that value, and sets qprime to the
 * corresponding value of q.  If there is a tie, the preference
 * order is to (1) keep the same order, then (2) decrease the order,
 * and finally (3) increase the order.  If the maximum eta value
 * is below the threshhold THRESH, the order is kept unchanged and
 * eta is set to 1.
 */

static void CVChooseEta(CVodeMem cv_mem)
{
	realtype etam;
	
	etam = MAX(etaqm1, MAX(etaq, etaqp1));
	
	if (etam < THRESH) {
		eta = ONE;
		qprime = cv_q;
		return;
	}
	
	if (etam == etaq) {
		
		eta = etaq;
		qprime = cv_q;
		
	} else if (etam == etaqm1) {
		
		eta = etaqm1;
		qprime = cv_q - 1;
		
	} else {
		
		eta = etaqp1;
		qprime = cv_q + 1;
		
		if (lmm == CV_BDF) {
			
			/* 
			 * Store Delta_n in zn[qmax] to be used in order increase 
			 *
			 * This happens at the last step of order q before an increase
			 * to order q+1, so it represents Delta_n in the ELTE at q+1
			 */
			
			N_VScale(ONE, acor, zn[qmax]);
			
		}
		
	}
	
}

/*
 * CVHandleFailure
 *
 * This routine prints error messages for all cases of failure by
 * CVHin and CVStep. It returns to CVode the value that CVode is 
 * to return to the user.
 */

static int CVHandleFailure(CVodeMem cv_mem, int flag)
{
	
	/* Set vector of  absolute weighted local errors */
	/*
	 N_VProd(acor, ewt, tempv);
	 N_VAbs(tempv, tempv);
	 */
	
	/* Depending on flag, print error message and return error flag */
	switch (flag) {
		case CV_ERR_FAILURE: 
			CVProcessError(cv_mem, CV_ERR_FAILURE, "CVODE", "CVode", MSGCV_ERR_FAILS, tn, h);
			break;
		case CV_CONV_FAILURE:
			CVProcessError(cv_mem, CV_CONV_FAILURE, "CVODE", "CVode", MSGCV_CONV_FAILS, tn, h);
			break;
		case CV_LSETUP_FAIL:
			CVProcessError(cv_mem, CV_LSETUP_FAIL, "CVODE", "CVode", MSGCV_SETUP_FAILED, tn);
			break;
		case CV_LSOLVE_FAIL:
			CVProcessError(cv_mem, CV_LSOLVE_FAIL, "CVODE", "CVode", MSGCV_SOLVE_FAILED, tn);
			break;
		case CV_RHSFUNC_FAIL:
			CVProcessError(cv_mem, CV_RHSFUNC_FAIL, "CVODE", "CVode", MSGCV_RHSFUNC_FAILED, tn);
			break;
		case CV_UNREC_RHSFUNC_ERR:
			CVProcessError(cv_mem, CV_UNREC_RHSFUNC_ERR, "CVODE", "CVode", MSGCV_RHSFUNC_UNREC, tn);
			break;
		case CV_REPTD_RHSFUNC_ERR:
			CVProcessError(cv_mem, CV_REPTD_RHSFUNC_ERR, "CVODE", "CVode", MSGCV_RHSFUNC_REPTD, tn);
			break;
		case CV_RTFUNC_FAIL:    
			CVProcessError(cv_mem, CV_RTFUNC_FAIL, "CVODE", "CVode", MSGCV_RTFUNC_FAILED, tn);
			break;
		case CV_TOO_CLOSE:
			CVProcessError(cv_mem, CV_TOO_CLOSE, "CVODE", "CVode", MSGCV_TOO_CLOSE);
			break;
		default:
			return(CV_SUCCESS);   
	}
	
	return(flag);
	
}

/* 
 * =================================================================
 * BDF Stability Limit Detection                       
 * =================================================================
 */

/*
 * CVBDFStab
 *
 * This routine handles the BDF Stability Limit Detection Algorithm
 * STALD.  It is called if lmm = CV_BDF and the SLDET option is on.
 * If the order is 3 or more, the required norm data is saved.
 * If a decision to reduce order has not already been made, and
 * enough data has been saved, CVsldet is called.  If it signals
 * a stability limit violation, the order is reduced, and the step
 * size is reset accordingly.
 */

void CVBDFStab(CVodeMem cv_mem)
{
	int i,k, ldflag, factorial;
	realtype sq, sqm1, sqm2;
	
	/* If order is 3 or greater, then save scaled derivative data,
     push old data down in i, then add current values to top.    */
	
	if (cv_q >= 3) {
		for (k = 1; k <= 3; k++)
		{ for (i = 5; i >= 2; i--) ssdat[i][k] = ssdat[i-1][k]; }
		factorial = 1;
		for (i = 1; i <= cv_q-1; i++) factorial *= i;
		sq = factorial*cv_q*(cv_q+1)*acnrm/MAX(tq[5],TINY);
		sqm1 = factorial*cv_q*N_VWrmsNorm(zn[cv_q], ewt);
		sqm2 = factorial*N_VWrmsNorm(zn[cv_q-1], ewt);
		ssdat[1][1] = sqm2*sqm2;
		ssdat[1][2] = sqm1*sqm1;
		ssdat[1][3] = sq*sq;
	}  
	
	if (qprime >= cv_q) {
		
		/* If order is 3 or greater, and enough ssdat has been saved,
		 nscon >= q+5, then call stability limit detection routine.  */
		
		if ( (cv_q >= 3) && (nscon >= cv_q+5) ) {
			ldflag = CVsldet(cv_mem);
			if (ldflag > 3) {
				/* A stability limit violation is indicated by
				 a return flag of 4, 5, or 6.
				 Reduce new order.                     */
				qprime = cv_q-1;
				eta = etaqm1; 
				eta = MIN(eta,etamax);
				eta = eta/MAX(ONE,ABS(h)*hmax_inv*eta);
				hprime = h*eta;
				nor = nor + 1;
			}
		}
	}
	else {
		/* Otherwise, let order increase happen, and 
		 reset stability limit counter, nscon.     */
		nscon = 0;
	}
}

/*
 * CVsldet
 *
 * This routine detects stability limitation using stored scaled 
 * derivatives data. CVsldet returns the magnitude of the
 * dominate characteristic root, rr. The presents of a stability
 * limit is indicated by rr > "something a little less then 1.0",  
 * and a positive kflag. This routine should only be called if
 * order is greater than or equal to 3, and data has been collected
 * for 5 time steps. 
 * 
 * Returned values:
 *    kflag = 1 -> Found stable characteristic root, normal matrix case
 *    kflag = 2 -> Found stable characteristic root, quartic solution
 *    kflag = 3 -> Found stable characteristic root, quartic solution,
 *                 with Newton correction
 *    kflag = 4 -> Found stability violation, normal matrix case
 *    kflag = 5 -> Found stability violation, quartic solution
 *    kflag = 6 -> Found stability violation, quartic solution,
 *                 with Newton correction
 *
 *    kflag < 0 -> No stability limitation, 
 *                 or could not compute limitation.
 *
 *    kflag = -1 -> Min/max ratio of ssdat too small.
 *    kflag = -2 -> For normal matrix case, vmax > vrrt2*vrrt2
 *    kflag = -3 -> For normal matrix case, The three ratios
 *                  are inconsistent.
 *    kflag = -4 -> Small coefficient prevents elimination of quartics.  
 *    kflag = -5 -> R value from quartics not consistent.
 *    kflag = -6 -> No corrected root passes test on qk values
 *    kflag = -7 -> Trouble solving for sigsq.
 *    kflag = -8 -> Trouble solving for B, or R via B.
 *    kflag = -9 -> R via sigsq[k] disagrees with R from data.
 */

static int CVsldet(CVodeMem cv_mem)
{
	int i, k, j, it, kmin, kflag = 0;
	realtype rat[5][4], rav[4], qkr[4], sigsq[4], smax[4], ssmax[4];
	realtype drr[4], rrc[4],sqmx[4], qjk[4][4], vrat[5], qc[6][4], qco[6][4];
	realtype rr, rrcut, vrrtol, vrrt2, sqtol, rrtol;
	realtype smink, smaxk, sumrat, sumrsq, vmin, vmax, drrmax, adrr;
	realtype tem, sqmax, saqk, qp, s, sqmaxk, saqj, sqmin;
	realtype rsa, rsb, rsc, rsd, rd1a, rd1b, rd1c;
	realtype rd2a, rd2b, rd3a, cest1, corr1; 
	realtype ratp, ratm, qfac1, qfac2, bb, rrb;
	
	/* The following are cutoffs and tolerances used by this routine */
	
	rrcut  = RCONST(0.98);
	vrrtol = RCONST(1.0e-4);
	vrrt2  = RCONST(5.0e-4);
	sqtol  = RCONST(1.0e-3);
	rrtol  = RCONST(1.0e-2);
	
	rr = ZERO;
	
	/*  Index k corresponds to the degree of the interpolating polynomial. */
	/*      k = 1 -> q-1          */
	/*      k = 2 -> q            */
	/*      k = 3 -> q+1          */
	
	/*  Index i is a backward-in-time index, i = 1 -> current time, */
	/*      i = 2 -> previous step, etc    */
	
	/* get maxima, minima, and variances, and form quartic coefficients  */
	
	for (k=1; k<=3; k++) {
		smink = ssdat[1][k];
		smaxk = ZERO;
		
		for (i=1; i<=5; i++) {
			smink = MIN(smink,ssdat[i][k]);
			smaxk = MAX(smaxk,ssdat[i][k]);
		}
		
		if (smink < TINY*smaxk) {
			kflag = -1;  
			return(kflag);
		}
		smax[k] = smaxk;
		ssmax[k] = smaxk*smaxk;
		
		sumrat = ZERO;
		sumrsq = ZERO;
		for (i=1; i<=4; i++) {
			rat[i][k] = ssdat[i][k]/ssdat[i+1][k];
			sumrat = sumrat + rat[i][k];
			sumrsq = sumrsq + rat[i][k]*rat[i][k];
		} 
		rav[k] = FOURTH*sumrat;
		vrat[k] = ABS(FOURTH*sumrsq - rav[k]*rav[k]);
		
		qc[5][k] = ssdat[1][k]*ssdat[3][k] - ssdat[2][k]*ssdat[2][k];
		qc[4][k] = ssdat[2][k]*ssdat[3][k] - ssdat[1][k]*ssdat[4][k];
		qc[3][k] = ZERO;
		qc[2][k] = ssdat[2][k]*ssdat[5][k] - ssdat[3][k]*ssdat[4][k];
		qc[1][k] = ssdat[4][k]*ssdat[4][k] - ssdat[3][k]*ssdat[5][k];
		
		for (i=1; i<=5; i++) {
			qco[i][k] = qc[i][k];
		}
	}                            /* End of k loop */
	
	/* Isolate normal or nearly-normal matrix case. Three quartic will
     have common or nearly-common roots in this case. 
     Return a kflag = 1 if this procedure works. If three root 
     differ more than vrrt2, return error kflag = -3.    */
	
	vmin = MIN(vrat[1],MIN(vrat[2],vrat[3]));
	vmax = MAX(vrat[1],MAX(vrat[2],vrat[3]));
	
	if(vmin < vrrtol*vrrtol) {
		if (vmax > vrrt2*vrrt2) {
			kflag = -2;  
			return(kflag);
		} else {
			rr = (rav[1] + rav[2] + rav[3])/THREE;
			
			drrmax = ZERO;
			for(k = 1;k<=3;k++) {
				adrr = ABS(rav[k] - rr);
				drrmax = MAX(drrmax, adrr);
			}
			if (drrmax > vrrt2) {
				kflag = -3;    
			}
			
			kflag = 1;
			
			/*  can compute charactistic root, drop to next section   */
			
		}
	} else {
		
		/* use the quartics to get rr. */
		
		if (ABS(qco[1][1]) < TINY*ssmax[1]) {
			kflag = -4;    
			return(kflag);
		}
		
		tem = qco[1][2]/qco[1][1];
		for(i=2; i<=5; i++) {
			qco[i][2] = qco[i][2] - tem*qco[i][1];
		}
		
		qco[1][2] = ZERO;
		tem = qco[1][3]/qco[1][1];
		for(i=2; i<=5; i++) {
			qco[i][3] = qco[i][3] - tem*qco[i][1];
		}
		qco[1][3] = ZERO;
		
		if (ABS(qco[2][2]) < TINY*ssmax[2]) {
			kflag = -4;    
			return(kflag);
		}
		
		tem = qco[2][3]/qco[2][2];
		for(i=3; i<=5; i++) {
			qco[i][3] = qco[i][3] - tem*qco[i][2];
		}
		
		if (ABS(qco[4][3]) < TINY*ssmax[3]) {
			kflag = -4;    
			return(kflag);
		}
		
		rr = -qco[5][3]/qco[4][3];
		
		if (rr < TINY || rr > HUN) {
			kflag = -5;   
			return(kflag);
		}
		
		for(k=1; k<=3; k++) {
			qkr[k] = qc[5][k] + rr*(qc[4][k] + rr*rr*(qc[2][k] + rr*qc[1][k]));
		}  
		
		sqmax = ZERO;
		for(k=1; k<=3; k++) {
			saqk = ABS(qkr[k])/ssmax[k];
			if (saqk > sqmax) sqmax = saqk;
		} 
		
		if (sqmax < sqtol) {
			kflag = 2;
			
			/*  can compute charactistic root, drop to "given rr,etc"   */
			
		} else {
			
			/* do Newton corrections to improve rr.  */
			
			for(it=1; it<=3; it++) {
				for(k=1; k<=3; k++) {
					qp = qc[4][k] + rr*rr*(THREE*qc[2][k] + rr*FOUR*qc[1][k]);
					drr[k] = ZERO;
					if (ABS(qp) > TINY*ssmax[k]) drr[k] = -qkr[k]/qp;
					rrc[k] = rr + drr[k];
				} 
				
				for(k=1; k<=3; k++) {
					s = rrc[k];
					sqmaxk = ZERO;
					for(j=1; j<=3; j++) {
						qjk[j][k] = qc[5][j] + s*(qc[4][j] + 
												  s*s*(qc[2][j] + s*qc[1][j]));
						saqj = ABS(qjk[j][k])/ssmax[j];
						if (saqj > sqmaxk) sqmaxk = saqj;
					} 
					sqmx[k] = sqmaxk;
				} 
				
				sqmin = sqmx[1]; kmin = 1;
				for(k=2; k<=3; k++) {
					if (sqmx[k] < sqmin) {
						kmin = k;
						sqmin = sqmx[k];
					}
				} 
				rr = rrc[kmin];
				
				if (sqmin < sqtol) {
					kflag = 3;
					/*  can compute charactistic root   */
					/*  break out of Newton correction loop and drop to "given rr,etc" */ 
					break;
				} else {
					for(j=1; j<=3; j++) {
						qkr[j] = qjk[j][kmin];
					}
				}     
			}          /*  end of Newton correction loop  */ 
			
			if (sqmin > sqtol) {
				kflag = -6;
				return(kflag);
			}
		}     /*  end of if (sqmax < sqtol) else   */
	}      /*  end of if(vmin < vrrtol*vrrtol) else, quartics to get rr. */
	
	/* given rr, find sigsq[k] and verify rr.  */
	/* All positive kflag drop to this section  */
	
	for(k=1; k<=3; k++) {
		rsa = ssdat[1][k];
		rsb = ssdat[2][k]*rr;
		rsc = ssdat[3][k]*rr*rr;
		rsd = ssdat[4][k]*rr*rr*rr;
		rd1a = rsa - rsb;
		rd1b = rsb - rsc;
		rd1c = rsc - rsd;
		rd2a = rd1a - rd1b;
		rd2b = rd1b - rd1c;
		rd3a = rd2a - rd2b;
		
		if (ABS(rd1b) < TINY*smax[k]) {
			kflag = -7;
			return(kflag);
		}
		
		cest1 = -rd3a/rd1b;
		if (cest1 < TINY || cest1 > FOUR) {
			kflag = -7;
			return(kflag);
		}
		corr1 = (rd2b/cest1)/(rr*rr);
		sigsq[k] = ssdat[3][k] + corr1;
	}
	
	if (sigsq[2] < TINY) {
		kflag = -8;
		return(kflag);
	}
	
	ratp = sigsq[3]/sigsq[2];
	ratm = sigsq[1]/sigsq[2];
	qfac1 = FOURTH*(cv_q*cv_q - ONE);
	qfac2 = TWO/(cv_q - ONE);
	bb = ratp*ratm - ONE - qfac1*ratp;
	tem = ONE - qfac2*bb;
	
	if (ABS(tem) < TINY) {
		kflag = -8;
		return(kflag);
	}
	
	rrb = ONE/tem;
	
	if (ABS(rrb - rr) > rrtol) {
		kflag = -9;
		return(kflag);
	}
	
	/* Check to see if rr is above cutoff rrcut  */
	if (rr > rrcut) {
		if (kflag == 1) kflag = 4;
		if (kflag == 2) kflag = 5;
		if (kflag == 3) kflag = 6;
	}
	
	/* All positive kflag returned at this point  */
	
	return(kflag);
	
}

/* 
 * =================================================================
 * Root finding   
 * =================================================================
 */

/*-----------------------------------------------------------------*/

/* 
 * CVRcheck1
 *
 * This routine completes the initialization of rootfinding memory
 * information, and checks whether g has a zero both at and very near
 * the initial point of the IVP.
 *
 * This routine returns an int equal to:
 *  CV_RTFUNC_FAIL = -12  if the g function failed, or
 *  CV_SUCCESS     =   0  otherwise.
 */

static int CVRcheck1(CVodeMem cv_mem)
{
	int i, retval;
	realtype smallh, hratio;
	booleantype zroot;
	
	for (i = 0; i < nrtfn; i++) iroots[i] = 0;
	tlo = tn;
	ttol = (ABS(tn) + ABS(h))*uround*HUN;
	
	/* Evaluate g at initial t and check for zero values. */
	retval = gfun(tlo, zn[0], glo, cv_user_data);
	nge = 1;
	if (retval != 0) return(CV_RTFUNC_FAIL);
	
	zroot = FALSE;
	for (i = 0; i < nrtfn; i++) {
		if (ABS(glo[i]) == ZERO) {
			zroot = TRUE;
			gactive[i] = FALSE;
		}
	}
	if (!zroot) return(CV_SUCCESS);
	
	/* Some g_i is zero at t0; look at g at t0+(small increment). */
	hratio = MAX(ttol/ABS(h), TENTH);
	smallh = hratio*h;
	tlo += smallh;
	N_VLinearSum_Serial(ONE, zn[0], hratio, zn[1], cv_y);
	retval = gfun(tlo, cv_y, glo, cv_user_data);
	nge++;
	if (retval != 0) return(CV_RTFUNC_FAIL);
	
	/* We check now only the components of g which were exactly 0.0 at t0
	 * to see if we can 'activate' them. */
	
	for (i = 0; i < nrtfn; i++) {
		if (!gactive[i] && ABS(glo[i]) != ZERO) {
			gactive[i] = TRUE;
			
		}
	}
	
	return(CV_SUCCESS);
}

/*
 * CVRcheck2
 *
 * This routine checks for exact zeros of g at the last root found,
 * if the last return was a root.  It then checks for a close
 * pair of zeros (an error condition), and for a new root at a
 * nearby point.  The left endpoint (tlo) of the search interval
 * is adjusted if necessary to assure that all g_i are nonzero
 * there, before returning to do a root search in the interval.
 *
 * On entry, tlo = tretlast is the last value of tret returned by
 * CVode.  This may be the previous tn, the previous tout value, or
 * the last root location.
 *
 * This routine returns an int equal to:
 *      CV_RTFUNC_FAIL = -12 if the g function failed, or
 *      CLOSERT        =  3  if a close pair of zeros was found, or
 *      RTFOUND        =  1  if a new zero of g was found near tlo, or
 *      CV_SUCCESS     =  0  otherwise.
 */

static int CVRcheck2(CVodeMem cv_mem)
{
	int i, retval;
	realtype smallh, hratio;
	booleantype zroot;
	
	if (irfnd == 0) return(CV_SUCCESS);
	
	(void) CVodeGetDky(cv_mem, tlo, 0, cv_y);
	retval = gfun(tlo, cv_y, glo, cv_user_data);
	nge++;
	if (retval != 0) return(CV_RTFUNC_FAIL);
	
	zroot = FALSE;
	for (i = 0; i < nrtfn; i++) iroots[i] = 0;
	for (i = 0; i < nrtfn; i++) {
		if (!gactive[i]) continue;
		if (ABS(glo[i]) == ZERO) {
			zroot = TRUE;
			iroots[i] = 1;
		}
	}
	if (!zroot) return(CV_SUCCESS);
	
	/* One or more g_i has a zero at tlo.  Check g at tlo+smallh. */
	ttol = (ABS(tn) + ABS(h))*uround*HUN;
	smallh = (h > ZERO) ? ttol : -ttol;
	tlo += smallh;
	if ( (tlo - tn)*h >= ZERO) {
		hratio = smallh/h;
		N_VLinearSum_Serial(ONE, cv_y, hratio, zn[1], cv_y);
	} else {
		(void) CVodeGetDky(cv_mem, tlo, 0, cv_y);
	}
	retval = gfun(tlo, cv_y, glo, cv_user_data);
	nge++;
	if (retval != 0) return(CV_RTFUNC_FAIL);
	
	zroot = FALSE;
	for (i = 0; i < nrtfn; i++) {
		if (ABS(glo[i]) == ZERO) {
			if (!gactive[i]) continue;
			if (iroots[i] == 1) return(CLOSERT);
			zroot = TRUE;
			iroots[i] = 1;
		}
	}
	if (zroot) return(RTFOUND);
	return(CV_SUCCESS);
	
}

/*
 * CVRcheck3
 *
 * This routine interfaces to CVRootfind to look for a root of g
 * between tlo and either tn or tout, whichever comes first.
 * Only roots beyond tlo in the direction of integration are sought.
 *
 * This routine returns an int equal to:
 *      CV_RTFUNC_FAIL = -12 if the g function failed, or
 *      RTFOUND        =  1  if a root of g was found, or
 *      CV_SUCCESS     =  0  otherwise.
 */

static int CVRcheck3(CVodeMem cv_mem)
{
	int i, retval, ier;
	
	/* Set thi = tn or tout, whichever comes first; set y = y(thi). */
	if (taskc == CV_ONE_STEP) {
		thi = tn;
		N_VScale(ONE, zn[0], cv_y);
	}
	if (taskc == CV_NORMAL) {
		if ( (toutc - tn)*h >= ZERO) {
			thi = tn; 
			N_VScale(ONE, zn[0], cv_y);
		} else {
			thi = toutc;
			(void) CVodeGetDky(cv_mem, thi, 0, cv_y);
		}
	}
	
	/* Set ghi = g(thi) and call CVRootfind to search (tlo,thi) for roots. */
	retval = gfun(thi, cv_y, ghi, cv_user_data);
	nge++;
	if (retval != 0) return(CV_RTFUNC_FAIL);
	
	ttol = (ABS(tn) + ABS(h))*uround*HUN;
	ier = CVRootfind(cv_mem);
	if (ier == CV_RTFUNC_FAIL) return(CV_RTFUNC_FAIL);
	for(i=0; i<nrtfn; i++) {
		if(!gactive[i] && grout[i] != ZERO) gactive[i] = TRUE;
	}
	tlo = trout;
	for (i = 0; i < nrtfn; i++) glo[i] = grout[i];
	
	/* If no root found, return CV_SUCCESS. */  
	if (ier == CV_SUCCESS) return(CV_SUCCESS);
	
	/* If a root was found, interpolate to get y(trout) and return.  */
	(void) CVodeGetDky(cv_mem, trout, 0, cv_y);
	return(RTFOUND);
	
}

/*
 * CVRootfind
 *
 * This routine solves for a root of g(t) between tlo and thi, if
 * one exists.  Only roots of odd multiplicity (i.e. with a change
 * of sign in one of the g_i), or exact zeros, are found.
 * Here the sign of tlo - thi is arbitrary, but if multiple roots
 * are found, the one closest to tlo is returned.
 *
 * The method used is the Illinois algorithm, a modified secant method.
 * Reference: Kathie L. Hiebert and Lawrence F. Shampine, Implicitly
 * Defined Output Points for Solutions of ODEs, Sandia National
 * Laboratory Report SAND80-0180, February 1980.
 *
 * This routine uses the following parameters for communication:
 *
 * nrtfn    = number of functions g_i, or number of components of
 *            the vector-valued function g(t).  Input only.
 *
 * gfun     = user-defined function for g(t).  Its form is
 *            (void) gfun(t, y, gt, cv_user_data)
 *
 * rootdir  = in array specifying the direction of zero-crossings.
 *            If rootdir[i] > 0, search for roots of g_i only if
 *            g_i is increasing; if rootdir[i] < 0, search for
 *            roots of g_i only if g_i is decreasing; otherwise
 *            always search for roots of g_i.
 *
 * gactive  = array specifying whether a component of g should
 *            or should not be monitored. gactive[i] is initially
 *            set to TRUE for all i=0,...,nrtfn-1, but it may be
 *            reset to FALSE if at the first step g[i] is 0.0
 *            both at the I.C. and at a small perturbation of them.
 *            gactive[i] is then set back on TRUE only after the 
 *            corresponding g function moves away from 0.0.
 *
 * nge      = cumulative counter for gfun calls.
 *
 * ttol     = a convergence tolerance for trout.  Input only.
 *            When a root at trout is found, it is located only to
 *            within a tolerance of ttol.  Typically, ttol should
 *            be set to a value on the order of
 *               100 * UROUND * max (ABS(tlo), ABS(thi))
 *            where UROUND is the unit roundoff of the machine.
 *
 * tlo, thi = endpoints of the interval in which roots are sought.
 *            On input, and must be distinct, but tlo - thi may
 *            be of either sign.  The direction of integration is
 *            assumed to be from tlo to thi.  On return, tlo and thi
 *            are the endpoints of the final relevant interval.
 *
 * glo, ghi = arrays of length nrtfn containing the vectors g(tlo)
 *            and g(thi) respectively.  Input and output.  On input,
 *            none of the glo[i] should be zero.
 *
 * trout    = root location, if a root was found, or thi if not.
 *            Output only.  If a root was found other than an exact
 *            zero of g, trout is the endpoint thi of the final
 *            interval bracketing the root, with size at most ttol.
 *
 * grout    = array of length nrtfn containing g(trout) on return.
 *
 * iroots   = int array of length nrtfn with root information.
 *            Output only.  If a root was found, iroots indicates
 *            which components g_i have a root at trout.  For
 *            i = 0, ..., nrtfn-1, iroots[i] = 1 if g_i has a root
 *            and g_i is increasing, iroots[i] = -1 if g_i has a
 *            root and g_i is decreasing, and iroots[i] = 0 if g_i
 *            has no roots or g_i varies in the direction opposite
 *            to that indicated by rootdir[i].
 *
 * This routine returns an int equal to:
 *      CV_RTFUNC_FAIL = -12 if the g function failed, or
 *      RTFOUND        =  1  if a root of g was found, or
 *      CV_SUCCESS     =  0  otherwise.
 */

static int CVRootfind(CVodeMem cv_mem)
{
	realtype alpha, tmid, gfrac, maxfrac, fracint, fracsub;
	int i, retval, imax, side, sideprev;
	booleantype zroot, sgnchg;
	
	imax = 0;
	
	/* First check for change in sign in ghi or for a zero in ghi. */
	maxfrac = ZERO;
	zroot = FALSE;
	sgnchg = FALSE;
	for (i = 0;  i < nrtfn; i++) {
		if(!gactive[i]) continue;
		if (ABS(ghi[i]) == ZERO) {
			if(rootdir[i]*glo[i] <= ZERO) {
				zroot = TRUE;
			}
		} else {
			if ( (glo[i]*ghi[i] < ZERO) && (rootdir[i]*glo[i] <= ZERO) ) {
				gfrac = ABS(ghi[i]/(ghi[i] - glo[i]));
				if (gfrac > maxfrac) {
					sgnchg = TRUE;
					maxfrac = gfrac;
					imax = i;
				}
			}
		}
	}
	
	/* If no sign change was found, reset trout and grout.  Then return
     CV_SUCCESS if no zero was found, or set iroots and return RTFOUND.  */ 
	if (!sgnchg) {
		trout = thi;
		for (i = 0; i < nrtfn; i++) grout[i] = ghi[i];
		if (!zroot) return(CV_SUCCESS);
		for (i = 0; i < nrtfn; i++) {
			iroots[i] = 0;
			if(!gactive[i]) continue;
			if (ABS(ghi[i]) == ZERO) iroots[i] = glo[i] > 0 ? -1:1;
		}
		return(RTFOUND);
	}
	
	/* Initialize alpha to avoid compiler warning */
	alpha = ONE;
	
	/* A sign change was found.  Loop to locate nearest root. */
	
	side = 0;  sideprev = -1;
	loop {                                    /* Looping point */
		
		/* Set weight alpha.
		 On the first two passes, set alpha = 1.  Thereafter, reset alpha
		 according to the side (low vs high) of the subinterval in which
		 the sign change was found in the previous two passes.
		 If the sides were opposite, set alpha = 1.
		 If the sides were the same, then double alpha (if high side),
		 or halve alpha (if low side).
		 The next guess tmid is the secant method value if alpha = 1, but
		 is closer to tlo if alpha < 1, and closer to thi if alpha > 1.    */
		
		if (sideprev == side) {
			alpha = (side == 2) ? alpha*TWO : alpha*HALF;
		} else {
			alpha = ONE;
		}
		
		/* Set next root approximation tmid and get g(tmid).
		 If tmid is too close to tlo or thi, adjust it inward,
		 by a fractional distance that is between 0.1 and 0.5.  */
		tmid = thi - (thi - tlo)*ghi[imax]/(ghi[imax] - alpha*glo[imax]);
		if (ABS(tmid - tlo) < HALF*ttol) {
			fracint = ABS(thi - tlo)/ttol;
			fracsub = (fracint > FIVE) ? TENTH : HALF/fracint;
			tmid = tlo + fracsub*(thi - tlo);
		}
		if (ABS(thi - tmid) < HALF*ttol) {
			fracint = ABS(thi - tlo)/ttol;
			fracsub = (fracint > FIVE) ? TENTH : HALF/fracint;
			tmid = thi - fracsub*(thi - tlo);
		}
		
		(void) CVodeGetDky(cv_mem, tmid, 0, cv_y);
		retval = gfun(tmid, cv_y, grout, cv_user_data);
		nge++;
		if (retval != 0) return(CV_RTFUNC_FAIL);
		
		/* Check to see in which subinterval g changes sign, and reset imax.
		 Set side = 1 if sign change is on low side, or 2 if on high side.  */  
		maxfrac = ZERO;
		zroot = FALSE;
		sgnchg = FALSE;
		sideprev = side;
		for (i = 0;  i < nrtfn; i++) {
			if(!gactive[i]) continue;
			if (ABS(grout[i]) == ZERO) {
				if(rootdir[i]*glo[i] <= ZERO) {
					zroot = TRUE;
				}
			} else {
				if ( (glo[i]*grout[i] < ZERO) && (rootdir[i]*glo[i] <= ZERO) ) {
					gfrac = ABS(grout[i]/(grout[i] - glo[i]));
					if (gfrac > maxfrac) {
						sgnchg = TRUE;
						maxfrac = gfrac;
						imax = i;
					}
				}
			}
		}
		if (sgnchg) {
			/* Sign change found in (tlo,tmid); replace thi with tmid. */
			thi = tmid;
			for (i = 0; i < nrtfn; i++) ghi[i] = grout[i];
			side = 1;
			/* Stop at root thi if converged; otherwise loop. */
			if (ABS(thi - tlo) <= ttol) break;
			continue;  /* Return to looping point. */
		}
		
		if (zroot) {
			/* No sign change in (tlo,tmid), but g = 0 at tmid; return root tmid. */
			thi = tmid;
			for (i = 0; i < nrtfn; i++) ghi[i] = grout[i];
			break;
		}
		
		/* No sign change in (tlo,tmid), and no zero at tmid.
		 Sign change must be in (tmid,thi).  Replace tlo with tmid. */
		tlo = tmid;
		for (i = 0; i < nrtfn; i++) glo[i] = grout[i];
		side = 2;
		/* Stop at root thi if converged; otherwise loop back. */
		if (ABS(thi - tlo) <= ttol) break;
		
	} /* End of root-search loop */
	
	/* Reset trout and grout, set iroots, and return RTFOUND. */
	trout = thi;
	for (i = 0; i < nrtfn; i++) {
		grout[i] = ghi[i];
		iroots[i] = 0;
		if(!gactive[i]) continue;
		if ( (ABS(ghi[i]) == ZERO) && (rootdir[i]*glo[i] <= ZERO) ) 
			iroots[i] = glo[i] > 0 ? -1:1;
		if ( (glo[i]*ghi[i] < ZERO) && (rootdir[i]*glo[i] <= ZERO) ) 
			iroots[i] = glo[i] > 0 ? -1:1;
	}
	return(RTFOUND);
}

/* 
 * =================================================================
 * Internal EWT function
 * =================================================================
 */

/*
 * CVEwtSet
 *
 * This routine is responsible for setting the error weight vector ewt,
 * according to tol_type, as follows:
 *
 * (1) ewt[i] = 1 / (reltol * ABS(ycur[i]) + *abstol), i=0,...,neq-1
 *     if tol_type = CV_SS
 * (2) ewt[i] = 1 / (reltol * ABS(ycur[i]) + abstol[i]), i=0,...,neq-1
 *     if tol_type = CV_SV
 *
 * CVEwtSet returns 0 if ewt is successfully set as above to a
 * positive vector and -1 otherwise. In the latter case, ewt is
 * considered undefined.
 *
 * All the real work is done in the routines CVEwtSetSS, CVEwtSetSV.
 */

int CVEwtSet(N_Vector ycur, N_Vector weight, void *data)
{
	CVodeMem cv_mem;
	int flag = 0;
	
	/* data points to cv_mem here */
	
	cv_mem = (CVodeMem) data;
	
	switch(itol) {
		case CV_SS: 
			flag = CVEwtSetSS(cv_mem, ycur, weight);
			break;
		case CV_SV: 
			flag = CVEwtSetSV(cv_mem, ycur, weight);
			break;
	}
	
	return(flag);
}

/*
 * CVEwtSetSS
 *
 * This routine sets ewt as decribed above in the case tol_type = CV_SS.
 * It tests for non-positive components before inverting. CVEwtSetSS
 * returns 0 if ewt is successfully set to a positive vector
 * and -1 otherwise. In the latter case, ewt is considered undefined.
 */

static int CVEwtSetSS(CVodeMem cv_mem, N_Vector ycur, N_Vector weight)
{
	N_VAbs(ycur, tempv);
	N_VScale(cv_reltol, tempv, tempv);
	N_VAddConst(tempv, Sabstol, tempv);
	if (N_VMin(tempv) <= ZERO) return(-1);
	N_VInv(tempv, weight);
	return(0);
}

/*
 * CVEwtSetSV
 *
 * This routine sets ewt as decribed above in the case tol_type = CV_SV.
 * It tests for non-positive components before inverting. CVEwtSetSV
 * returns 0 if ewt is successfully set to a positive vector
 * and -1 otherwise. In the latter case, ewt is considered undefined.
 */

static int CVEwtSetSV(CVodeMem cv_mem, N_Vector ycur, N_Vector weight)
{
	N_VAbs(ycur, tempv);
	N_VLinearSum_Serial(cv_reltol, tempv, ONE, Vabstol, tempv);
	if (N_VMin(tempv) <= ZERO) return(-1);
	N_VInv(tempv, weight);
	return(0);
}

/* 
 * =================================================================
 * CVODE Error Handling function   
 * =================================================================
 */

/* 
 * CVProcessError is a high level error handling function
 * - if cv_mem==NULL it prints the error message to stderr
 * - otherwise, it sets-up and calls the error hadling function 
 *   pointed to by cv_ehfun
 */

#define ehfun    (cv_mem->cv_ehfun)
#define eh_data  (cv_mem->cv_eh_data)

void CVProcessError(CVodeMem cv_mem, 
                    int error_code, const char *module, const char *fname, 
                    const char *msgfmt, ...)
{
	va_list ap;
	char msg[256];
	
	/* Initialize the argument pointer variable 
     (msgfmt is the last required argument to CVProcessError) */
	
	va_start(ap, msgfmt);
	
	if (cv_mem == NULL) {    /* We write to stderr */
		
#ifndef NO_FPRINTF_OUTPUT
		fprintf(stderr, "\n[%s ERROR]  %s\n  ", module, fname);
		fprintf(stderr, msgfmt);
		fprintf(stderr, "\n\n");
#endif
		
	} else {                 /* We can call ehfun */
		
		/* Compose the message */
		
		vsprintf(msg, msgfmt, ap);
		
		/* Call ehfun */
		
		ehfun(error_code, module, fname, msg, eh_data);
		
	}
	
	/* Finalize argument processing */
	
	va_end(ap);
	
	return;
	
}

/* CVErrHandler is the default error handling function.
 It sends the error message to the stream pointed to by cv_errfp */

#define errfp    (cv_mem->cv_errfp)

void CVErrHandler(int error_code, const char *module,
                  const char *function, char *msg, void *data)
{
	CVodeMem cv_mem;
	char err_type[10];
	
	/* data points to cv_mem here */
	
	cv_mem = (CVodeMem) data;
	
	if (error_code == CV_WARNING)
		sprintf(err_type,"WARNING");
	else
		sprintf(err_type,"ERROR");
	
#ifndef NO_FPRINTF_OUTPUT
	if (errfp!=NULL) {
		fprintf(errfp,"\n[%s %s]  %s\n",module,err_type,function);
		fprintf(errfp,"  %s\n\n",msg);
	}
#endif
	
	return;
}

/*
 * -----------------------------------------------------------------
 * Functions in the 'ops' structure
 * -----------------------------------------------------------------
 */

N_Vector N_VClone(N_Vector w)
{
	N_Vector v = NULL;
	v = w->ops->nvclone(w);
	return(v);
}

N_Vector N_VCloneEmpty(N_Vector w)
{
	N_Vector v = NULL;
	v = w->ops->nvcloneempty(w);
	return(v);
}

void N_VDestroy(N_Vector v)
{
	if (v==NULL) return;
	v->ops->nvdestroy(v);
	return;
}

/*
void N_VSpace(N_Vector v, long int *lrw, long int *liw)
{
	v->ops->nvspace(v, lrw, liw);
	return;
}
*/ 
 
realtype *N_VGetArrayPointer(N_Vector v)
{
	return((realtype *) v->ops->nvgetarraypointer(v));
}

void N_VSetArrayPointer(realtype *v_data, N_Vector v)
{
	v->ops->nvsetarraypointer(v_data, v);
	return;
}

/*
void N_VLinearSum(realtype a, N_Vector x, realtype b, N_Vector y, N_Vector z)
{
	z->ops->nvlinearsum(a, x, b, y, z);
	return;
}
*/
 
void N_VConst(realtype c, N_Vector z)
{
	z->ops->nvconst(c, z);
	return;
}

/*
void N_VProd(N_Vector x, N_Vector y, N_Vector z)
{
	z->ops->nvprod(x, y, z);
	return;
}
*/

/*
void N_VDiv(N_Vector x, N_Vector y, N_Vector z)
{
	z->ops->nvdiv(x, y, z);
	return;
}
*/
 
void N_VScale(realtype c, N_Vector x, N_Vector z) 
{
	z->ops->nvscale(c, x, z);
	return;
}

void N_VAbs(N_Vector x, N_Vector z)
{
	z->ops->nvabs(x, z);
	return;
}

void N_VInv(N_Vector x, N_Vector z)
{
	z->ops->nvinv(x, z);
	return;
}

void N_VAddConst(N_Vector x, realtype b, N_Vector z)
{
	z->ops->nvaddconst(x, b, z);
	return;
}

/*
realtype N_VDotProd(N_Vector x, N_Vector y)
{
	return((realtype) y->ops->nvdotprod(x, y));
}
*/
 
realtype N_VMaxNorm(N_Vector x)
{
	return((realtype) x->ops->nvmaxnorm(x));
}

realtype N_VWrmsNorm(N_Vector x, N_Vector w)
{
	return((realtype) x->ops->nvwrmsnorm(x, w));
}

realtype N_VWrmsNormMask(N_Vector x, N_Vector w, N_Vector id)
{
	return((realtype) x->ops->nvwrmsnormmask(x, w, id));
}

realtype N_VMin(N_Vector x)
{
	return((realtype) x->ops->nvmin(x));
}

realtype N_VWL2Norm(N_Vector x, N_Vector w)
{
	return((realtype) x->ops->nvwl2norm(x, w));
}

realtype N_VL1Norm(N_Vector x)
{
	return((realtype) x->ops->nvl1norm(x));
}

void N_VCompare(realtype c, N_Vector x, N_Vector z)
{
	z->ops->nvcompare(c, x, z);
	return;
}

booleantype N_VInvTest(N_Vector x, N_Vector z)
{
	return((booleantype) z->ops->nvinvtest(x, z));
}

booleantype N_VConstrMask(N_Vector c, N_Vector x, N_Vector m)
{
	return((booleantype) x->ops->nvconstrmask(c, x, m));
}

realtype N_VMinQuotient(N_Vector num, N_Vector denom)
{
	return((realtype) num->ops->nvminquotient(num, denom));
}

/*
 * -----------------------------------------------------------------
 * Additional functions exported by the generic NVECTOR:
 *   N_VCloneEmptyVectorArray
 *   N_VCloneVectorArray
 *   N_VDestroyVectorArray
 * -----------------------------------------------------------------
 */

N_Vector *N_VCloneEmptyVectorArray(int count, N_Vector w)
{
	N_Vector *vs = NULL;
	int j;
	
	if (count <= 0) return(NULL);
	
	vs = (N_Vector *) malloc(count * sizeof(N_Vector));
	if(vs == NULL) return(NULL);
	
	for (j = 0; j < count; j++) {
		vs[j] = N_VCloneEmpty(w);
		if (vs[j] == NULL) {
			N_VDestroyVectorArray(vs, j-1);
			return(NULL);
		}
	}
	
	return(vs);
}

N_Vector *N_VCloneVectorArray(int count, N_Vector w)
{
	N_Vector *vs = NULL;
	int j;
	
	if (count <= 0) return(NULL);
	
	vs = (N_Vector *) malloc(count * sizeof(N_Vector));
	if(vs == NULL) return(NULL);
	
	for (j = 0; j < count; j++) {
		vs[j] = N_VClone(w);
		if (vs[j] == NULL) {
			N_VDestroyVectorArray(vs, j-1);
			return(NULL);
		}
	}
	
	return(vs);
}

void N_VDestroyVectorArray(N_Vector *vs, int count)
{
	int j;
	
	if (vs==NULL) return;
	
	for (j = 0; j < count; j++) N_VDestroy(vs[j]);
	
	free(vs); vs = NULL;
	
	return;
}

int CVDense(void *cvode_mem, int N)
{
	CVodeMem cv_mem;
	CVDlsMem cvdls_mem;
	
	/* Return immediately if cvode_mem is NULL */
	if (cvode_mem == NULL) {
		CVProcessError(NULL, CVDLS_MEM_NULL, "CVDENSE", "CVDense", MSGD_CVMEM_NULL);
		return(CVDLS_MEM_NULL);
	}
	cv_mem = (CVodeMem) cvode_mem;
	
	/* Test if the NVECTOR package is compatible with the DENSE solver */
	if (vec_tmpl->ops->nvgetarraypointer == NULL ||
		vec_tmpl->ops->nvsetarraypointer == NULL) {
		CVProcessError(cv_mem, CVDLS_ILL_INPUT, "CVDENSE", "CVDense", MSGD_BAD_NVECTOR);
		return(CVDLS_ILL_INPUT);
	}
	
	if (lfree !=NULL) lfree(cv_mem);
	
	/* Set four main function fields in cv_mem */
	linit  = cvDenseInit;
	lsetup = cvDenseSetup;
	lsolve = cvDenseSolve;
	lfree  = cvDenseFree;
	
	/* Get memory for CVDlsMemRec */
	cvdls_mem = NULL;
	cvdls_mem = (CVDlsMem) malloc(sizeof(struct CVDlsMemRec));
	if (cvdls_mem == NULL) {
		CVProcessError(cv_mem, CVDLS_MEM_FAIL, "CVDENSE", "CVDense", MSGD_MEM_FAIL);
		return(CVDLS_MEM_FAIL);
	}
	
	/* Set matrix type */
	mtype = SUNDIALS_DENSE;
	
	/* Initialize Jacobian-related data */
	jacDQ = TRUE;
	jac = NULL;
	J_data = NULL;
	
	last_flag = CVDLS_SUCCESS;
	
	setupNonNull = TRUE;
	
	/* Set problem dimension */
	d_n = N;
	
	/* Allocate memory for M, savedJ, and pivot array */
	
	d_M = NULL;
	d_M = NewDenseMat(N, N);
	if (d_M == NULL) {
		CVProcessError(cv_mem, CVDLS_MEM_FAIL, "CVDENSE", "CVDense", MSGD_MEM_FAIL);
		free(cvdls_mem); cvdls_mem = NULL;
		return(CVDLS_MEM_FAIL);
	}
	savedJ = NULL;
	savedJ = NewDenseMat(N, N);
	if (savedJ == NULL) {
		CVProcessError(cv_mem, CVDLS_MEM_FAIL, "CVDENSE", "CVDense", MSGD_MEM_FAIL);
		DestroyMat(d_M);
		free(cvdls_mem); cvdls_mem = NULL;
		return(CVDLS_MEM_FAIL);
	}
	pivots = NULL;
	pivots = NewIntArray(N);
	if (pivots == NULL) {
		CVProcessError(cv_mem, CVDLS_MEM_FAIL, "CVDENSE", "CVDense", MSGD_MEM_FAIL);
		DestroyMat(d_M);
		DestroyMat(savedJ);
		free(cvdls_mem); cvdls_mem = NULL;
		return(CVDLS_MEM_FAIL);
	}
	
	/* Attach linear solver memory to integrator memory */
	lmem = cvdls_mem;
	
	return(CVDLS_SUCCESS);
}

int CVodeSetMaxOrd(void *cvode_mem, int maxord)
{
	CVodeMem cv_mem;
	int qmax_alloc;
	
	if (cvode_mem==NULL) {
		CVProcessError(NULL, CV_MEM_NULL, "CVODE", "CVodeSetMaxOrd", MSGCV_NO_MEM);
		return(CV_MEM_NULL);
	}
	
	cv_mem = (CVodeMem) cvode_mem;
	
	if (maxord <= 0) {
		CVProcessError(cv_mem, CV_ILL_INPUT, "CVODE", "CVodeSetMaxOrd", MSGCV_NEG_MAXORD);
		return(CV_ILL_INPUT);
	}
	
	/* Cannot increase maximum order beyond the value that
     was used when allocating memory */
	qmax_alloc = cv_mem->cv_qmax_alloc;
	
	if (maxord > qmax_alloc) {
		CVProcessError(cv_mem, CV_ILL_INPUT, "CVODE", "CVodeSetMaxOrd", MSGCV_BAD_MAXORD);
		return(CV_ILL_INPUT);
	}
	
	cv_mem->cv_qmax = maxord;
	
	printf("max ord = %d\n",maxord);
	
	return(CV_SUCCESS);
}

static int cvDenseInit(CVodeMem cv_mem)
{
	CVDlsMem cvdls_mem;
	
	cvdls_mem = (CVDlsMem) lmem;
	
	nje   = 0;
	nfeDQ = 0;
	nstlj = 0;
	
	/* Set Jacobian function and data, depending on jacDQ */
	if (jacDQ) {
		jac = cvDlsDenseDQJac;
		J_data = cv_mem;
	} 
	else {
		J_data = cv_user_data;
	}
	
	last_flag = CVDLS_SUCCESS;
	return(0);
}

/*
 * -----------------------------------------------------------------
 * cvDenseSetup
 * -----------------------------------------------------------------
 * This routine does the setup operations for the dense linear solver.
 * It makes a decision whether or not to call the Jacobian evaluation
 * routine based on various state variables, and if not it uses the 
 * saved copy.  In any case, it constructs the Newton matrix 
 * M = I - gamma*J, updates counters, and calls the dense LU 
 * factorization routine.
 * -----------------------------------------------------------------
 */

static int cvDenseSetup(CVodeMem cv_mem, int convfail, N_Vector ypred,
                        N_Vector fpred, booleantype *jcurPtr, 
                        N_Vector vtemp1, N_Vector vtemp2, N_Vector vtemp3)
{
	booleantype jbad, jok;
	realtype dgamma;
	long int ier;
	CVDlsMem cvdls_mem;
	int retval;
	
	cvdls_mem = (CVDlsMem) lmem;
	
	/* Use nst, gamma/gammap, and convfail to set J eval. flag jok */
	
	dgamma = ABS((cv_gamma/gammap) - ONE);
	jbad = (nst == 0) || (nst > nstlj + CVD_MSBJ) ||
	((convfail == CV_FAIL_BAD_J) && (dgamma < CVD_DGMAX)) ||
	(convfail == CV_FAIL_OTHER);
	jok = !jbad;
	
	if (jok) {
		
		/* If jok = TRUE, use saved copy of J */
		*jcurPtr = FALSE;
		DenseCopy(savedJ, d_M);
		
	} else {
		
		/* If jok = FALSE, call jac routine for new J value */
		nje++;
		nstlj = nst;
		*jcurPtr = TRUE;
		SetToZero(d_M);
		
		retval = jac(d_n, tn, ypred, fpred, d_M, J_data, vtemp1, vtemp2, vtemp3);
		if (retval < 0) {
			CVProcessError(cv_mem, CVDLS_JACFUNC_UNRECVR, "CVDENSE", "cvDenseSetup", MSGD_JACFUNC_FAILED);
			last_flag = CVDLS_JACFUNC_UNRECVR;
			return(-1);
		}
		if (retval > 0) {
			last_flag = CVDLS_JACFUNC_RECVR;
			return(1);
		}
		
		DenseCopy(d_M, savedJ);
		
	}
	
	/* Scale and add I to get M = I - gamma*J */
	DenseScale(-cv_gamma, d_M);
	AddIdentity(d_M);
	
	/* Do LU factorization of M */
	ier = DenseGETRF(d_M, pivots); 
	
	/* Return 0 if the LU was complete; otherwise return 1 */
	last_flag = ier;
	if (ier > 0) return(1);
	return(0);
}

/*
 * -----------------------------------------------------------------
 * cvDenseSolve
 * -----------------------------------------------------------------
 * This routine handles the solve operation for the dense linear solver
 * by calling the dense backsolve routine.  The returned value is 0.
 * -----------------------------------------------------------------
 */

static int cvDenseSolve(CVodeMem cv_mem, N_Vector b, N_Vector weight,
                        N_Vector ycur, N_Vector fcur)
{
	CVDlsMem cvdls_mem;
	realtype *bd;
	
	cvdls_mem = (CVDlsMem) lmem;
	
	bd = N_VGetArrayPointer(b);
	
	DenseGETRS(d_M, pivots, bd);
	
	/* If CV_BDF, scale the correction to account for change in gamma */
	if ((lmm == CV_BDF) && (gamrat != ONE)) {
		N_VScale(TWO/(ONE + gamrat), b, b);
	}
	
	last_flag = CVDLS_SUCCESS;
	return(0);
}

/*
 * -----------------------------------------------------------------
 * cvDenseFree
 * -----------------------------------------------------------------
 * This routine frees memory specific to the dense linear solver.
 * -----------------------------------------------------------------
 */

static void cvDenseFree(CVodeMem cv_mem)
{
	CVDlsMem  cvdls_mem;
	
	cvdls_mem = (CVDlsMem) lmem;
	
	DestroyMat(d_M);
	DestroyMat(savedJ);
	DestroyArray(pivots);
	free(cvdls_mem); cvdls_mem = NULL;
}

DlsMat NewDenseMat(int M, int N)
{
	DlsMat A;
	int j;
	
	if ( (M <= 0) || (N <= 0) ) return(NULL);
	
	A = NULL;
	A = (DlsMat) malloc(sizeof *A);
	if (A==NULL) return (NULL);
	
	A->data = (realtype *) malloc(M * N * sizeof(realtype));
	if (A->data == NULL) {
		free(A); A = NULL;
		return(NULL);
	}
	A->cols = (realtype **) malloc(N * sizeof(realtype *));
	if (A->cols == NULL) {
		free(A->data); A->data = NULL;
		free(A); A = NULL;
		return(NULL);
	}
	
	for (j=0; j < N; j++) A->cols[j] = A->data + j * M;
	
	A->M = M;
	A->N = N;
	A->ldim = M;
	A->ldata = M*N;
	
	A->type = SUNDIALS_DENSE;
	
	return(A);
}

realtype **newDenseMat(int m, int n)
{
	int j;
	realtype **a;
	
	if ( (n <= 0) || (m <= 0) ) return(NULL);
	
	a = NULL;
	a = (realtype **) malloc(n * sizeof(realtype *));
	if (a == NULL) return(NULL);
	
	a[0] = NULL;
	a[0] = (realtype *) malloc(m * n * sizeof(realtype));
	if (a[0] == NULL) {
		free(a); a = NULL;
		return(NULL);
	}
	
	for (j=1; j < n; j++) a[j] = a[0] + j * m;
	
	return(a);
}


DlsMat NewBandMat(int N, int mu, int ml, int smu)
{
	DlsMat A;
	int j, colSize;
	
	if (N <= 0) return(NULL);
	
	A = NULL;
	A = (DlsMat) malloc(sizeof *A);
	if (A == NULL) return (NULL);
	
	colSize = smu + ml + 1;
	A->data = NULL;
	A->data = (realtype *) malloc(N * colSize * sizeof(realtype));
	if (A->data == NULL) {
		free(A); A = NULL;
		return(NULL);
	}
	
	A->cols = NULL;
	A->cols = (realtype **) malloc(N * sizeof(realtype *));
	if (A->cols == NULL) {
		free(A->data);
		free(A); A = NULL;
		return(NULL);
	}
	
	for (j=0; j < N; j++) A->cols[j] = A->data + j * colSize;
	
	A->M = N;
	A->N = N;
	A->mu = mu;
	A->ml = ml;
	A->s_mu = smu;
	A->ldim =  colSize;
	A->ldata = N * colSize;
	
	A->type = SUNDIALS_BAND;
	
	return(A);
}

realtype **newBandMat(int n, int smu, int ml)
{
	realtype **a;
	int j, colSize;
	
	if (n <= 0) return(NULL);
	
	a = NULL;
	a = (realtype **) malloc(n * sizeof(realtype *));
	if (a == NULL) return(NULL);
	
	colSize = smu + ml + 1;
	a[0] = NULL;
	a[0] = (realtype *) malloc(n * colSize * sizeof(realtype));
	if (a[0] == NULL) {
		free(a); a = NULL;
		return(NULL);
	}
	
	for (j=1; j < n; j++) a[j] = a[0] + j * colSize;
	
	return(a);
}

void DestroyMat(DlsMat A)
{
	free(A->data);  A->data = NULL;
	free(A->cols);
	free(A); A = NULL;
}

void destroyMat(realtype **a)
{
	free(a[0]); a[0] = NULL;
	free(a); a = NULL;
}

int *NewIntArray(int N)
{
	int *vec;
	
	if (N <= 0) return(NULL);
	
	vec = NULL;
	vec = (int *) malloc(N * sizeof(int));
	
	return(vec);
}

int *newIntArray(int n)
{
	int *v;
	
	if (n <= 0) return(NULL);
	
	v = NULL;
	v = (int *) malloc(n * sizeof(int));
	
	return(v);
}

realtype *NewRealArray(int N)
{
	realtype *vec;
	
	if (N <= 0) return(NULL);
	
	vec = NULL;
	vec = (realtype *) malloc(N * sizeof(realtype));
	
	return(vec);
}

realtype *newRealArray(int m)
{
	realtype *v;
	
	if (m <= 0) return(NULL);
	
	v = NULL;
	v = (realtype *) malloc(m * sizeof(realtype));
	
	return(v);
}

void DestroyArray(void *V)
{ 
	free(V); 
	V = NULL;
}

void destroyArray(void *v)
{
	free(v); 
	v = NULL;
}


void AddIdentity(DlsMat A)
{
	int i;
	
	switch (A->type) {
			
		case SUNDIALS_DENSE:
			for (i=0; i<A->N; i++) A->cols[i][i] += ONE;
			break;
			
		case SUNDIALS_BAND:
			for (i=0; i<A->M; i++) A->cols[i][A->s_mu] += ONE;
			break;
			
	}
	
}


void SetToZero(DlsMat A)
{
	int i, j, colSize;
	realtype *col_j;
	
	switch (A->type) {
			
		case SUNDIALS_DENSE:
			
			for (j=0; j<A->N; j++) {
				col_j = A->cols[j];
				for (i=0; i<A->M; i++)
					col_j[i] = ZERO;
			}
			
			break;
			
		case SUNDIALS_BAND:
			
			colSize = A->mu + A->ml + 1;
			for (j=0; j<A->M; j++) {
				col_j = A->cols[j] + A->s_mu - A->mu;
				for (i=0; i<colSize; i++)
					col_j[i] = ZERO;
			}
			
			break;
			
	}
	
}


void PrintMat(DlsMat A)
{
	int i, j, start, finish;
	realtype **a;
	
	switch (A->type) {
			
		case SUNDIALS_DENSE:
			
			printf("\n");
			for (i=0; i < A->M; i++) {
				for (j=0; j < A->N; j++) {
#if defined(SUNDIALS_EXTENDED_PRECISION)
					printf("%12Lg  ", DENSE_ELEM(A,i,j));
#elif defined(SUNDIALS_DOUBLE_PRECISION)
					printf("%12lg  ", DENSE_ELEM(A,i,j));
#else
					printf("%12g  ", DENSE_ELEM(A,i,j));
#endif
				}
				printf("\n");
			}
			printf("\n");
			
			break;
			
		case SUNDIALS_BAND:
			
			a = A->cols;
			printf("\n");
			for (i=0; i < A->N; i++) {
				start = MAX(0,i-A->ml);
				finish = MIN(A->N-1,i+A->mu);
				for (j=0; j < start; j++) printf("%12s  ","");
				for (j=start; j <= finish; j++) {
#if defined(SUNDIALS_EXTENDED_PRECISION)
					printf("%12Lg  ", a[j][i-j+A->s_mu]);
#elif defined(SUNDIALS_DOUBLE_PRECISION)
					printf("%12lg  ", a[j][i-j+A->s_mu]);
#else
					printf("%12g  ", a[j][i-j+A->s_mu]);
#endif
				}
				printf("\n");
			}
			printf("\n");
			
			break;
			
	}
	
}

/*
 * -----------------------------------------------------
 * Functions working on DlsMat
 * -----------------------------------------------------
 */

int DenseGETRF(DlsMat A, int *p)
{
	return(denseGETRF(A->cols, A->M, A->N, p));
}

void DenseGETRS(DlsMat A, int *p, realtype *b)
{
	denseGETRS(A->cols, A->N, p, b);
}

int DensePOTRF(DlsMat A)
{
	return(densePOTRF(A->cols, A->M));
}

void DensePOTRS(DlsMat A, realtype *b)
{
	densePOTRS(A->cols, A->M, b);
}

int DenseGEQRF(DlsMat A, realtype *beta, realtype *wrk)
{
	return(denseGEQRF(A->cols, A->M, A->N, beta, wrk));
}

int DenseORMQR(DlsMat A, realtype *beta, realtype *vn, realtype *vm, realtype *wrk)
{
	return(denseORMQR(A->cols, A->M, A->N, beta, vn, vm, wrk));
}

void DenseCopy(DlsMat A, DlsMat B)
{
	denseCopy(A->cols, B->cols, A->M, A->N);
}

void DenseScale(realtype c, DlsMat A)
{
	denseScale(c, A->cols, A->M, A->N);
}

void denseGETRS(realtype **a, int n, int *p, realtype *b)
{
	int i, k, pk;
	realtype *col_k, tmp;
	
	/* Permute b, based on pivot information in p */
	for (k=0; k<n; k++) {
		pk = p[k];
		if(pk != k) {
			tmp = b[k];
			b[k] = b[pk];
			b[pk] = tmp;
		}
	}
	
	/* Solve Ly = b, store solution y in b */
	for (k=0; k<n-1; k++) {
		col_k = a[k];
		for (i=k+1; i<n; i++) b[i] -= col_k[i]*b[k];
	}
	
	/* Solve Ux = y, store solution x in b */
	for (k = n-1; k > 0; k--) {
		col_k = a[k];
		b[k] /= col_k[k];
		for (i=0; i<k; i++) b[i] -= col_k[i]*b[k];
	}
	b[0] /= a[0][0];
	
}

/*
 * Cholesky decomposition of a symmetric positive-definite matrix
 * A = C^T*C: gaxpy version.
 * Only the lower triangle of A is accessed and it is overwritten with
 * the lower triangle of C.
 */
int densePOTRF(realtype **a, int m)
{
	realtype *a_col_j, *a_col_k;
	realtype a_diag;
	int i, j, k;
	
	for (j=0; j<m; j++) {
		
		a_col_j = a[j];
		
		if (j>0) {
			for(i=j; i<m; i++) {
				for(k=0;k<j;k++) {
					a_col_k = a[k];
					a_col_j[i] -= a_col_k[i]*a_col_k[j];
				}
			}
		}
		
		a_diag = a_col_j[j];
		if (a_diag <= ZERO) return(j);
		a_diag = RSqrt(a_diag);
		
		for(i=j; i<m; i++) a_col_j[i] /= a_diag;
		
	}
	
	return(0);
}

/*
 * Solution of Ax=b, with A s.p.d., based on the Cholesky decomposition
 * obtained with denPOTRF.; A = C*C^T, C lower triangular
 *
 */
void densePOTRS(realtype **a, int m, realtype *b)
{
	realtype *col_j, *col_i;
	int i, j;
	
	/* Solve C y = b, forward substitution - column version.
     Store solution y in b */
	for (j=0; j < m-1; j++) {
		col_j = a[j];
		b[j] /= col_j[j];
		for (i=j+1; i < m; i++)
			b[i] -= b[j]*col_j[i];
	}
	col_j = a[m-1];
	b[m-1] /= col_j[m-1];
	
	/* Solve C^T x = y, backward substitution - row version.
     Store solution x in b */
	col_j = a[m-1];
	b[m-1] /= col_j[m-1];
	for (i=m-2; i>=0; i--) {
		col_i = a[i];
		for (j=i+1; j<m; j++) 
			b[i] -= col_i[j]*b[j];
		b[i] /= col_i[i];
	}
	
}

/*
 * QR factorization of a rectangular matrix A of size m by n (m >= n)
 * using Householder reflections.
 *
 * On exit, the elements on and above the diagonal of A contain the n by n 
 * upper triangular matrix R; the elements below the diagonal, with the array beta, 
 * represent the orthogonal matrix Q as a product of elementary reflectors .
 *
 * v (of length m) must be provided as workspace.
 *
 */

int denseGEQRF(realtype **a, int m, int n, realtype *beta, realtype *v)
{
	realtype ajj, s, mu, v1, v1_2;
	realtype *col_j, *col_k;
	int i, j, k;
	
	/* For each column...*/
	for(j=0; j<n; j++) {
		
		col_j = a[j];
		
		ajj = col_j[j];
		
		/* Compute the j-th Householder vector (of length m-j) */
		v[0] = ONE;
		s = ZERO;
		for(i=1; i<m-j; i++) {
			v[i] = col_j[i+j];
			s += v[i]*v[i];
		}
		
		if(s != ZERO) {
			mu = RSqrt(ajj*ajj+s);
			v1 = (ajj <= ZERO) ? ajj-mu : -s/(ajj+mu);
			v1_2 = v1*v1;
			beta[j] = TWO * v1_2 / (s + v1_2);
			for(i=1; i<m-j; i++) v[i] /= v1;
		} else {
			beta[j] = ZERO;      
		}
		
		/* Update upper triangle of A (load R) */
		for(k=j; k<n; k++) {
			col_k = a[k];
			s = ZERO;
			for(i=0; i<m-j; i++) s += col_k[i+j]*v[i];
			s *= beta[j];
			for(i=0; i<m-j; i++) col_k[i+j] -= s*v[i];
		}
		
		/* Update A (load Householder vector) */
		if(j<m-1) {
			for(i=1; i<m-j; i++) col_j[i+j] = v[i];
		}
		
	}
	
	
	return(0);
}

/*
 * Computes vm = Q * vn, where the orthogonal matrix Q is stored as
 * elementary reflectors in the m by n matrix A and in the vector beta.
 * (NOTE: It is assumed that an QR factorization has been previously 
 * computed with denGEQRF).
 *
 * vn (IN) has length n, vm (OUT) has length m, and it's assumed that m >= n.
 *
 * v (of length m) must be provided as workspace.
 */
int denseORMQR(realtype **a, int m, int n, realtype *beta,
               realtype *vn, realtype *vm, realtype *v)
{
	realtype *col_j, s;
	int i, j;
	
	/* Initialize vm */
	for(i=0; i<n; i++) vm[i] = vn[i];
	for(i=n; i<m; i++) vm[i] = ZERO;
	
	/* Accumulate (backwards) corrections into vm */
	for(j=n-1; j>=0; j--) {
		
		col_j = a[j];
		
		v[0] = ONE;
		s = vm[j];
		for(i=1; i<m-j; i++) {
			v[i] = col_j[i+j];
			s += v[i]*vm[i+j];
		}
		s *= beta[j];
		
		for(i=0; i<m-j; i++) vm[i+j] -= s * v[i];
		
	}
	
	return(0);
}

void denseCopy(realtype **a, realtype **b, int m, int n)
{
	int i, j;
	realtype *a_col_j, *b_col_j;
	
	for (j=0; j < n; j++) {
		a_col_j = a[j];
		b_col_j = b[j];
		for (i=0; i < m; i++)
			b_col_j[i] = a_col_j[i];
	}
	
}

void denseScale(realtype c, realtype **a, int m, int n)
{
	int i, j;
	realtype *col_j;
	
	for (j=0; j < n; j++) {
		col_j = a[j];
		for (i=0; i < m; i++)
			col_j[i] *= c;
	}
}

void denseAddIdentity(realtype **a, int n)
{
	int i;
	
	for (i=0; i < n; i++) a[i][i] += ONE;
}

int cvDlsDenseDQJac(int N, realtype t, N_Vector y, N_Vector fy, DlsMat Jac, void *data, N_Vector tmp1, N_Vector tmp2, N_Vector tmp3)
{
	realtype fnorm, minInc, inc, inc_inv, yjsaved, srur;
	realtype *tmp2_data, *y_data, *ewt_data;
	N_Vector ftemp0;
	N_Vector jthCol = NULL;
	int j;
	int retval = 0;
	
	CVodeMem cv_mem;
	CVDlsMem cvdls_mem;
	
	/* data points to cvode_mem */
	cv_mem = (CVodeMem) data;
	cvdls_mem = (CVDlsMem) lmem;
	
	/* Save pointer to the array in tmp2 */
	tmp2_data = N_VGetArrayPointer(tmp2);
	
	/* Rename work vectors for readibility */
	ftemp0 = tmp1; 
	jthCol = tmp2;
	
	/* Obtain pointers to the data for ewt, y */
	ewt_data = N_VGetArrayPointer(ewt);
	y_data   = N_VGetArrayPointer(y);
	
	/* Set minimum increment based on uround and norm of f */
	srur = RSqrt(uround);
	fnorm = N_VWrmsNorm(fy, ewt);
	minInc = (fnorm != ZERO) ?
	(MIN_INC_MULT * ABS(h) * uround * N * fnorm) : ONE;
	
	for (j = 0; j < N; j++) {
		
		/* Generate the jth col of J(tn,y) */
		
		N_VSetArrayPointer(DENSE_COL(Jac,j), jthCol);
		
		yjsaved = y_data[j];
		inc = MAX(srur*ABS(yjsaved), minInc/ewt_data[j]);
		y_data[j] += inc;
		
		retval = f(t, y, ftemp0, cv_user_data);
		nfeDQ++;
		if (retval != 0) break;
		
		y_data[j] = yjsaved;
		
		inc_inv = ONE/inc;
		N_VLinearSum_Serial(inc_inv, ftemp0, -inc_inv, fy, jthCol);
		
		DENSE_COL(Jac,j) = N_VGetArrayPointer(jthCol);
	}
	
	/* Restore original array pointer in tmp2 */
	N_VSetArrayPointer(tmp2_data, tmp2);
	
	return(retval);
}

int denseGETRF(realtype **a, int m, int n, int *p)
{
	int i, j, k, l;
	realtype *col_j, *col_k;
	realtype temp, mult, a_kj;
	
	/* k-th elimination step number */
	for (k=0; k < n; k++) {
		
		col_k  = a[k];
		
		/* find l = pivot row number */
		l=k;
		for (i=k+1; i < m; i++)
			if (ABS(col_k[i]) > ABS(col_k[l])) l=i;
		p[k] = l;
		
		/* check for zero pivot element */
		if (col_k[l] == ZERO) return(k+1);
		
		/* swap a(k,1:n) and a(l,1:n) if necessary */    
		if ( l!= k ) {
			for (i=0; i<n; i++) {
				temp = a[i][l];
				a[i][l] = a[i][k];
				a[i][k] = temp;
			}
		}
		/* Scale the elements below the diagonal in
		 * column k by 1.0/a(k,k). After the above swap
		 * a(k,k) holds the pivot element. This scaling
		 * stores the pivot row multipliers a(i,k)/a(k,k)
		 * in a(i,k), i=k+1, ..., m-1.                      
		 */
		mult = ONE/col_k[k];
		for(i=k+1; i < m; i++) col_k[i] *= mult;
		
		/* row_i = row_i - [a(i,k)/a(k,k)] row_k, i=k+1, ..., m-1 */
		/* row k is the pivot row after swapping with row l.      */
		/* The computation is done one column at a time,          */
		/* column j=k+1, ..., n-1.                                */
		
		for (j=k+1; j < n; j++) {
			
			col_j = a[j];
			a_kj = col_j[k];
			
			/* a(i,j) = a(i,j) - [a(i,k)/a(k,k)]*a(k,j)  */
			/* a_kj = a(k,j), col_k[i] = - a(i,k)/a(k,k) */
			
			if (a_kj != ZERO) {
				for (i=k+1; i < m; i++)
					col_j[i] -= a_kj * col_k[i];
			}
		}
	}
	
	/* return 0 to indicate success */
	
	return(0);
}

/* 
 * CVodeSetUserData
 *
 * Specifies the user data pointer for f
 */

int CVodeSetUserData(void *cvode_mem, void *user_data)
{
	CVodeMem cv_mem;
	
	if (cvode_mem==NULL) {
		CVProcessError(NULL, CV_MEM_NULL, "CVODE", "CVodeSetUserData", MSGCV_NO_MEM);
		return(CV_MEM_NULL);
	}
	
	cv_mem = (CVodeMem) cvode_mem;
	
	cv_user_data = user_data;
	
	return(CV_SUCCESS);
}

int CVodeSetMaxNumSteps(void *cvode_mem, long int mxsteps)
{
	CVodeMem cv_mem;
	
	if (cvode_mem==NULL) {
		CVProcessError(NULL, CV_MEM_NULL, "CVODE", "CVodeSetMaxNumSteps", MSGCV_NO_MEM);
		return(CV_MEM_NULL);
	}
	
	cv_mem = (CVodeMem) cvode_mem;
	
	/* Passing mxsteps=0 sets the default. Passing mxsteps<0 disables the test. */
	
	if (mxsteps == 0)
		cv_mem->cv_mxstep = MXSTEP_DEFAULT;
	else
		cv_mem->cv_mxstep = mxsteps;
	
	return(CV_SUCCESS);
}


void coherence_options_dump (coherence_OP *coherence_options, FILE *pfile)
  {
  fprintf (stderr, "coherence_options_dump:\n") ;
	fprintf (stderr, "coherence_options->T                         = %e [K]\n",  coherence_options->T) ;
	fprintf (stderr, "coherence_options->relaxation                = %e [s]\n",  coherence_options->relaxation) ;
	fprintf (stderr, "coherence_options->time_step                 = %e [s]\n",  coherence_options->time_step) ;
	fprintf (stderr, "coherence_options->duration                  = %e [s]\n",  coherence_options->duration) ;
	fprintf (stderr, "coherence_options->clock_high                = %e [J]\n",  coherence_options->clock_high) ;
	fprintf (stderr, "coherence_options->clock_low                 = %e [J]\n",  coherence_options->clock_low) ;
	fprintf (stderr, "coherence_options->clock_shift               = %e [J]\n",  coherence_options->clock_shift) ;
	fprintf (stderr, "coherence_options->clock_amplitude_factor    = %e\n",      coherence_options->clock_amplitude_factor) ;
	fprintf (stderr, "coherence_options->radius_of_effect          = %e [nm]\n", coherence_options->radius_of_effect) ;
	fprintf (stderr, "coherence_options->epsilonR                  = %e\n",      coherence_options->epsilonR) ;
	fprintf (stderr, "coherence_options->layer_separation          = %e [nm]\n", coherence_options->layer_separation) ;
	fprintf (stderr, "coherence_options->algorithm                 = %d\n",      coherence_options->algorithm) ;
	fprintf (stderr, "coherence_options->randomize_cells           = %s\n",      coherence_options->randomize_cells ? "TRUE" : "FALSE") ;
	fprintf (stderr, "coherence_options->animate_simulation        = %s\n",      coherence_options->animate_simulation ? "TRUE" : "FALSE") ;
// Added by Marco
	fprintf (stderr, "coherence_options->jitter_phase_0            = %f degrees\n",      coherence_options->jitter_phase_0) ;
	fprintf (stderr, "coherence_options->jitter_phase_1            = %f degrees\n",      coherence_options->jitter_phase_1) ;
	fprintf (stderr, "coherence_options->jitter_phase_2            = %f degrees\n",      coherence_options->jitter_phase_2) ;
	fprintf (stderr, "coherence_options->jitter_phase_3            = %f degrees\n",      coherence_options->jitter_phase_3) ;
// End added by Marco
//Added by Faizal
	fprintf (stderr, "coherence_options->wave_number_kx            = %lf [1/nm]\n",      coherence_options->wave_number_kx) ;
	fprintf (stderr, "coherence_options->wave_number_ky            = %lf [1/nm]\n",      coherence_options->wave_number_ky) ;
	fprintf (stderr, "coherence_options->clocking                  = %d\n",             coherence_options->clocking) ;
//End added by Faizal	
  }
