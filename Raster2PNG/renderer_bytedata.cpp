#include "renderer_bytedata.h"

Renderer_ByteData::Renderer_ByteData(const char *inputRasterPath,
                                     ColorRamp ramp,
                                     int nTransparency):Renderer_StretchMinMax(inputRasterPath, ramp, nTransparency, false, false)
{
    setPrecision(0);
}

void Renderer_ByteData::createByteRaster()
{
    unsigned char *byteRow = (unsigned char*) CPLMalloc(sizeof(int)*nCols);
    for (int i=0; i<nRows; i++)
    {
        pRaster->GetRasterBand(1)->RasterIO(GF_Read, 0, i, nCols, 1, byteRow, nCols, 1, GDT_Byte, 0, 0);

        for (int j=0; j<nCols; j++)
        {
            if (byteRow[j] <= 0 || byteRow[j] >= 255)
            {
                newRow[j] = 0;
            }
            else
            {
                newRow[j] = byteRow[j];
            }
        }
        pTempRaster->GetRasterBand(1)->RasterIO(GF_Write, 0, i, nCols, 1, newRow, nCols, 1, GDT_Byte, 0, 0);
    }
    CPLFree(byteRow);
}
