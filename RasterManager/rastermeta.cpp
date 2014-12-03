#define MY_DLL_EXPORT

#include "rastermeta.h"
#include "rastermanager_interface.h"

#include "gdal_priv.h"
#include "ogrsf_frmts.h"

namespace RasterManager
{

const char * DEFAULT_RASTER_DRIVER = "GTiff";

RasterMeta::RasterMeta() : ExtentRectangle()
{
    m_psGDALDriver = NULL;
    m_psProjection = NULL;
    double fNoDataValue = (double) std::numeric_limits<float>::lowest();
    Init(fNoDataValue, DEFAULT_RASTER_DRIVER, GDT_Float32, NULL);
}

RasterMeta::RasterMeta(double fTop, double fLeft, int nRows, int nCols,
                       double dCellHeight, double dCellWidth, double fNoData,
                       const char * psDriver, GDALDataType eDataType, const char * psProjection)
    : ExtentRectangle(fTop, fLeft,  nRows, nCols, dCellHeight, dCellWidth)
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
    Init(source.GetNoDataValue(), source.GetGDALDriver(), source.GetGDALDataType(), source.GetProjectionRef());
}

RasterMeta::~RasterMeta()
{
    if (m_psGDALDriver)
        free(m_psGDALDriver);

    if (m_psProjection)
        free(m_psProjection);
}


void RasterMeta::Init(double fNoDataValue, const char * psDriver, GDALDataType eDataType, const char * psProjection)
{

    if (fNoDataValue != 0)
        SetNoDataValue(fNoDataValue);

    if (eDataType != NULL)
        SetGDALDataType(eDataType);

    SetGDALDriver(psDriver);
    SetProjectionRef(psProjection);

}

void RasterMeta::operator=(RasterMeta &source)
{
    ExtentRectangle::operator =(source);
    Init(source.GetNoDataValue(), source.GetGDALDriver(), source.GetGDALDataType(), source.GetProjectionRef());
}

GDALDataType RasterMeta::GetGDALDataType() { return m_eDataType; }

void RasterMeta::SetGDALDataType(GDALDataType fDataType) { m_eDataType = fDataType; }

void RasterMeta::GetPropertiesFromExistingRaster(const char * psFilePath)
{
    // Open the original dataset
    GDALDataset * pDS = (GDALDataset*) GDALOpen(psFilePath, GA_ReadOnly);
    if (pDS  == NULL)
        throw std::runtime_error("error opening raster file");

    int nSuccess;

    GDALDataType gdDataType =  pDS->GetRasterBand(1)->GetRasterDataType();

    double dNoData =  pDS->GetRasterBand(1)->GetNoDataValue(&nSuccess);

    const char * psDriver = pDS->GetDriver()->GetDescription();

    const char * psProjection = pDS->GetProjectionRef();

    if (nSuccess == 0)
        dNoData = DEFAULT_NO_DATA;

    Init(dNoData, psDriver, gdDataType, psProjection);

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

int RasterMeta::IsConcurrent(RasterMeta * pCompareMeta){
    if (pCompareMeta->GetTop() == GetTop()
            && pCompareMeta->GetLeft() == GetLeft()
            && pCompareMeta->GetRows() == GetRows()
            && pCompareMeta->GetCols() == GetCols() ){
        return 1;
    }
    return 0;
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
