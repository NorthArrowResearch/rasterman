#ifndef RENDERER_GCDERROR_H
#define RENDERER_GCDERROR_H

#include "renderer_classified.h"

class Renderer_GCDError : public Renderer_Classified
{
public:
    Renderer_GCDError(const char *rasterPath,
                      int nTransparency = 255);

protected:
    void createByteRaster();
    void setClassBreaks();
};

#endif // RENDERER_GCDERROR_H
