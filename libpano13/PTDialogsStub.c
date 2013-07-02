#include "filter.h"

#ifdef __Mac__
  #include "sys_mac.h"
#endif

// Jim Watters 2008/01/11  need sys_win.h for dialogs to work in Windows
//                         need sys_ansi.h for command prompt to work
#ifdef __Ansi__
  #include "sys_ansi.h"
#elif defined (__Win__)
  #include "sys_win.h"
#endif


#if !__Mac__ && !__Win__ && !__Ansi__
  #include "sys_X11.h"
#endif

int SetPerspectivePrefs(  pPrefs * thePrefs )
{

}