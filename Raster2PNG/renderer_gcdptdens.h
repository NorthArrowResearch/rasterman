#ifndef RENDERER_GCDPTDENS_H
#define RENDERER_GCDPTDENS_H

#include "renderer_classified.h"

class Renderer_GCDPtDens : public Renderer_Classified
{
public:
    Renderer_GCDPtDens(const char *rasterPath,
                       int nTransparency = 255);

protected:
    void createByteRaster();
    void setClassBreaks();
};

#endif // RENDERER_GCDPTDENS_H
