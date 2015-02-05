#ifndef RENDERER_H
#define RENDERER_H

#include "gdal_priv.h"
#include <QtGui>
#include "raster2png_global.h"

enum ColorRamp{
  CR_BlackWhite,
  CR_DEM,
  CR_DoD,
  CR_GrainSize,
  CR_GreenBlue,
  CR_LtBlueDkBlue,
  CR_PartialSpectrum,
  CR_Precipitation,
  CR_Slope,
  CR_SlopeGCD,
  CR_WhiteRed,
};

namespace Raster2PNG {
class RASTER2PNGSHARED_EXPORT Renderer
{
public:
    Renderer(const char *inputRasterPath,
             ColorRamp ramp = CR_BlackWhite,
             int nTransparency = 255,
             bool zeroNoData = FALSE);
    virtual ~Renderer();

    void printLegend();
    void printLegend(const char *path);
    int rasterToPNG(const char *pngPath,
                    int nQuality,
                    int nLength);
    int setRendererColorTable(ColorRamp rampStyle,
                      int nTransparency);
    void setPrecision(int prec);
    void setZeroNoData(bool bValue);

    static void stackImages(const char *inputList, const char *outputImage, int nQuality);

protected:
    const char *rasterPath;
    const char *pngOutPath;
    float *oldRow;
    unsigned char *newRow;
    QString tempRasterPath, legendPath;
    GDALDataset *pRaster, *pTempRaster, *pPngDS;
    GDALDriver *pDriverPNG, *pDriverTiff;
    GDALColorTable *colorTable;
    GDALDataType rasterType;
    int nRows, nCols, precision;
    double min, max, mean, stdev, noData, noData2;
    double adjMin, adjMax, adjMean, range;
    double transform[6];
    bool zeroNoData, zeroCenter;

    void cleanUp();
    virtual void createByteRaster() = 0;
    virtual void createLegend() = 0;
    int resizeAndCompressPNG(const char *inputImage,
                             int nLength,
                             int nQuality);
    void setLegendPath();
    void setLegendPath(const char *path);
    void setPrecision();
    int setTempRasterPath(const char *rasterPath);
    void setup();
    int setupRaster(const char *inputRasterPath);
};
}
#endif // RENDERER_H
