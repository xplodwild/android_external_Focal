#include "filter.h"
#include <math.h>
#include <float.h>

#include "adjust.h"

lmfunc	fcn; 
static int			AllocateLMStruct( struct LMStruct *LM );
static void			FreeLMStruct	( struct LMStruct *LM );
void 			bracket( struct LMStruct	*LM );
double 			sumSquared( double *a, int n );

#define FUNCS_PER_CP getFcnPanoNperCP()  // number of functions per control point

// Call Levenberg-Marquard optimizer
#if 1
void  RunLMOptimizer( OptInfo	*o)
{
	struct 	LMStruct	LM;
	int 	iflag;
	char	*warning;
	char 	*infmsg[] = {
				"improper input parameters",
				"the relative error in the sum of squares is at most tol",
				"the relative error between x and the solution is at most tol",
				"conditions for info = 1 and info = 2 both hold",
				"fvec is orthogonal to the columns of the jacobian to machine precision",
				"number of calls to fcn has reached or exceeded 200*(n+1)",
				"tol is too small. no further reduction in the sum of squares is possible",
				"tol too small. no further improvement in approximate solution x possible",
				"Interrupted"
				};
	int istrat;       // strategy
	int totalfev;     // total function evaluations
	int numconstraints;  // number of constraints imposed by control points
	int i;
	int lmInfo;
	AlignInfo	*g;	              // obtained from adjust.c

	// PrintError("RunLMOptimizer");
	
	// The method used here is a hybrid of two optimization strategies.
	// In the first strategy, fcnPano is configured to return one function per
	// control point, that function being total distance without regard for
	// direction.  In the second strategy, fcnPano is configured to return two
	// functions per control point, those functions being distance in two
	// directions, typically longitude and latitude.  The second strategy
	// converges much faster, but may be less stable with poor initial estimates.
	// So, we use the first method as long as it makes significant progress
	// (currently 5% reduction in error per iteration), and then switch to
	// the second method to rapidly polish the estimate.  Final result
	// returned to the user is that of the second method.
	//
	// Older versions of Panorama Tools used just the first strategy,
	// with error tolerances set to make it run to full convergence,
	// which often took hundreds or thousands of iterations.  The hybrid
	// approach typically converges much faster (a few tens of iterations)
	// and appears to be equally robust in testing to date.  Full convergence
	// (to am lmdif ftol of 1.0e-14) is not always achieved faster than the old
	// version.  However the convergence rate (error reduction per wall-clock
	// second) has been significantly better in all cases tested, and the final
	// accuracy has been equal or improved.
	//
	// So, in the interest of behavior that is friendlier to the user, I have
	// set an ftol convergence criterion that is looser than before, 1.0e-6
	// instead of 1.0e-14.  By this point, it is very unlikely that
	// significant reductions can be achieved by more iterating, since
	// even 10,000 more iterations would be predicted to make at most 1%
	// improvement in the total error.
	//
	// I have also made the diagnosis of too few control points more precise
	// and more obvious to the user.  The old version complained if the
	// number of control points was less than the number of parameters,
	// although in fact each normal control point contributes two independent
	// constraints (x and y) so the actual critical number is
	// 2*controlpoints >= parameters.  As a result, the old version often
	// complained even when things were fine, and never complained more loudly
	// even when things were awful.  This version does not complain
	// at all unless there are not enough actual constraints, and then it puts
	// out an error dialog that must be dismissed by the user.
	//
	//   Rik Littlefield (rj.littlefield@computer.org), May 2004.

	LM.n = o->numVars;
	
	g = GetGlobalPtr();
	numconstraints = 0;
	for(i=0; i < g->numPts; i++) {
		if (g->cpt[i].type == 0)
		     numconstraints += 2;
		else numconstraints += 1;
	}

	warning = "";
	if( numconstraints < LM.n )
	{
		char msgx[200];
		warning	= "Warning: Number of Data Points is smaller than Number of Variables to fit.\n";
		sprintf (msgx,"You have too few control points (%d) or too many parameters (%d).  Strange values may result!",o->numData,LM.n);
		PrintError(msgx);
	}
	
	totalfev = 0;
	for (istrat=1; istrat <= 2; istrat++) {

		setFcnPanoNperCP(istrat);

		LM.m = o->numData*FUNCS_PER_CP;
		if( LM.m < LM.n ) LM.m = LM.n;  // in strategy #1, fcnpano will pad fvec if needed

		fcn = o->fcn;

		if( AllocateLMStruct( &LM ) != 0 )
		{
			PrintError( "Not enough Memory" );
			return;
		}

		// Initialize optimization params

		if( o->SetVarsToX( LM.x ) != 0)
		{
			PrintError("Internal Error");
			return;
		}

		iflag 		= -100; // reset counter and initialize dialog
		fcn(LM.m, LM.n, LM.x, LM.fvec, &iflag);
		if (istrat == 2) setFcnPanoDoNotInitAvgFov();

		// infoDlg ( _initProgress, "Optimizing Variables" );

		/* Call lmdif. */
		LM.ldfjac 	= LM.m;
		LM.mode 	= 1;
		LM.nprint 	= 1; // 10
		LM.info 	= 0;
		LM.factor 	= 100.0;

		LM.ftol 	= 	1.0e-6; // used to be DBL_EPSILON; //1.0e-14;
		if (istrat == 1) {
			LM.ftol = 0.05;  // for distance-only strategy, bail out when convergence slows
		}

		lmdif(	LM.m,		LM.n,		LM.x,		LM.fvec,	LM.ftol,	LM.xtol,
				LM.gtol,	LM.maxfev,	LM.epsfcn,	LM.diag,	LM.mode,	LM.factor,
				LM.nprint,	&LM.info,	&LM.nfev,	LM.fjac,	LM.ldfjac,	LM.ipvt,
				LM.qtf,		LM.wa1,		LM.wa2,		LM.wa3,		LM.wa4);

		lmInfo = LM.info;

		// At end, one final evaluation to get errors that do not have fov stabilization applied,
		// for reporting purposes.
		
		if (istrat == 2) {
			forceFcnPanoReinitAvgFov();
			iflag = 1;
			fcn(LM.m, LM.n, LM.x, LM.fvec, &iflag);
		}
		
		o->SetXToVars( LM.x );

		iflag 		= -99; // reset counter and dispose dialog
		fcn(LM.m, LM.n, LM.x, LM.fvec, &iflag);
		// infoDlg ( _disposeProgress, "" );

		// Display solver info
		
		if(LM.info >= 8)
				LM.info = 4;
		if(LM.info < 0)
				LM.info = 8;
		totalfev += LM.nfev;

		sprintf( (char*) o->message, "# %s%d function evaluations\n# %s\n# final rms error %g units\n",
									warning, totalfev, infmsg[LM.info],
									sqrt(sumSquared(LM.fvec,LM.m)/LM.m) * sqrt((double)FUNCS_PER_CP));

		FreeLMStruct( &LM );
		
		if (lmInfo < 0) break;  // to honor user cancel in strategy 1
	}
	setFcnPanoNperCP(1); // Force back to startegy 1 for backwards compatability

}

#endif

#if 0
void  RunLMOptimizer( OptInfo	*o){
	RunBROptimizer ( o, 1.0e-9);
}
#endif
// Call Bracketing optimizer


void  RunBROptimizer ( OptInfo	*o, double minStepWidth)
{
	struct 	LMStruct	LM;
	int 	iflag;

	// PrintError("RunBROptimizer");
	LM.n = o->numVars;
	
	setFcnPanoNperCP(1);  // This optimizer does not use direction, don't waste time computing it

	if( o->numData*FUNCS_PER_CP < LM.n )
	{
		LM.m 		= LM.n;
	}
	else
	{
		LM.m 		= o->numData*FUNCS_PER_CP;
	}

	fcn = o->fcn;
		
	if( AllocateLMStruct( &LM ) != 0 )
	{
		PrintError( "Not enough Memory to allocate Data for BR-solver" );
		return;
	}
				
				
	// Initialize optimization params

	if( o->SetVarsToX( LM.x ) != 0)
	{
		PrintError("Internal Error");
		return;
	}

	iflag 		= -100; // reset counter
	fcn(LM.m, LM.n, LM.x, LM.fvec, &iflag);
		
	//infoDlg ( _initProgress, "Optimizing Params" );

	/* Call lmdif. */
	LM.ldfjac 	= LM.m;
	LM.mode 	= 1;
	LM.nprint 	= 1;
	// Set stepwidth to angle corresponding to one pixel in final pano
	LM.epsfcn	= minStepWidth; // g->pano.hfov / (double)g->pano.width; 
	
	LM.info 	= 0;
	LM.factor 	= 1.0;

#if 0		
	lmdif(	LM.m,		LM.n,		LM.x,		LM.fvec,	LM.ftol,	LM.xtol,
			LM.gtol,	LM.maxfev,	LM.epsfcn,	LM.diag,	LM.mode,	LM.factor,
			LM.nprint,	&LM.info,	&LM.nfev,	LM.fjac,	LM.ldfjac,	LM.ipvt,
			LM.qtf,		LM.wa1,		LM.wa2,		LM.wa3,		LM.wa4);

#endif

	bracket( &LM );

	o->SetXToVars( LM.x );
	iflag 		= -99; // 
	fcn(LM.m, LM.n, LM.x, LM.fvec, &iflag);
	//infoDlg ( _disposeProgress, "" );
	

	FreeLMStruct( &LM );
	
}



// Allocate Memory and set default values. n must be set!

int	AllocateLMStruct( struct LMStruct *LM )
{
	int i,k;
	

	if( LM->n <= 0 || LM->m <= 0 || LM->n > LM->m )
		return -1;
		
	LM->ftol 	= 	DBL_EPSILON;//1.0e-14;
	LM->xtol 	= 	DBL_EPSILON;//1.0e-14;
	LM->gtol 	= 	DBL_EPSILON;//1.0e-14;
	LM->epsfcn 	=  	DBL_EPSILON * 10.0;//1.0e-15;
	LM->maxfev 	=  	100 * (LM->n+1) * 100; 
	
	LM->ipvt = NULL;
	LM->x = LM->fvec = LM->diag = LM->qtf = LM->wa1 = LM->wa2 = LM->wa3 = LM->wa4 = LM->fjac = NULL;

	LM->ipvt 	= (int*) 	malloc(  LM->n * sizeof( int ));		
	LM->x 		= (double*) malloc(  LM->n * sizeof( double )); 		
	LM->fvec 	= (double*) malloc(  LM->m * sizeof( double )); 		
	LM->diag 	= (double*) malloc(  LM->n * sizeof( double )); 		
	LM->qtf 	= (double*) malloc(  LM->n * sizeof( double )); 		
	LM->wa1 	= (double*) malloc(  LM->n * sizeof( double )); 		
	LM->wa2 	= (double*) malloc(  LM->n * sizeof( double )); 		
	LM->wa3 	= (double*) malloc(  LM->n * sizeof( double )); 		
	LM->wa4 	= (double*) malloc(  LM->m * sizeof( double )); 		
	LM->fjac 	= (double*) malloc(  LM->m  * LM->n * sizeof( double ));

	if( LM->ipvt == NULL ||  LM->x    == NULL 	|| LM->fvec == NULL || LM->diag == NULL || 
		LM->qtf  == NULL ||  LM->wa1  == NULL 	|| LM->wa2  == NULL || LM->wa3  == NULL || 
		LM->wa4  == NULL ||  LM->fjac == NULL )
	{
		FreeLMStruct( LM );
		return -1;
	}


	// Initialize to zero

	for(i=0; i<LM->n; i++)
	{
		LM->x[i] = LM->diag[i] = LM->qtf[i] = LM->wa1[i] = LM->wa2[i] = LM->wa3[i] =  0.0;
		LM->ipvt[i] = 0;
	}

	for(i=0; i<LM->m; i++)
	{
		LM->fvec[i] = LM->wa4[i] = 0.0;
	}

	k = LM->m * LM->n;
	for( i=0; i<k; i++)
			LM->fjac[i] = 0.0;
	
	return 0;
}
		



	
void FreeLMStruct( struct LMStruct *LM )
{
	if(LM->x 	!= NULL) 	free( LM->x );
	if(LM->fvec != NULL) 	free( LM->fvec );
	if(LM->diag != NULL) 	free( LM->diag );
	if(LM->qtf 	!= NULL) 	free( LM->qtf );
	if(LM->wa1 	!= NULL) 	free( LM->wa1 );
	if(LM->wa2 	!= NULL) 	free( LM->wa2 );
	if(LM->wa3 	!= NULL) 	free( LM->wa3 );
	if(LM->wa4 	!= NULL) 	free( LM->wa4 );
	if(LM->fjac != NULL) 	free( LM->fjac );
	if(LM->ipvt != NULL) 	free( LM->ipvt );
}


void bracket( struct LMStruct	*LM )
{
	int iflag = 1,i;
	double eps, delta, delta_max;
	int changed, c = 1;
	
	
	fcn(LM->m, LM->n, LM->x, LM->fvec, &iflag);
	if( iflag < 0 ) return;

	// and do a print
	iflag = 0;
	fcn(LM->m, LM->n, LM->x, LM->fvec, &iflag);
	if( iflag < 0 ) return;
	iflag = 1;
	
	eps = sumSquared( LM->fvec, LM->m );
	
	// Choose delta_max to be between 1 and 2 degrees
	
	if( LM->epsfcn <= 0.0 ) return; // This is an error
	
	for( delta_max = LM->epsfcn; delta_max < 1.0; delta_max *= 2.0){}
	
	for( delta = delta_max; 
		 delta >= LM->epsfcn; 
		 delta /= 2.0  )
	{
		c = 1;
		
		// PrintError("delta = %lf", delta);
		while( c )
		{
			c = 0;
		
			for( i = 0; i < LM->n; i++ )
			{
				changed = 0;
				LM->x[i] += delta;
				fcn(LM->m, LM->n, LM->x, LM->fvec, &iflag); if( iflag < 0 ) return;
				
				if( delta == delta_max ) // search everywhere
				{
					while(  sumSquared( LM->fvec, LM->m ) < eps )
					{
						changed = 1;
						eps = sumSquared( LM->fvec, LM->m );
						LM->x[i] += delta;
						fcn(LM->m, LM->n, LM->x, LM->fvec, &iflag);if( iflag < 0 ) return;
					}
					LM->x[i] -= delta;
				}
				else // do just this one step
				{
					if( sumSquared( LM->fvec, LM->m ) < eps )
					{
						eps = sumSquared( LM->fvec, LM->m );
						changed = 1;
					}
					else
						LM->x[i] -= delta;
				}
		
				if( !changed )	// Try other direction
				{
					LM->x[i] -= delta;
					fcn(LM->m, LM->n, LM->x, LM->fvec, &iflag);if( iflag < 0 ) return;
					
					if( delta == delta_max ) // search everywhere
					{
						while(  sumSquared( LM->fvec, LM->m ) < eps )
						{
							changed = 1;
							eps = sumSquared( LM->fvec, LM->m );
							LM->x[i] -= delta;
							fcn(LM->m, LM->n, LM->x, LM->fvec, &iflag);if( iflag < 0 ) return;
						}
						LM->x[i] += delta;
					}
					else // do just this one step
					{
						if( sumSquared( LM->fvec, LM->m ) < eps )
						{
							eps = sumSquared( LM->fvec, LM->m );
							changed = 1;
						}
						else
							LM->x[i] += delta;
					}
				}
				
				if( changed ) c = 1;
		
				if (c) { // an improvement, let's see it (and give the user a chance to bail out)
					iflag = 0;
					fcn(LM->m, LM->n, LM->x, LM->fvec, &iflag);
					if( iflag < 0 ) return;
					iflag = 1;
				}
								
			}
		}
		// PrintError("%lf %ld %lf", delta, c, eps);
					iflag = 0;
					LM->fvec[0] = sqrt(eps/LM->m);
					fcn(LM->m, LM->n, LM->x, LM->fvec, &iflag);
					if( iflag < 0 ) return;
					iflag = 1;

	}
	
}
	
	
double sumSquared( double *a, int n )
{
	double result = 0.0;
	int i;
	
	for( i=0; i<n; i++ )
		result += a[i] * a[i];
		
	return result;
}
		




