#ifndef RASTERMETA_H
#define RASTERMETA_H

#include "rastermanager_global.h"
#include "extentrectangle.h"
#include "gdal.h"

namespace RasterManager {

const int DEFAULT_NO_DATA = -9999;

class RasterMeta : public ExtentRectangle
{
public:

    // Build a RasterMeta from first principles
    RasterMeta(double fTop, double fLeft, int nRows, int nCols, double dCellHeight, double dCellWidth, double fNoData, const char * psDriver, GDALDataType eDataType);

    // Build a RasterMeta from an existing raster file path
    RasterMeta(const char * psFilePath);

    // Copy constructor for creating a RasterMeta from an existing RasterMeta
    RasterMeta(RasterMeta &source);

    // Assignment Operator, for assigning properties from another RasterMeta
    void operator=(RasterMeta &source);

    inline const char * GetGDALDriver() { return m_psGDALDriver; }
    inline double GetNoData() { return m_fNoData; }
    inline GDALDataType GetGDALDataType() { return m_eDataType; }

private:

    void Init(double fNoData, const char * psDriver, GDALDataType eDataType);
    void GetPropertiesFromExistingRaster(const char * psFilePath);

    const char * m_psGDALDriver;
    double m_fNoData;
    GDALDataType m_eDataType;
};

} // RasterManager

#endif // RASTERMETA_H
