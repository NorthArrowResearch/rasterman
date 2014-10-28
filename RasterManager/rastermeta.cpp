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
    float fNoDataValue = (float) std::numeric_limits<float>::min();
    Init(fNoDataValue, DEFAULT_RASTER_DRIVER, GDT_Float32);
}

RasterMeta::RasterMeta(double fTop, double fLeft, int nRows, int nCols, double dCellHeight, double dCellWidth, double fNoData, const char * psDriver, GDALDataType eDataType)
    : ExtentRectangle(fTop, fLeft,  nRows, nCols, dCellHeight, dCellWidth)
{
    Init(fNoData, psDriver, eDataType);
}

RasterMeta::RasterMeta(const char * psFilePath) : ExtentRectangle(psFilePath)
{
    GetPropertiesFromExistingRaster(psFilePath);
}

RasterMeta::RasterMeta(RasterMeta &source) : ExtentRectangle(source)
{
    Init(source.GetNoDataValue(), source.GetGDALDriver(), source.GetGDALDataType());
}

void RasterMeta::Init(double fNoDataValue, const char * psDriver, GDALDataType eDataType)
{
    m_fNoDataValue = fNoDataValue;
    m_psGDALDriver = const_cast<char *>(psDriver);
    m_eDataType = eDataType;
}

void RasterMeta::operator=(RasterMeta &source)
{
    ExtentRectangle::operator =(source);
    Init(source.GetNoDataValue(), source.GetGDALDriver(), source.GetGDALDataType());
}


void RasterMeta::GetPropertiesFromExistingRaster(const char * psFilePath)
{
    GDALAllRegister();

    // Open the original dataset
    GDALDataset * pDS = (GDALDataset*) GDALOpen(psFilePath, GA_ReadOnly);
    if (pDS  == NULL)
        throw "error opening raster file";

    int nSuccess;
    SetNoDataValue(pDS->GetRasterBand(1)->GetNoDataValue(&nSuccess));

    if (nSuccess == 0)
        SetNoDataValue(DEFAULT_NO_DATA);

    GDALClose(pDS);
    //GDALDestroyDriverManager();
}

} // RasterManager
