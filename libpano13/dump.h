
#ifndef _Included_dump

#include "filter.h"
#include "panorama.h"

#define _Included_dump

void PanoDumpPTRect(PTRect *rect, char *label, int indent);
void PanoDumpCropInfo(CropInfo *crop, char *label, int indent);
void PanoDumpImage(Image *im, char *label, int indent);
void PanoDumpAdjustData(aPrefs* aP, char *label, int indent);
void PanoDumpCorrectPrefs(cPrefs *cP, char *label, int indent);


#endif
