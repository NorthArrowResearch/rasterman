#ifndef RENDERER_GCDSLOPEPER_H
#define RENDERER_GCDSLOPEPER_H

#include "renderer_classified.h"

class Renderer_GCDSlopePer : public Renderer_Classified
{
public:
    Renderer_GCDSlopePer(const char *rasterPath,
                         int nTransparency = 255);

protected:
    void createByteRaster();
    void setClassBreaks();
};

#endif // RENDERER_GCDSLOPEPER_H
