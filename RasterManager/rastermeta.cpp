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
    m_psUnit = NULL;
    double fNoDataValue = (double) -std::numeric_limits<float>::max();
    Init(&fNoDataValue, DEFAULT_RASTER_DRIVER, &nDType, NULL, NULL);
}

RasterMeta::RasterMeta(double fTop, double fLeft, int nRows, int nCols,
                       double *dCellHeight, double *dCellWidth, double *fNoData,
                       const char * psDriver, GDALDataType * eDataType, const char * psProjection, const char * psUnit)
    : ExtentRectangle(fTop, fLeft,  nRows, nCols, *dCellHeight, *dCellWidth)
{
    m_psGDALDriver = NULL;
    m_psProjection = NULL;
    m_psUnit = NULL;
    Init(fNoData, psDriver, eDataType, psProjection, psUnit);
}

RasterMeta::RasterMeta(const char * psFilePath) : ExtentRectangle(psFilePath)
{
    m_psGDALDriver = NULL;
    m_psProjection = NULL;
    m_psUnit = NULL;
    GetPropertiesFromExistingRaster(psFilePath);
}
RasterMeta::RasterMeta(QString psFilePath) : ExtentRectangle(psFilePath)
{
    m_psGDALDriver = NULL;
    m_psProjection = NULL;
    m_psUnit = NULL;
    const QByteArray qbFilePath = psFilePath.toLocal8Bit();
    GetPropertiesFromExistingRaster(qbFilePath.data());
}

RasterMeta::RasterMeta(RasterMeta &source) : ExtentRectangle(source)
{
    m_psGDALDriver = NULL;
    m_psProjection = NULL;
    m_psUnit = NULL;
    Init(source.GetNoDataValuePtr(), source.GetGDALDriver(), source.GetGDALDataType(), source.GetProjectionRef(), source.GetUnit());
}

RasterMeta::~RasterMeta()
{
    if (m_psGDALDriver)
        free(m_psGDALDriver);

    if (m_psProjection)
        free(m_psProjection);

    if (m_psUnit)
        free(m_psUnit);
}


void RasterMeta::Init(double * fNoDataValue, const char * psDriver, GDALDataType * eDataType, const char * psProjection, const char * psUnit)
{
    SetNoDataValue(fNoDataValue);

    if (eDataType != NULL)
        SetGDALDataType(eDataType);

    SetGDALDriver(psDriver);
    SetUnit(psUnit);
    SetProjectionRef(psProjection);

}

void RasterMeta::operator=(RasterMeta &source)
{
    ExtentRectangle::operator =(source);
    Init(source.GetNoDataValuePtr(), source.GetGDALDriver(), source.GetGDALDataType(), source.GetProjectionRef(), source.GetUnit());
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
    char * psUnit = NULL;
    char * psWKT = NULL;
    psWKT = (char *) psProjection;

    OGRSpatialReference poSRS;
    poSRS.importFromWkt(&psWKT);
    poSRS.GetLinearUnits(&psUnit);

    if (nSuccess == 0){
        b_HasNoData = false;
        dNoData = DEFAULT_NO_DATA;
    }
    Init(&dNoData, psDriver, &gdDataType, psProjection, psUnit);

//    if (psUnit)
//        CPLFree(psUnit);
//    if (psWKT)
//        CPLFree(psWKT);

    GDALClose(pDS);

}

bool RasterMeta::IsDivisible(){

    // Check that the cell width and height are valid AND
    // 1. Left / CellWidth % 1 == 0
    // 2. Top / CellHeight % 1 == 0
    if ( !qFuzzyIsNull( GetCellWidth() ) || !qFuzzyIsNull(GetCellHeight()) ||
         !qFuzzyIsNull( fmod( GetLeft() / GetCellWidth() , 1)) ||
         !qFuzzyIsNull( fmod( GetTop()  / GetCellHeight(), 1 )) ){
        return true;
    }
    return false;
}

bool RasterMeta::IsConcurrent(RasterMeta * pCompareMeta){

    // Concurrent is a special case of orthoginality so check that first.
    if (!IsOrthogonal(pCompareMeta))
        return false;

    //
    if (!qFuzzyCompare( (double)pCompareMeta->GetTop(), (double)GetTop()) ||
            !qFuzzyCompare( (double)pCompareMeta->GetLeft(), (double)GetLeft() ) ||
            !qFuzzyCompare( (double)pCompareMeta->GetRows(), (double)GetRows() ) ||
            !qFuzzyCompare( (double)pCompareMeta->GetCols(), (double)GetCols() ) ){
        return false;
    }
    return true;
}

bool RasterMeta::IsOrthogonal(RasterMeta * pCompareMeta){

    // TODO: We really should check that the Projections are the same

    // Cell dimensions must be the same
    if ( !qFuzzyCompare( fabs(GetCellHeight()), fabs(pCompareMeta->GetCellHeight()) ) ||
         !qFuzzyCompare( fabs(GetCellWidth() ), fabs(pCompareMeta->GetCellWidth() ) ))
        return false;

    // Make sure the difference between left and right is divisible by the cell dimentsions
    if ( !qFuzzyIsNull( fmod((GetLeft() - pCompareMeta->GetLeft() ) / GetCellWidth(), 1 ) )  ||
         !qFuzzyIsNull( fmod((GetTop()  - pCompareMeta->GetTop()  ) / GetCellHeight(),  1 ) ) )
        return false;

    return true;
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

void RasterMeta::SetUnit(const char * psUnit)
{
    if (m_psUnit){
        free(m_psUnit);
    }
    // Now set it if necessary
    if (psUnit)
        m_psUnit = strdup(psUnit);
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

    if (sRasterSplit.size() == 0 || sRasterSplit.at(0).length() == 0)
        throw RasterManagerException(INPUT_FILE_NOT_VALID);
    QString sFirstRaster = sRasterSplit.at(0);
    RasterMeta pFirstRaster(sFirstRaster);

    int counter = 0;

    foreach (QString raster, sRasterSplit) {
        if (raster.length() > 8){
            if (bCheckExist)
                CheckFile(raster, true);
            counter++;
            if (bCheckOthogonal || bCheckConcurrent){
                RasterMeta pOtherRaster(raster);
                if (bCheckOthogonal && !pOtherRaster.IsOrthogonal(&pFirstRaster)){
                        throw RasterManagerException(RASTER_ORTHOGONAL, QString("%1").arg(raster) );
                }
                else if(bCheckConcurrent && !pOtherRaster.IsConcurrent(&pFirstRaster)){
                        throw RasterManagerException(RASTER_CONCURRENCY, QString("%1 vs. %2").arg(sFirstRaster).arg(raster) );
                }
            }
            slRasters.append(raster);
        }
    }
    return slRasters;
}

} // RasterManager
