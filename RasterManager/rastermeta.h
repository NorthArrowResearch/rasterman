#ifndef RASTERMETA_H
#define RASTERMETA_H

#include "rastermanager_global.h"
#include "extentrectangle.h"
#include "gdal.h"

namespace RasterManager {

const int DEFAULT_NO_DATA = -9999;

class DLL_API RasterMeta : public ExtentRectangle
{
public:

    // Build an empty rastermeta object
    RasterMeta();

    // Build a RasterMeta from first principles
    RasterMeta(double fTop, double fLeft, int nRows, int nCols,
               double dCellHeight, double dCellWidth, double fNoData,
               const char * psDriver, GDALDataType eDataType);

    // Build a RasterMeta from an existing raster file path
    RasterMeta(const char * psFilePath);

    // Copy constructor for creating a RasterMeta from an existing RasterMeta
    RasterMeta(RasterMeta &source);

    // Assignment Operator, for assigning properties from another RasterMeta
    void operator=(RasterMeta &source);

    /**
     * @brief GetGDALDriver
     * @return
     */
    inline char * GetGDALDriver() { return m_psGDALDriver; }

    /**
     * @brief GetNoData
     * @return
     */
    inline double GetNoData() { return m_fNoData; }

    /**
     * @brief GetGDALDataType
     * @return
     */
    inline GDALDataType GetGDALDataType() { return m_eDataType; }

    /**
     * @brief GetProjectionRef
     * @return
     */
    inline char * GetProjectionRef() { return m_psProjection; }


private:

    void Init(double fNoData, const char * psDriver, GDALDataType eDataType);
    void GetPropertiesFromExistingRaster(const char * psFilePath);

    char * m_psGDALDriver;
    char * m_psProjection;

    double m_fNoData;
    GDALDataType m_eDataType;



};

} // RasterManager

#endif // RASTERMETA_H
