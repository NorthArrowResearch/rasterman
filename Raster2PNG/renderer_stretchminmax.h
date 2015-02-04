#ifndef STRETCHRENDERERMINMAX_H
#define STRETCHRENDERERMINMAX_H

#include "renderer.h"

class Renderer_StretchMinMax : public Renderer
{
public:
    Renderer_StretchMinMax(const char *inputRasterPath,
                          ColorRamp ramp = CR_BlackWhite,
                          int nTransparency = 255,
                          bool zeroCenter = FALSE,
                          bool zeroNoData = FALSE);

protected:
    double corVal, maxCalc, minCalc;

    void createByteRaster();
    void createLegend();
    void setZeroCenter(bool bValue);
};

#endif // STRETCHRENDERERMINMAX_H
