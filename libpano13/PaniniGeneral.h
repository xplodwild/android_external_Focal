/* PaniniGeneral.h		15Jan2010 TKS

Copyright (c) 2010, Thomas K Sharpless
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


This is the reference implementation of the General Pannini
Projection, an elaboration of the basic Pannini projection 
discovered by Bruno Postle and Thomas Sharpless in December 
2008 in paintings by Gian Paolo Pannini (1691-1765).

It is a parameterized mapping between sphere and plane, 
that gives synthetic perspective views on the plane when
the sphere holds a linear projection of the scene.  Sphere 
coordinates (phi, theta) are equirectangular: longitude and 
latitude angles, in radians, relative to a point on the equator.
Plane coordinates (h, v) are relative to the image of the same
point, typically but not necessarily the center point of
the view.  The plane y coordinate is negative upward, as is
typical in image processing software.

There are 3 parameters:
  d [0:infinity) controls horizontal compression
  t [-1:1] controls vertical compression at top
  b [-1:1] controls vertical compression at bottom

There are functions to map cooridnates in either direction
and one to compute the maximum feasible field of view of the
plane image, given a d value and the projection angle limit
of your display system.  

Angles passed to and returned by panini_general_maxVAs() 
are max view angles (half-FOVs) in radians.

All 3 functions return an integer: 0: failure, 1: OK.
Computed coordinates and FOVs are returned in arguments
passed by address.

*/
int panini_general_toPlane	( double phi, double theta, 
							  double* h,  double* v, 
							  double d, double t, double b
							 );
int panini_general_toSphere	( double* phi, double* theta, 
							  double  h,  double  v, 
							  double d, double t, double b
							 );
int panini_general_maxVAs	( double d, 
							  double maxProj,
							  double * maxView
							 );
