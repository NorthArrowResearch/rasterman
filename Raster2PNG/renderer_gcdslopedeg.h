#ifndef RENDERER_GCDSLOPEDEG_H
#define RENDERER_GCDSLOPEDEG_H

#include "renderer_classified.h"

class Renderer_GCDSlopeDeg : public Renderer_Classified
{
public:
    Renderer_GCDSlopeDeg(const char *rasterPath,
                         int nTransparency = 255);

protected:
    void createByteRaster();
    void setClassBreaks();
};

#endif // RENDERER_GCDSLOPEDEG_H
