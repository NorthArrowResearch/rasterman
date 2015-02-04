#ifndef RENDERER_BYTEDATA_H
#define RENDERER_BYTEDATA_H

#include "renderer_stretchminmax.h"

class Renderer_ByteData : public Renderer_StretchMinMax
{
public:
    Renderer_ByteData(const char *inputRasterPath,
                      ColorRamp ramp = CR_BlackWhite,
                      int nTransparency = 255);

protected:
    void createByteRaster();
};

#endif // RENDERER_BYTEDATA_H
