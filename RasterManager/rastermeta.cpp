#define MY_DLL_EXPORT

#include "rastermeta.h"
#include "rastermanager.h"
#include "rastermanager_interface.h"
#include "rastermanager_exception.h"

#include "gdal_priv.h"
#include "ogrsf_frmts.h"

namespace RasterManager
{

const char * DEFAULT_RASTER_DRIVER = "GTiff";
GDALDataType nDType = GDT_Float32;

RasterMeta::RasterMeta() : ExtentRectangle()
{
    m_psGDALDriver = NULL;
    m_psProjection = NULL;
    double fNoDataValue = (double) -std::numeric_limits<float>::max();
    Init(&fNoDataValue, DEFAULT_RASTER_DRIVER, &nDType, NULL);
}

RasterMeta::RasterMeta(double fTop, double fLeft, int nRows, int nCols,
                       double *dCellHeight, double *dCellWidth, double *fNoData,
                       const char * psDriver, GDALDataType * eDataType, const char * psProjection)
    : ExtentRectangle(fTop, fLeft,  nRows, nCols, *dCellHeight, *dCellWidth)
{
    m_psGDALDriver = NULL;
    m_psProjection = NULL;
    Init(fNoData, psDriver, eDataType, psProjection);
}

RasterMeta::RasterMeta(const char * psFilePath) : ExtentRectangle(psFilePath)
{
    m_psGDALDriver = NULL;
    m_psProjection = NULL;
    GetPropertiesFromExistingRaster(psFilePath);
}
RasterMeta::RasterMeta(QString psFilePath) : ExtentRectangle(psFilePath)
{
    m_psGDALDriver = NULL;
    m_psProjection = NULL;
    const QByteArray qbFilePath = psFilePath.toLocal8Bit();
    GetPropertiesFromExistingRaster(qbFilePath.data());
}

RasterMeta::RasterMeta(RasterMeta &source) : ExtentRectangle(source)
{
    m_psGDALDriver = NULL;
    m_psProjection = NULL;
    Init(source.GetNoDataValuePtr(), source.GetGDALDriver(), source.GetGDALDataType(), source.GetProjectionRef());
}

RasterMeta::~RasterMeta()
{
    if (m_psGDALDriver)
        free(m_psGDALDriver);

    if (m_psProjection)
        free(m_psProjection);
}


void RasterMeta::Init(double * fNoDataValue, const char * psDriver, GDALDataType * eDataType, const char * psProjection)
{
    SetNoDataValue(fNoDataValue);

    if (eDataType != NULL)
        SetGDALDataType(eDataType);

    SetGDALDriver(psDriver);
    SetProjectionRef(psProjection);

}

void RasterMeta::operator=(RasterMeta &source)
{
    ExtentRectangle::operator =(source);
    Init(source.GetNoDataValuePtr(), source.GetGDALDriver(), source.GetGDALDataType(), source.GetProjectionRef());
}

GDALDataType * RasterMeta::GetGDALDataType() { return &m_eDataType; }

void RasterMeta::SetGDALDataType(GDALDataType * fDataType) { m_eDataType = *fDataType; }

void RasterMeta::GetPropertiesFromExistingRaster(const char * psFilePath)
{
    // Open the original dataset
    b_HasNoData = true;

    GDALDataset * pDS = (GDALDataset*) GDALOpen(psFilePath, GA_ReadOnly);
    if (pDS  == NULL)
        throw RasterManagerException(INPUT_FILE_NOT_VALID, "Error opening raster file: " + QString(psFilePath) );

    int nSuccess;

    GDALDataType gdDataType =  pDS->GetRasterBand(1)->GetRasterDataType();

    double dNoData =  pDS->GetRasterBand(1)->GetNoDataValue(&nSuccess);

    const char * psDriver = pDS->GetDriver()->GetDescription();

    const char * psProjection = pDS->GetProjectionRef();

    if (nSuccess == 0){
        b_HasNoData = false;
        dNoData = DEFAULT_NO_DATA;
    }
    Init(&dNoData, psDriver, &gdDataType, psProjection);

    GDALClose(pDS);

}

int RasterMeta::IsOthogonal(){

    if ( GetCellWidth() != 0 && GetCellHeight() != 0
         && fmod( GetLeft(), GetCellWidth() ) == 0
         && fmod( GetTop(), GetCellHeight() ) == 0 ){
        return true;
    }
    return false;
}

int RasterMeta::GetPrecision(double num){
    int count = 0;
    num = fabs(num);
    num = num - int(num);
    while ( fabs(num) >= 0.0000001 ){
        num = num * 10;
        num = num - int(num);
        count++;
    }
    return count;
}

int RasterMeta::GetVerticalPrecision()
{

    int leftPrecision = GetPrecision( GetLeft() );
    int cellWidth = GetPrecision( GetCellHeight() );

    if (leftPrecision > cellWidth)
        return leftPrecision;
    else
        return cellWidth;

}

int RasterMeta::GetHorizontalPrecision()
{

    int topPrecision = GetPrecision( GetTop() );
    int cellHeight = GetPrecision( GetCellWidth() );

    if (topPrecision > cellHeight)
        return topPrecision;
    else
        return cellHeight;

}

void RasterMeta::SetNoDataValue(double * fNoData) {
    // Weird case. 0 is the same as NULL for pointers.
    if (fNoData == NULL ){
        m_fNoDataValue = 0;
    }
    else {
       m_fNoDataValue = *fNoData;
    }
    b_HasNoData = true;
}


void RasterMeta::SetGDALDriver(const char * sGDALDriver)
{
    if (m_psGDALDriver){
        free(m_psGDALDriver);
    }
    // Now set it if necessary
    if (sGDALDriver)
        m_psGDALDriver = strdup(sGDALDriver);

}

void RasterMeta::SetProjectionRef(const char * fProjectionRef)
{
    if (m_psProjection){
        free(m_psProjection);
    }
    // Now set it if necessary
    if (fProjectionRef)
        m_psProjection = strdup(fProjectionRef);
}

void RasterMeta::RasterMetaExpand(QList<QString> pRasters, RasterMeta * pOutputMeta){
    /*****************************************************************************************
     * Expand a Raster's output meta to include all the inputs
     */
    int counter = 0;
    foreach (QString raster, pRasters) {
        CheckFile(raster, true);
        counter++;
        RasterMeta erRasterInput (raster);

        // First time round set the bounds to the first raster we give it.
        pOutputMeta->Union(&erRasterInput);
    }
}

QList<QString> RasterMeta::RasterUnDelimit(QString sRasters, bool bCheckExist, bool bCheckOthogonal, bool bCheckConcurrent){
    /*****************************************************************************************
     * Split a delimited File list into individual raster paths, optionally checking for file existence.
     */
    QList<QString> sRasterSplit = sRasters.split(";");
    QList<QString> slRasters;
    RasterMeta * pFirstRaster;
    QString sFirstRaster;
    RasterMeta * pOtherRaster;
    int counter = 0;

    foreach (QString raster, sRasterSplit) {
        if (raster.length() > 8){
            if (bCheckExist)
                CheckFile(raster, true);
            counter++;
            if (counter == 1 ){
                sFirstRaster = raster;
                if (bCheckOthogonal || bCheckConcurrent){
                    pFirstRaster = new RasterMeta(raster);
                }
            }
            else if (bCheckOthogonal || bCheckConcurrent){
                pOtherRaster = new RasterMeta(raster);
                if (bCheckOthogonal && !pOtherRaster->IsOthogonal())
                    throw RasterManagerException(RASTER_ORTHOGONAL, QString("%1").arg(raster) );
                if (bCheckConcurrent && !pOtherRaster->IsConcurrent(pFirstRaster))
                    throw RasterManagerException(RASTER_CONCURRENCY, QString("%1 vs. %2").arg(sFirstRaster).arg(raster) );

            }


            slRasters.append(raster);
        }
    }
    return slRasters;
}

} // RasterManager
