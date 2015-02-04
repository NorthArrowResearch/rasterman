#include "renderer_gcdslopedeg.h"

Renderer_GCDSlopeDeg::Renderer_GCDSlopeDeg(const char *rasterPath,
                                           int nTransparency):Renderer_Classified(rasterPath, 10, CR_SlopeGCD, nTransparency, false, false)
{
    setZeroCenter(zeroCenter);
    setPrecision();
    adjMax = 90.0, adjMin = 0.0;
}

void Renderer_GCDSlopeDeg::createByteRaster()
{
    setClassBreaks();
    classifyRaster();
}

void Renderer_GCDSlopeDeg::setClassBreaks()
{
    classBreaks.resize(nClasses + 1);
    classBreaks[0] = 0.0;
    classBreaks[1] = 2.0;
    classBreaks[2] = 5.0;
    classBreaks[3] = 10.0;
    classBreaks[4] = 15.0;
    classBreaks[5] = 25.0;
    classBreaks[6] = 35.0;
    classBreaks[7] = 45.0;
    classBreaks[8] = 60.0;
    classBreaks[9] = 80.0;
    classBreaks[10] = 90.0;
}

