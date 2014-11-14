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
    double fNoDataValue = (double) std::numeric_limits<float>::lowest();
    Init(fNoDataValue, DEFAULT_RASTER_DRIVER, GDT_Float32, NULL);
}

RasterMeta::RasterMeta(double fTop, double fLeft, int nRows, int nCols,
                       double dCellHeight, double dCellWidth, double fNoData,
                       const char * psDriver, GDALDataType eDataType, const char * psProjection)
    : ExtentRectangle(fTop, fLeft,  nRows, nCols, dCellHeight, dCellWidth)
{
    Init(fNoData, psDriver, eDataType, psProjection);
}

RasterMeta::RasterMeta(const char * psFilePath) : ExtentRectangle(psFilePath)
{
    GetPropertiesFromExistingRaster(psFilePath);
}

RasterMeta::RasterMeta(RasterMeta &source) : ExtentRectangle(source)
{
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
    SetGDALDriver(psDriver);

    if (fNoDataValue != 0)
        SetNoDataValue(fNoDataValue);

    if (eDataType != NULL)
        SetGDALDataType(eDataType);

    SetProjectionRef(psProjection);

}

void RasterMeta::operator=(RasterMeta &source)
{
    ExtentRectangle::operator =(source);
    Init(source.GetNoDataValue(), source.GetGDALDriver(), source.GetGDALDataType(), source.GetProjectionRef());
}

void RasterMeta::GetPropertiesFromExistingRaster(const char * psFilePath)
{
    GDALAllRegister();

    // Open the original dataset
    GDALDataset * pDS = (GDALDataset*) GDALOpen(psFilePath, GA_ReadOnly);
    if (pDS  == NULL)
        throw std::runtime_error("error opening raster file");

    int nSuccess;

    SetGDALDataType(pDS->GetRasterBand(1)->GetRasterDataType());

    SetNoDataValue(pDS->GetRasterBand(1)->GetNoDataValue(&nSuccess));

    SetGDALDriver(pDS->GetDriver()->GetDescription());

    SetProjectionRef(pDS->GetProjectionRef());

    if (nSuccess == 0)
        SetNoDataValue(DEFAULT_NO_DATA);

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


void RasterMeta::SetGDALDriver(const char *sGDALDriver) {
    if (sGDALDriver){
        m_psGDALDriver = strdup(sGDALDriver);
    }
    else {
        m_psGDALDriver = NULL;
    }
}

void RasterMeta::SetProjectionRef(const char *fProjectionRef)
{
    if (fProjectionRef){
        m_psProjection = strdup(fProjectionRef);
    }
    else {
        m_psGDALDriver = NULL;
    }
}

} // RasterManager
