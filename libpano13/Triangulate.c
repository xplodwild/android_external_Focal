/* Panorama_Tools	-	Generate, Edit and Convert Panoramic Images
   Copyright (C) 1998,1999 - Helmut Dersch  der@fh-furtwangen.de
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/*------------------------------------------------------------*/

#include "filter.h"
#include <math.h>


// This is an implementation of the flipping algorithm. It starts with an arbitrary 
// triangulation and converts it into the Delaunay triangulation by flipping the diagonals
// of two neighbouring triangles. Two neighbouring triangles t1=(p0,p1,p2) and t2=(p3,p2,p1) 
// are flipped to t1'=(p0,p1,p3) and t2'=(p2,p0,p3), if vertex p3 lies in the circumcircle 
// of t1 (respectively p0 lies in the circumcircle of t2).
// see R.Sibson. Locally equiangular triangulations. The Computer Journal Vol.2 (3): 243-245, 1973. 

int debug = 0;

// One iteration; return number of changed triangle pairs
int DelaunayIteration( AlignInfo *g, int nIm )
{
	int i,j,k,result = 0;
	
	for(i=0; i<g->nt; i++)
	{
		if( g->t[i].nIm == nIm )
		{
			for(j=i+1; j<g->nt; j++)
			{
				if( g->t[j].nIm == nIm )
				{
					// Do these triangles have one side in common, ie
					// are of type (n0,n1,n2) and (n1,n2,n3)
					int n0,n1,n2,n3;
					
					n0 = n1 = n2 = n3 =-1;
					
					for(k=0; k<3; k++)
					{
						if( g->t[i].vert[k] == g->t[j].vert[0] ||
							g->t[i].vert[k] == g->t[j].vert[1] ||
							g->t[i].vert[k] == g->t[j].vert[2] )
							{
								if( n1 == -1 )
									n1 = g->t[i].vert[k];
								else
									n2 = g->t[i].vert[k];
							}
					}

					if(n1!=-1 && n2!=-1) // Found adjacent sides
					{
						triangle 	t0, t1;
						PTTriangle 	tC0, tC1;
						PTLine		s0, s1;
						
						for(k=0; k<3; k++)
						{
							if( g->t[i].vert[k] != n1 && g->t[i].vert[k] != n2 )
								n0 = g->t[i].vert[k];
							if( g->t[j].vert[k] != n1 && g->t[j].vert[k] != n2 )
								n3 = g->t[j].vert[k];
						}
//						printf("t1: %d %d %d		t2: %d %d %d\n", g->t[i].vert[0], g->t[i].vert[1], g->t[i].vert[2], g->t[j].vert[0], g->t[j].vert[1], g->t[j].vert[2]);
						if( n0 == -1 || n3==-1 || n0==n3) continue;
						t0.vert[0] = n0; t0.vert[1] = n1; t0.vert[2] = n2; t0.nIm = nIm;
						t1.vert[0] = n1; t1.vert[1] = n2; t1.vert[2] = n3; t1.nIm = nIm;
						
						SetTriangleCoordinates( &t0, &tC0, g );
						SetTriangleCoordinates( &t1, &tC1, g );
						
						CopyPTPoint( s0.v[0], tC0.v[1]);
						CopyPTPoint( s0.v[1], tC0.v[2]);
						CopyPTPoint( s1.v[0], tC0.v[0]);
						CopyPTPoint( s1.v[1], tC1.v[2]);
												
						if( PTDistance( &tC0.v[1], &tC0.v[2]) > PTDistance( &tC0.v[0], &tC1.v[2] ) &&
							LinesIntersect( &s0, &s1 ) )
						{
							t0.vert[0] = n0; t0.vert[1] = n1; t0.vert[2] = n3; t0.nIm = nIm;
							t1.vert[0] = n0; t1.vert[1] = n2; t1.vert[2] = n3; t1.nIm = nIm;
							memcpy( &g->t[i], &t0, sizeof( triangle ));
							memcpy( &g->t[j], &t1, sizeof( triangle ));
							OrderVerticesInTriangle( i, g );
							OrderVerticesInTriangle( j, g );
//						printf("New: t1: %d %d %d		t2: %d %d %d\n", g->t[i].vert[0], g->t[i].vert[1], g->t[i].vert[2], g->t[j].vert[0], g->t[j].vert[1], g->t[j].vert[2]);
							result++;
						}
					}
				}
			}
		}
	}
	return result;
}

							
int ReduceTriangles( AlignInfo *g, int nIm )
{
	int numIt = 0;
		
	while( DelaunayIteration( g, nIm ) && numIt++ < 100 )
		continue;
		//printf( "nit = %d\n", numIt );
	return 0;
}
								

int TriangulatePoints( AlignInfo *g, int nIm )
{
	int i,j,k,m;
	PTTriangle tC;
	triangle t;
	double c[2];
	
	SortControlPoints( g, nIm );

	// Remove all triangles in image nIm
	
	for(i=0; i<g->nt; i++)
	{
		if( g->t[i].nIm == nIm )
		{
			RemoveTriangle( i, g );
			i--;
		}
	}

	// Create Starttriangulation
	
	for(i=0; i<g->numPts; i++)
	{
		if( g->cpt[i].num[0] == nIm )
		{
			tC.v[0].x = g->cpt[i].x[0]; tC.v[0].y = g->cpt[i].y[0];

			for( j=i+1; j< g->numPts; j++ )
			{
				if( g->cpt[j].num[0] == nIm )
				{
					tC.v[1].x = g->cpt[j].x[0]; tC.v[1].y = g->cpt[j].y[0];
					
					for( k=j+1; k< g->numPts; k++ )
					{
						if( g->cpt[k].num[0] == nIm )
						{
							tC.v[2].x = g->cpt[k].x[0]; tC.v[2].y = g->cpt[k].y[0];
							
							if( PTAreaOfTriangle( &tC ) == 0.0 )
								goto IsNotEmpty;

							
							
							for( m=0;m<g->numPts; m++)
							{
								if( g->cpt[m].num[0] == nIm && m!=i && m!=j && m!=k )
								{
									if( PointInTriangle( g->cpt[m].x[0], g->cpt[m].y[0], &tC, c ) == 0 )
										goto IsNotEmpty;
								}
							}
							
							for( m=0;m<g->nt; m++)
							{
								if(g->t[m].nIm == nIm)
								{
									PTTriangle tcp;
									
									SetTriangleCoordinates( &g->t[m], &tcp, g );
								
									if( TrianglesOverlap( &tcp, &tC ) != 0 )
									{
										goto IsNotEmpty;
									}
								}
							}
							
							{
								t.vert[0] = i; t.vert[1] = j; t.vert[2] = k; t.nIm	  = nIm;
								if( AddTriangle( &t, g ) == -1)
								{
									PrintError("Could not add triangle");
									return -1;
								}
							}
							IsNotEmpty:
								continue;
						}
					}
				}
			}
		}
	}

	ReduceTriangles( g,  nIm );

	return 0;

}	
	

int AddTriangle( triangle *t, AlignInfo *g )
{
	void * tmp;
	
	
	tmp =  realloc( g->t, (g->nt+1) * sizeof( triangle ) );
	if( tmp == NULL )
	{
		return -1;
	}
	g->nt++; g->t = (triangle*)tmp; 
	
	memcpy( &g->t[g->nt-1], t, sizeof( triangle ));
	
	
	return g->nt-1;
}

int RemoveTriangle( int nt, AlignInfo *g )
{
	int i;

	
	if( nt >= g->nt )
		return -1;
	for(i=nt; i<g->nt-1; i++)
	{
		memcpy( &g->t[i], &g->t[i+1], sizeof( triangle ));
	}
	g->t = (triangle*)realloc( g->t, (g->nt-1) * sizeof( triangle ));
	g->nt--;
	return g->nt;
}

#define VERT_X(index) ((double)(g->cpt[g->t[nt].vert[index]].x[(g->cpt[g->t[nt].vert[index]].num[0] == g->t[nt].nIm?0:1)]))
#define VERT_Y(index) ((double)(g->cpt[g->t[nt].vert[index]].y[(g->cpt[g->t[nt].vert[index]].num[0] == g->t[nt].nIm?0:1)]))

void OrderVerticesInTriangle( int nt, AlignInfo *g )
{
	double v1[2], v2[2];
	
	v1[0] = VERT_X(0) - VERT_X(1) ;
	v2[0] = VERT_X(0) - VERT_X(2) ;
	v1[1] = VERT_Y(0) - VERT_Y(1) ;
	v2[1] = VERT_Y(0) - VERT_Y(2) ;
	
	if( v1[0]*v2[1] - v1[1]*v2[0]  > 0.0 )
	{
		int v = g->t[nt].vert[1];
		
		g->t[nt].vert[1] = g->t[nt].vert[2];
		g->t[nt].vert[2] = v;
	}
}

void SetTriangleCoordinates( triangle *t, PTTriangle *tC, AlignInfo *g )
{
	int i;
	
	for(i=0; i<3; i++)
	{
		if( g->cpt[t->vert[i]].num[0] == t->nIm )
		{
			tC->v[i].x = g->cpt[t->vert[i]].x[0];			
			tC->v[i].y = g->cpt[t->vert[i]].y[0];
		}
		else
		{
			tC->v[i].x = g->cpt[t->vert[i]].x[1];			
			tC->v[i].y = g->cpt[t->vert[i]].y[1];
		}
	}
}



	

// return 0 if there is no overlap

int TrianglesOverlap( PTTriangle *t0, PTTriangle *t1 )
{
	int i,j,k,m;
	PTLine s0, s1;
	
	for(i=0; i<3; i++)
	{
		k = i+1;
		if(k==3) k=0;
		
		CopyPTPoint( s0.v[0], t0->v[i] );
		CopyPTPoint( s0.v[1], t0->v[k] );
		
		for( j=0; j<3; j++)
		{
			m = j+1;
			if(m==3)m=0;
			
			CopyPTPoint( s1.v[0], t1->v[j] );
			CopyPTPoint( s1.v[1], t1->v[m] );

			if( LinesIntersect( &s0, &s1)  ) 
				return 1;
		}
	}
	
	return 0;
}

// return 0 if there is no intersection between the endpoints
int LinesIntersect( PTLine *s0, PTLine *s1) 
{
	PTPoint ps;
	
	if( SamePTPoint(  s0->v[0], s1->v[0] ) ||
		SamePTPoint(  s0->v[0], s1->v[1] ) ||
		SamePTPoint(  s0->v[1], s1->v[0] ) ||
		SamePTPoint(  s0->v[1], s1->v[1] ) )
		return 0;
	
	if( PTGetLineCrossing( s0, s1, &ps ) != 0 )
		return 0;
		

	return( PTPointInRectangle(  &ps, s0 ) &&
			PTPointInRectangle(  &ps, s1 ) );
}

double PTDistance( PTPoint *s0, PTPoint *s1 )
{
	return sqrt( (s0->x - s1->x) * (s0->x - s1->x) + (s0->y - s1->y) * (s0->y - s1->y) );
}

// Rectangle width diagonal line r
// return 1 if point inside rectangle

int PTPointInRectangle(  PTPoint *p, PTLine *r )
{
	return( PTElementOf( p->x, r->v[0].x, r->v[1].x) && PTElementOf( p->y, r->v[0].y, r->v[1].y) );
}
	

#define PT_EPS 1.e-8
// return 1 if x inside interval a,b

int PTElementOf(  double x, double a, double b )
{
	double c;
	
	if( b > a ) 
	{
		c = a; a = b; b = c;
	}
	
	return ( x > b - PT_EPS && x < a + PT_EPS );
}



// Get normal representation of line 
// ax + by + c = 0

int PTNormal( double *a, double *b, double *c, PTLine *s )
{
	if( s->v[0].x == s->v[1].x )
	{
		if( s->v[0].y == s->v[1].y ) // Points are identical
			return -1;
		else
		{
			*a = -1.0;
			*b = 0.0;
			*c = s->v[0].x;
			return 0;
		}
	}
	else
	{
		*a = (s->v[0].y - s->v[1].y) / (s->v[0].x - s->v[1].x);
		*b = -1.0;
		*c = s->v[0].y - s->v[0].x * (*a); 
		return 0;
	}
}

int PTGetLineCrossing( PTLine *s0, PTLine *s1, PTPoint *ps )
{
	double A[2],B[2],C[2], D;
	
	if( PTNormal( &A[0], &B[0], &C[0], s0 ) != 0  ||
		PTNormal( &A[1], &B[1], &C[1], s1 ) != 0 )
		return -1;
		
	D = A[0]*B[1] - A[1]*B[0];
	
	if( D == 0.0 ) 	// parallele Geraden
		return -1;
	
	ps->x = (B[0]*C[1] - B[1]*C[0]) / D;
	ps->y = (C[0]*A[1] - C[1]*A[0]) / D;
	return 0;
}

double PTAreaOfTriangle( PTTriangle *t )
{
	double A;
	
	A = ((t->v[2].x - t->v[0].x) * (t->v[1].y - t->v[0].y) - 
	  	 (t->v[2].y - t->v[0].y) * (t->v[1].x - t->v[0].x))/2.0; 
	// use fabs rather than abs as precision is lost with abs on 64bit platforms
	return fabs( A );	
}


int normalToTriangle( CoordInfo *n, CoordInfo *v, triangle *t )
{
	double v1[3], v2[3], a;
	int i;
	
	for(i=0; i<3; i++)
	{
		v1[i] = v[t->vert[0]].x[i] - v[t->vert[1]].x[i];
		v2[i] = v[t->vert[0]].x[i] - v[t->vert[2]].x[i];
	}
	
	n->x[0] = v1[1]*v2[2] - v1[2]*v2[1];
	n->x[1] = v1[2]*v2[0] - v1[0]*v2[2];
	n->x[2] = v1[0]*v2[1] - v1[1]*v2[0];
	
	a = sqrt( n->x[0]*n->x[0] + n->x[1]*n->x[1] + n->x[2]*n->x[2] );
	
	if( a!=0.0 )
	{
		n->x[0] /= a;
		n->x[1] /= a;
		n->x[2] /= a;
		return 0;
	}
	else
	{
		return -1;
	}
}
		


