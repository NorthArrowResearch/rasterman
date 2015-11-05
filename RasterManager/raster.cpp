#define MY_DLL_EXPORT

#include "raster.h"
#include "rastermanager.h"
#include "rastermanager_interface.h"
#include "gdal_priv.h"
#include "rastermanager_exception.h"

#include "ogrsf_frmts.h"
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

Raster::Raster(QString sFilePath) :
    RasterMeta(sFilePath)
{
    const QByteArray qbFilePath = sFilePath.toLocal8Bit();
    m_sFilePath = (char *) malloc(sFilePath.length() * sizeof(char)+1);
    std::strcpy(m_sFilePath, qbFilePath.data());
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
        throw RasterManagerException(INPUT_FILE_NOT_VALID, CPLGetLastErrorMsg());

    GDALRasterBand * band = ds->GetRasterBand(1);

    double dRMin, dRMax, dRMean, dRStdDev;

    // Get some easy stats that GDAL gives us
    band->GetStatistics( 0 , true, &dRMin, &dRMax, &dRMean, &dRStdDev );
    m_dRasterMax = dRMax;
    m_dRasterMin = dRMin;

    m_dRasterMean = dRMean;
    m_dRasterStdDev = dRStdDev;

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
            QString sErr = QString("Invalid origin ( %1, %2 ) and size ( %5, %6 ) for file: %7")
                    .arg(GetLeft())
                    .arg(GetTop())
                    .arg(GetCols())
                    .arg(GetRows())
                    .arg(FilePath());
            throw RasterManagerException(INPUT_FILE_NOT_VALID, sErr);
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

OGRSpatialReference Raster::getDSRef(const char * pDataSource){

    const char * pSuffix = ExtractFileExt(pDataSource);

    if (pSuffix == NULL)
        return NULL;

    if (strcmp(pSuffix, ".tif") == 0 || strcmp(pSuffix, ".img") == 0){
        OGRSpatialReference poSRS;
        GDALDataset * pDS = (GDALDataset*) GDALOpen(pDataSource, GA_ReadOnly);
        const char * psProjection = pDS->GetProjectionRef();
        char * psWKT = NULL;
        psWKT = (char *) psProjection;
        poSRS.importFromWkt(&psWKT);
        GDALClose(pDS);
        return poSRS;
    }
    else if (strcmp(pSuffix, ".shp") == 0){
        OGRRegisterAll();
        OGRDataSource * pDSVectorInput;
        pDSVectorInput = OGRSFDriverRegistrar::Open( pDataSource, FALSE );
        OGRLayer * poLayer = pDSVectorInput->GetLayer(0);
        OGRSpatialReference * psSRS = poLayer->GetSpatialRef();
        return *psSRS;
    }
    else
        return NULL;

    return NULL;
}

int Raster::Delete(const char * pDeleteRaster){

    GDALDriver * pDR = GetGDALDriverManager()->GetDriverByName(GetDriverFromFileName(pDeleteRaster));
    return pDR->Delete(pDeleteRaster) == CE_Failure ? INPUT_FILE_ERROR : PROCESS_OK;
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

    SetNoDataValue(src.GetNoDataValuePtr());

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

int Raster::Copy(const char * pOutputRaster,
                 double * dNewCellSize,
                 double fLeft, double fTop, int nRows, int nCols,
                 const char * psRef,
                 const char * psUnit)
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

    double dNewCellHeight = (*dNewCellSize) * -1;

    if (psRef){
        SetProjectionRef(psRef);
    }
    if (psUnit){
        SetUnit(psUnit);
    }
    RasterMeta OutputMeta(fTop, fLeft, nRows, nCols, &dNewCellHeight,
                          dNewCellSize, GetNoDataValuePtr(), GetGDALDriver(), GetGDALDataType(), GetProjectionRef(), GetUnit() );

    // Create the output dataset for writing
    GDALDataset * pDSOutput = CreateOutputDS(pOutputRaster, &OutputMeta);

    if (pDSOutput == NULL)
        return OUTPUT_FILE_ERROR;

    if (HasNoDataValue())
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


    return PROCESS_OK;
}


int Raster::Uniform(const char * pOutputRaster, double fValue)
{

    // Open up the Input File
    GDALDataset * pInputDS = (GDALDataset*) GDALOpen(m_sFilePath, GA_ReadOnly);
    if (pInputDS == NULL)
        throw RasterManagerException( INPUT_FILE_ERROR, "Input file could not be opened");

    GDALRasterBand * pRBInput = pInputDS->GetRasterBand(1);

    // Create the output dataset for writing
    GDALDataset * pOutputDS = CreateOutputDS(pOutputRaster, this);
    GDALRasterBand * pOutputRB = pOutputDS->GetRasterBand(1);

    // Assign our buffers
    double * pInputLine = (double*) CPLMalloc(sizeof(double) * GetCols());
    double * pOutputLine = (double*) CPLMalloc(sizeof(double) * GetCols());

    // Loop over rows
    for (int i=0; i < GetRows(); i++)
    {
        // Populate the buffer
        pRBInput->RasterIO(GF_Read, 0,  i, GetCols(), 1, pInputLine, GetCols(), 1, GDT_Float64, 0, 0);

        // Loop over columns
        for (int j=0; j < GetCols(); j++)
        {
            if (pInputLine[j] != GetNoDataValue()){
                pOutputLine[j] = fValue;
            }
            else {
                pOutputLine[j] = GetNoDataValue();
            }

        }
        // Write the row
        pOutputRB->RasterIO(GF_Write, 0, i, GetCols(), 1, pOutputLine, GetCols(), 1, GDT_Float64, 0, 0 );
    }

    CPLFree(pOutputLine);
    CPLFree(pInputLine);

    CalculateStats(pOutputDS->GetRasterBand(1));

    if ( pInputDS != NULL)
        GDALClose(pInputDS);
    if ( pOutputDS != NULL)
        GDALClose(pOutputDS);

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

    pDR = GetGDALDriverManager()->GetDriverByName(GetDriverFromFileName(pOutputRaster));
    if (strcmp( pDR->GetDescription() , "GTiff") == 0){
        papszOptions = CSLSetNameValue(papszOptions, "COMPRESS", "LZW");
    }
    else {
        papszOptions = CSLSetNameValue(papszOptions, "COMPRESS", "PACKBITS");
    }

    GDALDataset * pDSOutput = pDR->Create(pOutputRaster, nNewCols, nNewRows, 1,  *GetGDALDataType(), papszOptions);
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

    return PROCESS_OK;
}





}
