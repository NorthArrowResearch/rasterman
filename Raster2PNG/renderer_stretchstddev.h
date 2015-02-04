#ifndef STRETCHRENDERERSTDEV_H
#define STRETCHRENDERERSTDEV_H

#include "renderer.h"

class Renderer_StretchStdDev : public Renderer
{
public:
    Renderer_StretchStdDev(const char *inputRasterPath,
                         double stDevStretch = 2.5,
                         ColorRamp ramp = CR_BlackWhite,
                         int nTransparency = 255,
                         bool zeroCenter = FALSE,
                         bool zeroNoData = FALSE);

protected:
    double sdStretch, sdMin, sdMax, corVal, maxCalc, minCalc;

    void createByteRaster();
    void createLegend();
    void setZeroCenter(bool bValue);
};

#endif // STRETCHRENDERERSTDEV_H
