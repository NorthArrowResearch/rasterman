#ifndef CLASSIFYRENDERER_H
#define CLASSIFYRENDERER_H

#include "renderer.h"

class Renderer_Classified : public Renderer
{
public:
    Renderer_Classified(const char *rasterPath,
                     int classCount,
                     ColorRamp ramp = CR_BlackWhite,
                     int nTransparency = 255,
                     bool zeroCenter = FALSE,
                     bool zeroNoData = FALSE);

protected:
    QVector<double> classBreaks;
    int nClasses, nOdd;
    double interval;

    void classifyRaster();
    void createByteRaster();
    void createLegend();
    void setClassBreaks();
    void setEqualIntervalBreaks();
    void setZeroCenter(bool bValue);
};

#endif // CLASSIFYRENDERER_H
