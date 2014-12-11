#define MY_DLL_EXPORT

#include "raster.h"
#include "rastermanager_interface.h"
#include "gdal_priv.h"

#include "ogrsf_frmts.h"
#include "helpers.h"
#include <limits>
#include <string>
#include <vector>
#include <cstring>


namespace RasterManager {

Raster::Raster(const char * psFilePath) :
    RasterMeta(psFilePath)
{
    m_sFilePath = (char *) malloc(strlen(psFilePath) * sizeof(char)+1);
    std::strcpy(m_sFilePath, psFilePath);
    Init(true);
}

/**
* Copy constructor.
*/
Raster::Raster(Raster &src) :
    RasterMeta(src)
{
    CopyObject(src);
}

/*
 * Assignment operator.
 */
Raster& Raster::operator= (Raster& src)
{
    if (this == &src)
        return *this;
    CopyObject(src);
    return *this;
}

/*
     * Destructor.
     */
Raster::~Raster()
{
    Dispose();
}

/*
     * Basic object initialization.
     */
void Raster::Init(void)
{

}

/*
     * Initializes the object, assuming that filename, origin, size, etc are already set on the
     * member variables. Makes sure the raster can be opened, sets the block size, NODATA value,
     * pixel size, and the extent of the raster. If not using the full image then makes sure that
     * the subset chosen (based on origin and size) is valid. If using the full image then sets the
     * size and origin is assumed to be 0,0 and is set in the constructor.
     * @param fullImage True if using the full image, False if using a subset.
     */
void Raster::Init(bool bFullImage)
{
    Init();
    GDALDataset * ds = (GDALDataset*) GDALOpen(m_sFilePath, GA_ReadOnly);

    if (ds == NULL)
        throw std::runtime_error(CPLGetLastErrorMsg());

    GDALRasterBand * band = ds->GetRasterBand(1);

    double dRMin, dRMax, dRMean, dRStdDev;

    band->GetStatistics( 0 , true, &dRMin, &dRMax, &dRMean, &dRStdDev );
    m_dRasterMax = dRMax;
    m_dRasterMin = dRMin;

    OGRLinearRing ring = OGRLinearRing();
    if (bFullImage)
    {
        SetCols( band->GetXSize() );
        SetRows( band->GetYSize() );

        ring.addPoint(GetLeft(), GetTop());
        ring.addPoint(GetLeft(), GetTop() + (GetCellHeight() * GetRows()));
        ring.addPoint(GetLeft() + (GetCellWidth() * GetCols()), GetTop() + (GetCellHeight() * GetRows()));
        ring.addPoint(GetLeft() + (GetCellWidth() * GetCols()), GetTop());
        ring.closeRings();
    }
    else
    {
        if ((GetLeft() + GetCols() > band->GetXSize()) || (GetTop() + GetRows() > band->GetYSize()))
        {
            throw std::runtime_error("Invalid origin (" + stringify(GetLeft()) + "," +
                              stringify(GetTop()) + " and size (" + stringify(GetCols()) + "," + stringify(GetRows()) +
                              " for file " + FilePath());
        }
        double xMapOrigin = GetLeft() + (GetLeft() * GetCellWidth());
        double yMapOrigin = GetTop() + (GetTop() * GetCellHeight());
        ring.addPoint(xMapOrigin, yMapOrigin);
        ring.addPoint(xMapOrigin, yMapOrigin + (GetCellHeight() * GetRows()));
        ring.addPoint(xMapOrigin + (GetCellWidth() * GetCols()), yMapOrigin + (GetCellHeight() * GetRows()));
        ring.addPoint(xMapOrigin + (GetCellWidth() * GetCols()), yMapOrigin);
        ring.closeRings();
    }
    GDALClose(ds);

}




/*
 * Copy the object. (Note that this simply copies the member properties
 * of the class for assigning the object to a new variable. See the
 * Copy() method for actually copying the raster dataset object to a new
 * file on disk.
*/
void Raster::CopyObject(Raster &src)
{
    Init();

    m_sFilePath = (char *) malloc(strlen(src.FilePath()) * sizeof(char)+1);
    std::strcpy(m_sFilePath, src.FilePath());

    SetTransform(src.GetTop(), src.GetLeft(), src.GetCellWidth(), src.GetCellHeight());

    SetRows(src.GetRows());
    SetCols(src.GetCols());

    SetNoDataValue(src.GetNoDataValue());

    xBlockSize = src.xBlockSize;
    yBlockSize = src.yBlockSize;
}

/*
     * Clean up the object.
     */
void Raster::Dispose()
{
    free(m_sFilePath);
}

/*
     * Gets the number of rows and columns for this dataset. This is the number actually being used,
     * not necessarily the number in the image.
     * @param xSize The int to put the number of columns into.
     * @param ySize The int to put the number of rows into.
     */
void Raster::Size(int& xSize, int& ySize)
{
    xSize = GetCols();
    ySize = GetRows();
}

int  Raster::Copy(const char *pOutputRaster,
                  double dNewCellSize,
                  double fLeft, double fTop, int nRows, int nCols)
{
    if (fLeft <=0)
        return LEFT_ERROR;

    if (fTop <=0)
        return TOP_ERROR;

    if (nRows <=0)
        return ROWS_ERROR;

    if (nCols <=0)
        return COLS_ERROR;


    // Open the original dataset
    GDALDataset * pDSOld = (GDALDataset*) GDALOpen(m_sFilePath, GA_ReadOnly);
    if (pDSOld  == NULL)
        return INPUT_FILE_ERROR;

    GDALRasterBand * pRBInput = pDSOld->GetRasterBand(1);

    /* Create the new dataset. Determine the driver from the output file extension.
     * Enforce LZW compression for TIFs. The predictor 3 is used for floating point prediction.
     * Not using this value defaults the LZW to prediction to 1 which causes striping.
     */
    char **papszOptions = NULL;
    GDALDriver * pDR = NULL;
    char * psDR = NULL;
    const char * pSuffix = ExtractFileExt(pOutputRaster);
    if (pSuffix == NULL)
        return OUTPUT_FILE_EXT_ERROR;
    else
    {
        if (strcmp(pSuffix, ".tif") == 0)
        {
            psDR = "GTiff";
            pDR = GetGDALDriverManager()->GetDriverByName(psDR);
            papszOptions = CSLSetNameValue(papszOptions, "COMPRESS", "LZW");
            //papszOptions = CSLSetNameValue(papszOptions, "PREDICTOR", "3");
        }
        else if (strcmp(pSuffix, ".img") == 0){
            psDR = "HFA";
            pDR = GetGDALDriverManager()->GetDriverByName(psDR);
        }
        else
            return OUTPUT_UNHANDLED_DRIVER;
    }

    double dNewCellHeight = dNewCellSize * -1;
    RasterMeta OutputMeta(fTop, fLeft, nRows, nCols, dNewCellHeight,
                          dNewCellSize, GetNoDataValue(), psDR, GetGDALDataType(), GetProjectionRef() );

    //const char * pC = pDR->GetDescription();
    GDALDataset * pDSOutput = pDR->Create(pOutputRaster, nCols, nRows, 1, GetGDALDataType(), papszOptions);
    CSLDestroy( papszOptions );
    if (pDSOutput == NULL)
        return OUTPUT_FILE_ERROR;

    if (GetNoDataValue() != NULL)
    {
        CPLErr er = pDSOutput->GetRasterBand(1)->SetNoDataValue(GetNoDataValue());
        if (er == CE_Failure || er == CE_Fatal)
            return OUTPUT_NO_DATA_ERROR;
    }

    pDSOutput->SetGeoTransform(OutputMeta.GetGeoTransform());
    pDSOutput->SetProjection(pDSOld->GetProjectionRef());

    int nInputCols = pRBInput->GetXSize();
    int nInputRows = pRBInput->GetYSize();

    double * pInputLine = (double *) CPLMalloc(sizeof(double)*nInputCols);
    double * pOutputLine = (double *) CPLMalloc(sizeof(double)*pDSOutput->GetRasterBand(1)->GetXSize());

    int nRowTrans = GetRowTranslation(&OutputMeta);
    int nColTrans = GetColTranslation(&OutputMeta);

    /*
    * Loop over the raster rows. Note that geographic coordinate origin is bottom left. But
    * the GDAL image anchor is top left. The cell height is negative.
    *
    * The loop is over the centres of the output raster cells. Two rows are read from the
    * input raster. The line just above the output cell and the line just below. The line
    * just above is called the "anchor" row.
    */

    int nOldRow, nOldCol;
    int i, j;

    for (i = 0; i < nRows; i++)
    {
        nOldRow = i - nRowTrans;

        if (nOldRow >= 0 && nOldRow < nInputRows)
        {
            pRBInput->RasterIO(GF_Read, 0, nOldRow, nInputCols, 1, pInputLine, nInputCols, 1, GDT_Float64, 0, 0);

            for (j = 0; j < nCols; j++)
            {
                nOldCol = j + nColTrans;

                if (nOldCol >=0 && nOldCol < nInputCols)
                {
                    pOutputLine[j] = pInputLine[nOldCol];
                }
                else
                {
                    if (HasNoDataValue()) {
                        pOutputLine[j] = GetNoDataValue();
                    }
                    else
                    {
                        pOutputLine[j] = 0;
                    }
                }
            }
        }
        else
        {
            // Outside the bounds of the input image. Loop over all cells in current output row and set to NoData.
            for (j = 0; j < nCols; j++)
            {
                pOutputLine[j] = GetNoDataValue();
            }
        }
        pDSOutput->GetRasterBand(1)->RasterIO(GF_Write, 0, i,
                                              pDSOutput->GetRasterBand(1)->GetXSize(), 1,
                                              pOutputLine,
                                              pDSOutput->GetRasterBand(1)->GetXSize(), 1,
                                              GDT_Float64, 0, 0);
    }

    CPLFree(pInputLine);
    CPLFree(pOutputLine);

    CalculateStats(pDSOutput->GetRasterBand(1));

    GDALClose(pDSOld);
    GDALClose(pDSOutput);

    GDALDumpOpenDatasets(stderr);

    PrintRasterProperties(pOutputRaster);

    return PROCESS_OK;
}


/*
 * Algorithm taken from: http://www.quantdec.com/SYSEN597/GTKAV/section9/map_algebra.htm
 *
*/
int Raster::ReSample(const char * pOutputRaster, double fNewCellSize,
                     double fNewLeft, double fNewTop, int nNewRows, int nNewCols)
{
    if (fNewCellSize <= 0)
        return CELL_SIZE_ERROR;

    if (fNewLeft <=0)
        return LEFT_ERROR;

    if (fNewTop <=0)
        return TOP_ERROR;

    if (nNewRows <=0)
        return ROWS_ERROR;

    if (nNewCols <=0)
        return COLS_ERROR;


    /*************************************************************************************************
    * Open the original dataset and retrieve its basic properties
    */
    GDALDataset * pDSOld = (GDALDataset*) GDALOpen(m_sFilePath, GA_ReadOnly);
    if (pDSOld  == NULL)
        return INPUT_FILE_ERROR;

    GDALRasterBand * pRBInput = pDSOld->GetRasterBand(1);

    /*************************************************************************************************
     * Create the new dataset. Determine the driver from the output file extension.
     * Enforce LZW compression for TIFs. The predictor 3 is used for floating point prediction.
     * Not using this value defaults the LZW to prediction to 1 which causes striping.
     */
    char **papszOptions = NULL;
    GDALDriver * pDR = NULL;
    const char * pSuffix = ExtractFileExt(pOutputRaster);
    if (pSuffix == NULL)
        return OUTPUT_FILE_EXT_ERROR;
    else
    {
        if (strcmp(pSuffix, ".tif") == 0)
        {
            pDR = GetGDALDriverManager()->GetDriverByName("GTiff");
            papszOptions = CSLSetNameValue(papszOptions, "COMPRESS", "LZW");
            //papszOptions = CSLSetNameValue(papszOptions, "PREDICTOR", "3");
        }
        else if (strcmp(pSuffix, ".img") == 0)
            pDR = GetGDALDriverManager()->GetDriverByName("HFA");
        else
            return OUTPUT_UNHANDLED_DRIVER;
    }

    GDALDataset * pDSOutput = pDR->Create(pOutputRaster, nNewCols, nNewRows, 1,  GetGDALDataType(), papszOptions);
    CSLDestroy( papszOptions );
    if (pDSOutput == NULL)
        return OUTPUT_FILE_ERROR;

    GDALRasterBand * pRBOutput = pDSOutput->GetRasterBand(1);

    if (HasNoDataValue())
    {
        CPLErr er = pRBOutput->SetNoDataValue(this->GetNoDataValue());
        if (er == CE_Failure || er == CE_Fatal)
            return OUTPUT_NO_DATA_ERROR;
    }

    double newTransform[6];
    newTransform[0] = fNewLeft;
    newTransform[1] = fNewCellSize;
    newTransform[2] = 0;
    newTransform[3] = fNewTop;
    newTransform[4] = 0;
    newTransform[5] = -1 * fNewCellSize;
    pDSOutput->SetGeoTransform(newTransform);
    pDSOutput->SetProjection(GetProjectionRef());

    ReSampleRaster(pRBInput, pRBOutput, fNewCellSize, fNewLeft, fNewTop, nNewRows, nNewCols);

    CalculateStats(pDSOutput->GetRasterBand(1));

    GDALClose(pDSOld);
    GDALClose(pDSOutput);

    std::cout << "\n\n Input Raster: --------------------\n";
    PrintRasterProperties(m_sFilePath);
    std::cout << "\n\n Output Raster: --------------------\n";
    PrintRasterProperties(pOutputRaster);

    return PROCESS_OK;
}




}
