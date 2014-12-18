#ifndef RASTERMETA_H
#define RASTERMETA_H

#include "rastermanager_global.h"
#include "extentrectangle.h"
#include "gdal_priv.h"

namespace RasterManager {

const int DEFAULT_NO_DATA = -9999;

class RM_DLL_API RasterMeta : public ExtentRectangle
{
public:

    // Build an empty rastermeta object
    RasterMeta();

    ~RasterMeta();

    // Build a RasterMeta from first principles
    RasterMeta(double fTop, double fLeft, int nRows, int nCols,
               double dCellHeight, double dCellWidth, double fNoData,
               const char * psDriver, GDALDataType eDataType, const char *psProjection);

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
    inline double GetNoDataValue() { return m_fNoDataValue; }

    /**
     * @brief GetGDALDataType
     * @return
     */
    GDALDataType GetGDALDataType();

    /**
     * @brief GetProjectionRef
     * @return
     */
    inline char * GetProjectionRef() { return m_psProjection; }
    /**
     * @brief SetNoDataValue
     * @param fNoData
     */
    inline void SetNoDataValue(double fNoData) { m_fNoDataValue = fNoData; }

    /**
     * @brief HasNoDataValue
     * @return
     */
    inline bool HasNoDataValue() const {return b_HasNoData;}

    /**
     * @brief SetGDALDataType
     * @param fDataType
     */
    void SetGDALDataType(GDALDataType fDataType);

    /**
     * @brief IsOthogonal
     * @return
     */
    int IsOthogonal();

    /**
     * @brief IsConcurrent
     * @param pCompareMeta
     */
    int IsConcurrent(RasterMeta *pCompareMeta);


protected:

    void SetProjectionRef(const char * fProjectionRef);
    void SetGDALDriver(const char * sGDALDriver);

private:

    void Init(double fNoData, const char * psDriver, GDALDataType eDataType, const char *psProjection);
    void GetPropertiesFromExistingRaster(const char * psFilePath);

    char * m_psGDALDriver;
    char * m_psProjection;

    double m_fNoDataValue;
    bool b_HasNoData; // Recall we need this because m_fNoDataValue will be a value of
                     // lowest possible double if it doesn't read anything from the
                     // actual raster.
    GDALDataType m_eDataType;



};


} // RasterManager

#endif // RASTERMETA_H
