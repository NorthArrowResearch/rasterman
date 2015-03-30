#ifndef RASTERMETA_H
#define RASTERMETA_H

#include "rastermanager_global.h"
#include "extentrectangle.h"
#include "gdal_priv.h"
#include <QString>
#include <QList>

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
               double * dCellHeight, double * dCellWidth, double * fNoData,
               const char * psDriver, GDALDataType *eDataType, const char *psProjection);

    // Build a RasterMeta from an existing raster file path
    RasterMeta(const char * psFilePath);
    // SAme as above with QString
    RasterMeta(QString psFilePath);


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
     * @brief GetNoDataValuePtr
     * @return
     */
    inline double * GetNoDataValuePtr() { return &m_fNoDataValue; }

    inline double GetCellArea(){ return fabs(GetCellHeight()) * fabs(GetCellWidth()); }

    /**
     * @brief GetGDALDataType
     * @return
     */
    GDALDataType *GetGDALDataType();

    /**
     * @brief GetProjectionRef
     * @return
     */
    inline char * GetProjectionRef() { return m_psProjection; }

    /**
     * @brief HasNoDataValue
     * @return
     */
    inline bool HasNoDataValue() const {return b_HasNoData;}

    /**
     * @brief GetVerticalPrecision
     * @return
     */
    int GetVerticalPrecision();

    /**
     * @brief GetHorizontalPrecision
     * @return
     */
    int GetHorizontalPrecision();

    /**
     * @brief SetGDALDataType
     * @param fDataType
     */
    void SetGDALDataType(GDALDataType *fDataType);

    /**
     * @brief SetNoDataValue
     * @param fNoData
     */
    void SetNoDataValue(double *fNoData);
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

    /**
     * @brief SetProjectionRef
     * @param fProjectionRef
     */
    void SetProjectionRef(const char * fProjectionRef);

    /**
     * @brief RasterMetaExpand
     * @param Rasters
     * @param pOutputRaster
     */
    static RasterMeta *RasterMetaExpand(QList<QString> pRasters);

    /**
     * @brief RasterUnDelimit
     * @param sRasters
     * @param slRasters
     * @param bCheckExist
     */
    static QList<QString> RasterUnDelimit(QString sRasters, bool bCheckExist, bool bCheckOthogonal, bool bCheckConcurrent);

protected:

    void SetGDALDriver(const char * sGDALDriver);

private:

    void Init(double *fNoData, const char * psDriver, GDALDataType *eDataType, const char *psProjection);
    void GetPropertiesFromExistingRaster(const char * psFilePath);

    char * m_psGDALDriver;
    char * m_psProjection;

    double m_fNoDataValue;
    bool b_HasNoData; // Recall we need this because m_fNoDataValue will be a value of
                     // lowest possible double if it doesn't read anything from the
                     // actual raster.
    GDALDataType m_eDataType;

    /**
     * @brief GetPrecision
     * @param num
     * @return
     */
    static int GetPrecision(double num);

};


} // RasterManager

#endif // RASTERMETA_H
