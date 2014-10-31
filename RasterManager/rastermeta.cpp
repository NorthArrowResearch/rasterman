#define MY_DLL_EXPORT

#include "rastermeta.h"
#include "rastermanager_interface.h"
//#include "raster.h"

#include "gdal_priv.h"
#include "ogrsf_frmts.h"

namespace RasterManager
{

const char * DEFAULT_RASTER_DRIVER = "GTiff";

RasterMeta::RasterMeta() : ExtentRectangle()
{
    float fNoDataValue = (float) std::numeric_limits<float>::lowest();
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

void RasterMeta::Init(double fNoDataValue, const char * psDriver, GDALDataType eDataType, const char * psProjection)
{
    if (psProjection != NULL)
        SetGDALDriver(psDriver);

    if (fNoDataValue != NULL)
        SetNoDataValue(fNoDataValue);

    if (eDataType != NULL)
        SetGDALDataType(eDataType);

    if (psProjection != NULL)
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
    //GDALDestroyDriverManager();
}


void RasterMeta::SetGDALDriver(const char *sGDALDriver) {
    m_psGDALDriver = (char *) malloc(std::strlen(sGDALDriver) * sizeof(char)+1);
    std::strcpy(m_psGDALDriver, sGDALDriver);
}

void RasterMeta::SetProjectionRef(const char *fProjectionRef)
{
    m_psProjection = (char *) malloc(std::strlen(fProjectionRef) * sizeof(char)+1);
    std::strcpy(m_psProjection, fProjectionRef);
}

} // RasterManager
