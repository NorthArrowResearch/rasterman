#include "renderer_gcdslopeper.h"

Renderer_GCDSlopePer::Renderer_GCDSlopePer(const char *rasterPath,
                                           int nTransparency):Renderer_Classified(rasterPath, 13, CR_SlopeGCD, nTransparency, false, false)
{
}

void Renderer_GCDSlopePer::createByteRaster()
{
    setClassBreaks();
    classifyRaster();
}

void Renderer_GCDSlopePer::setClassBreaks()
{
    classBreaks.resize(14);
    classBreaks[0] = 0.0;
    classBreaks[1] = 1.0;
    classBreaks[2] = 2.0;
    classBreaks[3] = 5.0;
    classBreaks[4] = 10.0;
    classBreaks[5] = 15.0;
    classBreaks[6] = 20.0;
    classBreaks[7] = 25.0;
    classBreaks[8] = 30.0;
    classBreaks[9] = 35.0;
    classBreaks[10] = 40.0;
    classBreaks[11] = 50.0;
    classBreaks[12] = 100.0;
    classBreaks[13] = 300.0;
}
