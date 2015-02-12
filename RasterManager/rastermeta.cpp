#define MY_DLL_EXPORT

#include "rastermeta.h"
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

} // RasterManager
