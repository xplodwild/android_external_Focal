#include "filter.h"
#include "file.h"

static int lastCurTriangle = 0;

int getLastCurTriangle() {
	return lastCurTriangle;
}

// Solve equation
// a[0][0] * x[0] + a[0][1] * x[1] = b[0]
// a[1][0] * x[0] + a[1][1] * x[1] = b[1]
// return 0 if unique solution exists, else -1;

int SolveLinearEquation2( double a[2][2], double b[2], double x[2] )
{
	double detA = a[0][0] * a[1][1] - a[0][1]*a[1][0];
	
	if( detA == 0.0 )
		return -1;
		
	x[0] = (b[0]*a[1][1] - a[0][1]*b[1]) / detA;
	x[1] = (b[1]*a[0][0] - a[1][0]*b[0]) / detA;
	return 0;
}


// Check if point x is inside triangle t 
// if yes, return 0 and set c, so that
// x = T0 + c[0]*(T1-T0) + c[1]*(T2-T0)
// else return -1/+1
int PointInTriangle( double x, double y, PTTriangle *T, double c[2] )
{
	double a[2][2],b[2];
	
	
	a[0][0] = T->v[1].x - T->v[0].x;
	a[0][1] = T->v[2].x - T->v[0].x;
	a[1][0] = T->v[1].y - T->v[0].y;
	a[1][1] = T->v[2].y - T->v[0].y;
	
	b[0]	= x - T->v[0].x;
	b[1]	= y - T->v[0].y;
	
	if( SolveLinearEquation2( a, b, c ) != 0 )
		return -1;
	
	return (c[0] < 0.0 || c[1] < 0.0 || c[0] + c[1] > 1.0);

}


#if 0
int SetSourceTriangles( AlignInfo *g, int nIm, PTTriangle** t  )
{
	int i,j,nt=0;
	controlPoint *cp;
	double 	w2 	= (double) g->im[nIm].width  / 2.0 - 0.5;  // Steve's L
	double 	h2 	= (double) g->im[nIm].height / 2.0 - 0.5;

	
	*t = (PTTriangle*)malloc( g->nt  * sizeof( PTTriangle ) );
	if( *t== NULL )
	{
		PrintError("Not enough memory");
		return -1;
	}
	
	for(i=0; i<g->nt; i++)
	{
		if( g->t[i].nIm == nIm )
		{
			for(j=0; j<3; j++)
			{
				cp = &g->cpt[g->t[i].vert[j]];
				(*t)[nt].v[j].x = cp->x[0] - w2;
				(*t)[nt].v[j].y = cp->y[0] - h2;
			}
			nt++;
		}
	}
	return nt;
}
#endif


// Allocate space for triangles, and set them to
// T = s * T0 + (1-s) * T1
// Controlpoints must be sorted
int InterpolateTriangles( AlignInfo *g, int nIm, double s, PTTriangle** t  )
{
	int i,j,nt=0;
	double u = 1.0 - s;
	controlPoint *cp;
	double 	w2 	= (double) g->im[nIm].width  / 2.0 - 0.5;  // Steve's L
	double 	h2 	= (double) g->im[nIm].height / 2.0 - 0.5;
	
	*t = (PTTriangle*)malloc( g->nt  * sizeof( PTTriangle ) );
	if( *t== NULL )
	{
		PrintError("Not enough memory");
		return -1;
	}
	
	for(i=0; i<g->nt; i++)
	{
		if( g->t[i].nIm == nIm )
		{
			for(j=0; j<3; j++)
			{
				cp = &g->cpt[g->t[i].vert[j]];
				(*t)[nt].v[j].x = cp->x[0] * u + cp->x[1] * s - w2;
				(*t)[nt].v[j].y = cp->y[0] * u + cp->y[1] * s - h2;
			}
			nt++;
		}
	}
	return nt;
}

int SetSourceTriangles( AlignInfo *g, int nIm, PTTriangle** t  )
{
	return InterpolateTriangles( g, nIm, 0.0, t  );
}

int SetDestTriangles( AlignInfo *g, int nIm, PTTriangle** t  )
{
	return InterpolateTriangles( g, nIm, 1.0, t  );
}
	
	

// Put Controlpointcoordinates of image Nr nIm first

void SortControlPoints( AlignInfo *g, int nIm )
{
	int i;
	controlPoint cp;
	
	for(i=0; i<g->numPts; i++)
	{
		if( g->cpt[i].num[0] != nIm && g->cpt[i].num[1] == nIm )
		{
			memcpy( &cp, &g->cpt[i], sizeof(controlPoint));
			
			g->cpt[i].num[0] = cp.num[1];
			g->cpt[i].num[1] = cp.num[0];
			g->cpt[i].x[0] 	 = cp.x[1];
			g->cpt[i].x[1] 	 = cp.x[0];
			g->cpt[i].y[0] 	 = cp.y[1];
			g->cpt[i].y[1] 	 = cp.y[0];
		}
	}
}
			

#define OUTSIDE 10000000.0

int tmorph( double x_dest,double  y_dest, double* x_src, double* y_src, void* params )
{
	static int CurTriangle = 0;
	double c[2];
	PTTriangle *s, *td = ((PTTriangle**)params)[0] ,*ts= ((PTTriangle**)params)[1];
	int nt = *((int**)params)[2];
	
	//reset the CurTriangle if it exceeds the number of triangles passed in here
	if (CurTriangle >= nt)
		CurTriangle = 0;
	
	//iterate through all triangles and find the one that contains the destination point	
	if(  PointInTriangle( x_dest, y_dest, &td[CurTriangle], c ) != 0 )
	{
		for(CurTriangle = 0; 
			CurTriangle < nt && PointInTriangle( x_dest, y_dest, &td[CurTriangle], c ) != 0; 
			CurTriangle++) continue;
		if( CurTriangle == nt )
		{
			CurTriangle = 0;
			lastCurTriangle = CurTriangle;
			*x_src		= OUTSIDE;
			*y_src		= OUTSIDE;
			return 0;
		}
	}
	
	// At this point c contains valid coordinates
	s = &ts[CurTriangle];
	
	*x_src = s->v[0].x + c[0] * ( s->v[1].x - s->v[0].x ) + c[1] * ( s->v[2].x - s->v[0].x );
	*y_src = s->v[0].y + c[0] * ( s->v[1].y - s->v[0].y ) + c[1] * ( s->v[2].y - s->v[0].y );
	lastCurTriangle = CurTriangle;
    return 1;
}


int MorphImage( Image *src, Image *dst, PTTriangle *ts, PTTriangle *td, int nt )
{
	TrformStr  	Tr;
	fDesc		fD;
	void		*params[3];	



	// Set dest image
	// memcpy( dst, src, sizeof( Image ));
	dst->data = (unsigned char**)mymalloc((size_t)dst->dataSize);
	if(dst->data == NULL)
	{
		PrintError("Not enough memory");
		return -1;
	}
	
  memset(&Tr, 0, sizeof(TrformStr));
	Tr.mode					= _show_progress;
	Tr.interpolator			= _spline36;
	Tr.gamma				= 1.0;
    Tr.fastStep             = FAST_TRANSFORM_STEP_NONE;
	Tr.src					= src;
	Tr.dest					= dst;
	Tr.success				= 1;

	
	params[0] = (void*)td;
	params[1] = (void*)ts;
	params[2] = (void*)&nt;
	
	fD.func	  = tmorph;
	fD.param  = (void*)params;

	transForm( &Tr, &fD, 0 );
	
	if( Tr.success )
		return 0;
	else
	{
		if(dst->data)
			myfree((void**)dst->data);
		return -1;
	}
}

int MorphImageFile( fullPath *sfile, fullPath *dfile, AlignInfo *g,int nIm )
{
	PTTriangle *ts=NULL, *td=NULL;
	Image src, dst;
	int nt, result;
	double s = g->pano.cP.vertical_params[0];


	if( panoImageRead( &src, sfile ) == 0)
	{
		PrintError("Could not read image");
		return -1;
	}

	// Set dest
	
	memcpy(&dst, &src, sizeof(Image));
	dst.width	= g->pano.width;
	dst.height	= g->pano.height;
	dst.bytesPerLine = dst.width * 4;
	dst.dataSize = dst.height * dst.bytesPerLine;


	
	SortControlPoints( g, nIm );
	
	nt = SetSourceTriangles( g, nIm, &ts  );
	if( nt < 0 )	return -1;
	if( nt == 0 )	return 1; // Nothing to interpolate
	
	SortControlPoints( g, 0 );

	nt = InterpolateTriangles( g, nIm, s, &td  );
	if( nt < 0 )	return -1;
	if( nt == 0 )	return 1; // Nothing to interpolate

	
	result = MorphImage( &src, &dst, ts, td, nt );
	
	myfree( (void**)src.data );
	if(ts) free( ts );
	if(td) free( td );
	
	if( result == 0 ) // success
	{
		mycreate( dfile, '8BIM', '8BPS' );
		if( writePSD( &dst, dfile ) != 0)
		{
			PrintError("Could not write destination Image");
			result = -1;
		}
		myfree( (void**)dst.data );
	}
	
	return result;
}
	
	
	
	



int blendImages( fullPath *f0,  fullPath *f1, fullPath *result, double s )
{
	double u = 1.0 - s, r;
	uint32_t x,y,cy,i;
	Image im0, im1;
	unsigned char *c0, *c1;
	
	if( readPSD(&im0, f0, 1) != 0 )
	{
		PrintError("Error reading image file");
		return -1;
	}
	if( readPSD(&im1, f1, 1) != 0 )
	{
		PrintError("Error reading image file");
		return -1;
	}
	
	for(y=0; y<im0.height; y++)
	{
		cy = y * im0.bytesPerLine;
		for(x=0; x<im0.width; x++)
		{
			c0 = *im0.data + cy + 4 * x;
			c1 = *im1.data + cy + 4 * x;
			
			if( *c1 )
			{
				if( *c0 )
				{
					for(i=1; i<4; i++)
					{
						r = u * (double)c0[i] + s * (double)c1[i];
						DBL_TO_UC( c0[i], r );
					}
				}
				else
					memcpy( c0, c1, 4 );
			}
		}
	}

	mycreate( result, '8BIM', '8BPS' );
	if( writePSD( &im0, result ) != 0)
	{
		PrintError("Could not write destination Image");
		return -1;
	}
	myfree( (void**)im0.data );
	myfree( (void**)im1.data );
	
	return 0;
}





int InterpolateImageFile( fullPath *sfile, fullPath *dfile, AlignInfo *g,int nIm )
{
	PTTriangle *ts=NULL, *td=NULL;
	Image src, dst;
	int nt, result;
	double s = g->pano.cP.vertical_params[0];


	if( panoImageRead( &src, sfile ) == 0 )
	{
		PrintError("Could not read image");
		return -1;
	}

	// Set dest
	
	memcpy(&dst, &src, sizeof(Image));
	dst.width	= g->pano.width;
	dst.height	= g->pano.height;
	dst.bytesPerLine = dst.width * 4;
	dst.dataSize = dst.height * dst.bytesPerLine;

	
	
	SortControlPoints( g, nIm );
	
	nt = SetSourceTriangles( g, nIm, &ts  );
	if( nt < 0 )	return -1;
	if( nt == 0 )	return 1; // Nothing to interpolate


	SortControlPoints( g, 0 );
	
	nt = InterpolateTrianglesPerspective( g, nIm, s, &td  );
	if( nt < 0 )	return -1;
	if( nt == 0 )	return 1; // Nothing to interpolate

	

	result = MorphImage( &src, &dst, ts, td, nt );

	
	myfree( (void**)src.data );
	if(ts) free( ts );
	if(td) free( td );
	
	if( result == 0 ) // success
	{
		mycreate( dfile, '8BIM', '8BPS' );
		if( writePSD( &dst, dfile ) != 0)
		{
			PrintError("Could not write destination Image");
			result = -1;
		}
		myfree( (void**)dst.data );
	}
	
	return result;
}
	

// Allocate space for triangles, and set them to
// T = s * T0 + (1-s) * T1
// Controlpoints must be sorted
int InterpolateTrianglesPerspective( AlignInfo *g, int nIm, double s, PTTriangle** t  )
{
	int i,j,nt=0;
	double u = 1.0 - s;
	controlPoint *cp;
	double 	w2 	= (double) g->im[nIm].width  / 2.0 - 0.5;  // Steve's L
	double 	h2 	= (double) g->im[nIm].height / 2.0 - 0.5;
	struct 	MakeParams	m0, m1;
	fDesc 	m0_stack[15], m1_stack[15];	
	Image d0, dst;
	double x0,x1,y0,y1;

	memcpy(&d0, &g->im[nIm], sizeof(Image));
	d0.yaw 		= 0.0;
	d0.roll		= 0.0;
	d0.pitch 	= 0.0;


	SetInvMakeParams( m0_stack, &m0, &g->im[0], &d0, 0 );
	SetInvMakeParams( m1_stack, &m1, &g->im[1], &d0, 0 );

	
	*t = (PTTriangle*)malloc( g->nt  * sizeof( PTTriangle ) );
	if( *t== NULL )
	{
		PrintError("Not enough memory");
		return -1;
	}
	
	for(i=0; i<g->nt; i++)
	{
		if( g->t[i].nIm == nIm )
		{
			for(j=0; j<3; j++)
			{
				cp = &g->cpt[g->t[i].vert[j]];
				execute_stack( 	cp->x[0] - w2, cp->y[0] - h2, &x0, &y0, m0_stack );
				execute_stack( 	cp->x[1] - w2, cp->y[1] - h2, &x1, &y1, m1_stack );
				(*t)[nt].v[j].x = x0 * u + x1 * s;
				(*t)[nt].v[j].y = y0 * u + y1 * s;
			}
			nt++;
		}
	}

	memcpy(&dst, &g->im[nIm], sizeof(Image));
	dst.hfov	= g->pano.hfov;
	dst.width	= g->pano.width;
	dst.height	= g->pano.height;
	dst.bytesPerLine = dst.width * 4;
	dst.dataSize = dst.height * dst.bytesPerLine;
	dst.yaw 	= (1.0 - s) * g->im[0].yaw 		+ s * g->im[1].yaw;
	dst.pitch 	= (1.0 - s) * g->im[0].pitch 	+ s * g->im[1].pitch;
	dst.roll 	= (1.0 - s) * g->im[0].roll 	+ s * g->im[1].roll;

	SetMakeParams( m0_stack, &m0, &dst, &d0, 0 );
	for(i=0; i<nt; i++)
	{
		for(j=0; j<3; j++)
		{
			execute_stack( 	(*t)[i].v[j].x, (*t)[i].v[j].y, &x0, &y0, m0_stack );
			(*t)[i].v[j].x = x0;
			(*t)[i].v[j].y = y0;
		}
	}

	return nt;
}




