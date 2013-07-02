/* Panorama_Tools - Generate, Edit and Convert Panoramic Images
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


  

void remap(TrformStr *TrPtr, rPrefs *r_prefs)
{
  fDesc fD;                 // remapping function
  double  vars[3];          // variables required for calculation
                              // vars[0] = distance d
                              // vars[1] 
                              // vars[2] 
  double  a,b;              // horizontal/vertical field of view in 'rad'
  int   destwidth=0, destheight=0;

  
  fD.param  = (void*) vars;
  fD.func   = (trfn)NULL;
  
  
  // Check for inconsistent values (should be extended!)
  
  if( r_prefs->hfov <= 0.0  )
  {
    TrPtr->success = 0;
    PrintError("Parameter Error");
    return;
  }

  
  // Set destination image parameters and parameters for remapping
  
  a = DEG_TO_RAD( r_prefs->hfov );
  
  switch(r_prefs->from)
  {
    case _rectilinear:  
      if( a >= PI )
      {
        TrPtr->success = 0;
        PrintError("Wrong FOV: Must be smaller than 180 degrees");
        return;
      }
      vars[0] = TrPtr->src->width / ( 2.0 * tan( a/2.0 ) );
      switch(r_prefs->to)
      {
        case _rectilinear:  TrPtr->success = 0;
              PrintError("Same Mapping!");
              return;
              break;
        case _panorama   :  
              destheight = TrPtr->src->height;
              destwidth = (int)(a * vars[0] + 0.5);
              fD.func = rect_pano;
              break;
        case _equirectangular:
              destheight  = (int)(2.0 * vars[0] * atan( TrPtr->src->height/(2*vars[0]) ) + 0.5);
              destwidth = (int)(a * vars[0] + 0.5);
              fD.func = rect_erect;
              break;
        case _spherical_cp:TrPtr->success = 0;
              PrintError("Use Fisheye Horizontal and Perspective");
              return;
              break;
        case _spherical_tp:
              destheight  = (int)(2.0 * vars[0] * atan( TrPtr->src->height/(2*vars[0]) ) + 0.5);
              destwidth   =  (int)(a * vars[0] + 0.5);
              fD.func = rect_sphere_tp;
              break;
        case _mirror:TrPtr->success = 0;
              PrintError("Sorry, not yet available");
              return;
              break;
        default: break;
      }
      break;
    case _panorama:
      vars[0] = TrPtr->src->width / a;
      switch(r_prefs->to)
      {
        case _rectilinear: 
              if( a >= PI )
              {
                TrPtr->success = 0;
                PrintError("Wrong FOV: Must be smaller than 180 degrees");
                return;
              }
              destheight = TrPtr->src->height;
              destwidth = (int)(2.0 * tan( a/2.0 ) * vars[0] + 0.5);
              fD.func = pano_rect;
              break;
        case _panorama   :  TrPtr->success = 0;
              PrintError("Same Mapping!");
              return;
              break;
        case _equirectangular:
              destheight =  (int)(2 * vars[0] * atan( TrPtr->src->height/ (2*vars[0]) ));
              destwidth = TrPtr->src->width;
              fD.func = pano_erect;
              break;
        case _spherical_cp:
              TrPtr->success = 0;
              PrintError("Sorry, not yet available");
              return;
              break;
        case _spherical_tp:             
              destheight =  TrPtr->src->width ;//2 * vars[0] * atan( TrPtr->src->height/ (2*vars[0]) );
              destwidth = TrPtr->src->width;
              fD.func = pano_sphere_tp;
              break;
        case _mirror:TrPtr->success = 0;
              PrintError("Sorry, not yet available");
              return;
              break;
        default: break;
      }
      break;
    case _equirectangular:
      vars[0] = TrPtr->src->width / a;
      b       = TrPtr->src->height/ vars[0];
      switch(r_prefs->to)
      {
        case _rectilinear:
              if( a >= PI || b >= PI )
              {
                TrPtr->success = 0;
                PrintError("Wrong FOV: Must be smaller than 180 degrees");
                return;
              }
              destheight = (int)(2 * vars[0] * tan( TrPtr->src->height/(2*vars[0]) ));
              destwidth  = (int)(2 * vars[0] * tan( TrPtr->src->width/(2*vars[0]) ));
              fD.func = erect_rect;
              break;
        case _panorama   :
              if( b >= PI )
              {
                TrPtr->success = 0;
                PrintError("Wrong VFOV: Must be smaller than 180 degrees");
                return;
              }
              destheight = (int)(2 * vars[0] * tan( TrPtr->src->height/(2*vars[0]) ));
              destwidth = TrPtr->src->width;
              fD.func = erect_pano;
              break;
        case _equirectangular:TrPtr->success = 0;
              PrintError("Same Mapping!");
              return;
              break;
        case _spherical_cp: 
              destwidth   = 2 * TrPtr->src->height;
              destheight  = 2 * TrPtr->src->height;
              vars[1] = TrPtr->src->height/2; // vars[1] is midpoint
              fD.func = erect_sphere_cp;
              break;
        case _spherical_tp:
              destwidth   = TrPtr->src->width;
              destheight  = TrPtr->src->width;
              fD.func = erect_sphere_tp;
              break;
        case _mirror:TrPtr->success = 0;
              PrintError("Sorry, not yet available");
              return;
              break;
        default: break;
      }
      break;
    case _spherical_cp:
      if( r_prefs->hfov > MAX_FISHEYE_FOV && r_prefs->vfov > MAX_FISHEYE_FOV ){
        TrPtr->success = 0;
        PrintError("Fisheye lens processing limited to fov <= %lg", MAX_FISHEYE_FOV);
        return;
      }
      vars[0] = TrPtr->src->width / a;
      switch(r_prefs->to)
      {
        case _rectilinear:TrPtr->success = 0;
              PrintError("Sorry, not yet available");
              return; 
              break;
        case _panorama   :
              if(r_prefs->vfov >= 180.0 )
              {
                TrPtr->success = 0;
                PrintError("Wrong VFOV: Must be smaller than 180 degrees");
                return;
              }
              destheight = (int)(PI * vars[0] * tan(r_prefs->vfov * PI / 360.0));
              destwidth  = (int)(PI * vars[0] * PI);
              fD.func    = sphere_cp_pano;
              break;
        case _equirectangular:
              destheight = (int)((TrPtr->src->width > TrPtr->src->height ? 
                             TrPtr->src->width/2: TrPtr->src->height/2) * PI / 2.0);
              destwidth  = (int)(vars[0] *  PI * PI);
              vars[1]    = destheight / 2;

              fD.func = sphere_cp_erect;
              break;
        case _spherical_cp:TrPtr->success = 0;
              PrintError("Same Mapping!");
              return;
              break;
        case _spherical_tp:TrPtr->success = 0;
              PrintError("Use tool perspective.");
              return;
              break;
        case _mirror:
              vars[1] = TrPtr->src->width / (2 * sin(a/4.0)) ; // Radius of mirror
              destwidth  = TrPtr->src->width ;
              destheight = TrPtr->src->height ;
              fD.func = sphere_cp_mirror;
              break;
        default: break;
      }
      break;
    case _spherical_tp:
      if( r_prefs->hfov > MAX_FISHEYE_FOV && r_prefs->vfov > MAX_FISHEYE_FOV ){
        TrPtr->success = 0;
        PrintError("Fisheye lens processing limited to fov <= %lg", MAX_FISHEYE_FOV);
        return;
      }
      vars[0] = TrPtr->src->width / a;
      b = TrPtr->src->height / vars[0]; 
      switch(r_prefs->to)
      {
        case _rectilinear:
              if( a >= PI || b >= PI )
              {
                TrPtr->success = 0;
                PrintError("Wrong FOV: Must be smaller than 180 degrees");
                return;
              }
              destheight    = (int)(2.0 * vars[0] * tan( TrPtr->src->height / (2.0*vars[0]) ) + 0.5);
              destwidth     = (int)(2.0 * tan( a/2.0 ) * vars[0] + 0.5);
              fD.func = sphere_tp_rect;
              break;
        case _panorama   :
              if(  b >= PI )
              {
                TrPtr->success = 0;
                PrintError("Wrong VFOV: Must be smaller than 180 degrees");
                return;
              }
              destheight  = (int)(2.0 * vars[0] * tan( TrPtr->src->height / (2.0*vars[0]) ) + 0.5);
              destwidth   = TrPtr->src->width;
              fD.func = sphere_tp_pano;
              break;
        case _equirectangular:
              destheight  = TrPtr->src->height ;
              destwidth   = TrPtr->src->width;
              vars[1]             = destheight / 2;
              fD.func = sphere_tp_erect;
              break;
        case _spherical_cp:TrPtr->success = 0;
              PrintError("Use tool perspective.");
              return;
              break;
        case _spherical_tp:TrPtr->success = 0;
              PrintError( "Same Mapping!");
              return;
              break;
        case _mirror:TrPtr->success = 0;
              PrintError( "Sorry, not yet available");
              return;
              break;
        default: break;
      }
      break;
    case _mirror:
      vars[1] = TrPtr->src->width / (2 * sin(a/4.0)) ; // Radius of mirror
      vars[0] = TrPtr->src->width / a;
      switch(r_prefs->to)
      {
        case _rectilinear:TrPtr->success = 0;
              PrintError("Sorry, not yet available");
              return;
              break;
        case _panorama   :              
              if(r_prefs->vfov >= 180.0 )
              {
                TrPtr->success = 0;
                PrintError("Wrong VFOV: Must be smaller than 180 degrees");
                return;
              }
              destheight = (int)(PI * vars[0] * tan(r_prefs->vfov * PI / 360.0));
              destwidth  = (int)(PI * vars[0] *  PI);
              fD.func = mirror_pano;
              break;
        case _equirectangular:
              destwidth  = (int)(vars[0] *  PI * PI);
              destheight = (int)(PI * vars[0] * a / 4.0);
              vars[2] = destheight / 2.0;
              fD.func = mirror_erect;
              break;
        case _spherical_cp:
              destwidth  = TrPtr->src->width ;
              destheight = TrPtr->src->height ;
              fD.func = mirror_sphere_cp;
              break;
        case _spherical_tp:TrPtr->success = 0;
              PrintError( "Sorry, not yet available");
              return;
              break;
        case _mirror:TrPtr->success = 0;
              PrintError( "Same Mapping!");
              return;
              break;
        default: break;
      }
      break;
    default: break;
  }


  
  if( SetDestImage(TrPtr, destwidth, destheight) != 0 ) 
  {
    TrPtr->success = 0;
    PrintError( "Not enough Memory.");
    return; 
  }
  
  // Do transformation

  if( fD.func != NULL)
  {
    transFormEx( TrPtr, &fD, &fD, 0, 1 );
    //transForm( TrPtr, &fD, 0 );
  }
  else
    TrPtr->success = 0;

  if( TrPtr->success == 0 && ! (TrPtr->mode & _destSupplied)) // Moved here
          myfree( (void**)TrPtr->dest->data );

}

void SetRemapDefaults( rPrefs *rP )
{
  rP->magic   = 30;       // Magic number for file validity check
  rP->from  = _rectilinear;
  rP->to    = _panorama;
  rP->hfov  = 60.0;
  rP->vfov  = 60.0;

}


