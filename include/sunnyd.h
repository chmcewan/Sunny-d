#ifndef _SUNNYD_
#define _SUNNYD_

#include "mex.h"
#include <cvode/cvode.h>
#include <nvector/nvector_serial.h>
#include <cvode/cvode_dense.h>

typedef struct
{
    mxArray* f; 
    double* Tin;    
    double* x0;
    double* P;
    mxArray* Pm;
    int n_species;
    int n_points;   

    double* Tout;
    double* Y;

} SundialsCtx;

/*
 * Forward declare. The user will implement this.
 */
void dydt(double t, double *y, double *ydot, double *p);

/*
 * Construct a matrix
 */
mxArray* Dense(int m, int n)
{
    mxArray *B;
    B = mxCreateDoubleMatrix(0, 0, mxREAL);
    mxSetM(B,m);
    mxSetN(B,n);
    mxSetData(B, mxMalloc(sizeof(double)*m*n));
    return B;
}

/*
 * Called by CVODE to compute derivative. Delegates to user function.
 */
int InvokeUserFunction(realtype t, N_Vector y, N_Vector ydot, void *user_data)
{
    SundialsCtx* ctx = (SundialsCtx*) user_data;
    double  T  = (double)t;
    double *Y  = y->ops->nvgetarraypointer(y);
	double *dY = ydot->ops->nvgetarraypointer(ydot);    
	int     N  = NV_LENGTH_S(ydot);
	int i;

	for(i=0; i<N; i++)
		dY[i] = 0.0;
    dydt(T, Y, dY, ctx->P);

    return 0;
}

/*
 * Called by CVODE to compute derivative. Delegates to matlab.
 */
int InvokeMatlab(realtype t, N_Vector y, N_Vector ydot, void *user_data)
{
    SundialsCtx* ctx = (SundialsCtx*) user_data;
    mxArray *T = Dense(1,1);
    mxArray *Y = Dense(1, NV_LENGTH_S(ydot));
    double* ydata = y->ops->nvgetarraypointer(y);
    mxArray *RHS[4] = { ctx->f, T, Y, ctx->Pm};
    mxArray *LHS[1];
    mxArray* Mydot;
    double* data;
    int i,N;
    
    /* Copy from Sundials to Matlab structures */
    memcpy(mxGetPr(T), &t, sizeof(double));
    memcpy(mxGetPr(Y), ydata, sizeof(double)*NV_LENGTH_S(ydot));
    
    /* invoke feval with given function handle */
    mexCallMATLAB(1, LHS, 4, RHS, "feval");

    /* Copy from Matlab to Sundials structures */
    Mydot = LHS[0];
    data  = mxGetPr(Mydot);
    N     = (int)((mxGetM(Mydot)==1) ? mxGetN(Mydot) : mxGetM(Mydot));
    for(i=0; i<N; i++)
        NV_Ith_S(ydot,i) = data[i];

    // TODO: T and Y not destroyed???
    mxDestroyArray(T); 
    mxDestroyArray(Y); 
    mxDestroyArray(Mydot);    
    return 0;
}

/*
 * Setup CVODE and step
 */
void InvokeSundials(SundialsCtx* ctx)
{
	// xArray *f, double* x0, int Nspecies, int Npoints, double* Tin, double* T, double* Y, double* P

	void * cvode_mem  = CVodeCreate(CV_BDF, CV_NEWTON);    
    int flag, i, j;
    realtype t;
    N_Vector y;        


	/* Initial conditions and copy x0 vector for cvode */
	ctx->Tout[0] = ctx->Tin[0];         
	y    = N_VNew_Serial(ctx->n_species); 
	for (i = 0; i < ctx->n_species; i++)
	{
		NV_Ith_S(y,i) = ctx->x0[i];
		ctx->Y[ctx->n_points*i]  = ctx->x0[i];
	}

    /* Configure CVODE */
	flag = (ctx->f == NULL) ? CVodeInit(cvode_mem, InvokeUserFunction, ctx->Tin[0], y) :
                             CVodeInit(cvode_mem, InvokeMatlab, ctx->Tin[0], y);
	flag = CVodeSStolerances(cvode_mem, 1.0E-7, 1.0E-9);
    // flag = CVodeSStolerances(cvode_mem, 1.0E-3, 1.0E-6);
	flag = CVodeSetUserData(cvode_mem, ctx);
	flag = CVDense(cvode_mem, ctx->n_species);
	// flag = CVodeSetMinStep(cvode_mem, 1.0E-9); 
	flag = CVodeSetMaxNumSteps(cvode_mem, 50000);
	flag = CVodeSetMaxHnilWarns(cvode_mem, 0);

	/* Integrate */
	t = 0.0;
	for (i = 1; i < ctx->n_points; i++)
	{        
		//mexPrintf("CVODE: call...%f\n",Tin[i]);
        flag = CVode(cvode_mem, ctx->Tin[i], y, &t, CV_NORMAL);
		if (flag != CV_SUCCESS)
		{
			// mexPrintf("sunnyd: integration error. Code %d\n", flag);
			break;
		}

		ctx->Tout[i] = t;
    	for(j=0; j<ctx->n_species; j++)
			ctx->Y[i + ctx->n_points*j] = NV_Ith_S(y, j);
	}
	N_VDestroy_Serial(y);
	CVodeFree(&cvode_mem);    
}

/*
 * Entry point for Malab. Marshal into native datastuctures.
 */
void mexFunction(int nlhs, mxArray *lhs[], int nrhs, const mxArray *rhs[])
{
    SundialsCtx ctx;

    /* Initialise with arguments */
    ctx.f   = rhs[0];
	ctx.Tin = mxGetPr(rhs[1]);    
	ctx.x0  = mxGetPr(rhs[2]);
    ctx.P   = mxGetPr(rhs[3]);
    ctx.Pm  = rhs[3];
    ctx.n_species = (mxGetM(rhs[2])==1) ? mxGetN(rhs[2]) : mxGetM(rhs[2]);
	ctx.n_points  = (mxGetM(rhs[1])==1) ? mxGetN(rhs[1]) : mxGetM(rhs[1]);

    /* Allocate return values */
    lhs[0]   = Dense(ctx.n_points, 1);
    lhs[1]   = Dense(ctx.n_points, ctx.n_species);
    ctx.Tout = mxGetPr(lhs[0]);
    ctx.Y    = mxGetPr(lhs[1]);
    
    /* If no function pointer, will be empty array */
    if( mxGetM(ctx.f) == 0 || mxGetN(ctx.f) == 0 )
        ctx.f = NULL;

    InvokeSundials(&ctx);
}


#endif