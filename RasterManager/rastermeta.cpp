#define MY_DLL_EXPORT

#include "rastermeta.h"
#include "rastermanager_interface.h"
//#include "raster.h"

#include "gdal_priv.h"
#include "ogrsf_frmts.h"

namespace RasterManager
{

RasterMeta::RasterMeta(double fTop, double fLeft, int nRows, int nCols, double dCellHeight, double dCellWidth, double fNoData, const char * psDriver, GDALDataType eDataType)
    : ExtentRectangle(fTop, fLeft,  nRows, nCols, dCellHeight, dCellWidth)
{
    Init(fNoData, psDriver, eDataType);
}

RasterMeta::RasterMeta(const char * psFilePath) : ExtentRectangle(psFilePath)
{
    GetPropertiesFromExistingRaster(psFilePath);
}

//RasterMeta::RasterMeta(Raster * pRaster) :
//    ExtentRectangle(pRaster->YOrigin(), pRaster->XOrigin(), pRaster->YSize(), pRaster->XSize(),pRaster->CellHeight(), pRaster->CellWidth())
//{
//    Init(pRaster->GetNoData(), pRaster->GetGDALDriver(), pRaster->GetGDALDataType());
//}

RasterMeta::RasterMeta(RasterMeta &source) : ExtentRectangle(source)
{
    Init(source.GetNoData(), source.GetGDALDriver(), source.GetGDALDataType());
}

void RasterMeta::Init(double fNoData, const char * psDriver, GDALDataType eDataType)
{
    m_fNoData = fNoData;
    m_psGDALDriver = (char *) malloc(strlen(psDriver) * sizeof(char)+1);
    m_eDataType = eDataType;
}

void RasterMeta::operator=(RasterMeta &source)
{
    free((void *) m_psGDALDriver);
    Init(source.GetNoData(), source.GetGDALDriver(), source.GetGDALDataType());
}


void RasterMeta::GetPropertiesFromExistingRaster(const char * psFilePath)
{
    GDALAllRegister();

    // Open the original dataset
    GDALDataset * pDS = (GDALDataset*) GDALOpen(psFilePath, GA_ReadOnly);
    if (pDS  == NULL)
        throw "error opening raster file";

    int nSuccess;
    m_fNoData = pDS->GetRasterBand(1)->GetNoDataValue(&nSuccess);
    if (nSuccess == 0)
        m_fNoData = DEFAULT_NO_DATA;

    GDALClose(pDS);
    //GDALDestroyDriverManager();
}

} // RasterManager
