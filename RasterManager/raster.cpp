#define MY_DLL_EXPORT

#include "raster.h"
#include "rastermanager_interface.h"
#include "gdal_priv.h"
#include "gdalgrid.h"
#include "rmexception.h"
#include "ogrsf_frmts.h"

#include <limits>
#include <math.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

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
    GDALAllRegister();

    //pExtent = NULL;
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
    //GDALDataset * ds = (GDALDataset*) GDALOpen(FileInfo().absoluteFilePath().toStdString().c_str(), GA_ReadOnly);
    if (ds == NULL)
        throw RMException(CPLGetLastErrorMsg());

    GDALRasterBand * band = ds->GetRasterBand(1);
    band->GetBlockSize(&xBlockSize, &yBlockSize);
    SetNoDataValue( band->GetNoDataValue(&hasNoData) );
    double transform[6];
    ds->GetGeoTransform(transform);

    SetTransform(transform[3], transform[0], transform[1], transform[5]);

    SetGDALDataType(band->GetRasterDataType());

    OGRLinearRing ring = OGRLinearRing();
    if (bFullImage)
    {
        SetCols( band->GetXSize() );
        SetRows( band->GetYSize() );

        ring.addPoint(transform[0], transform[3]);
        ring.addPoint(transform[0], transform[3] + (GetCellHeight() * GetRows()));
        ring.addPoint(transform[0] + (GetCellWidth() * GetCols()), transform[3] + (GetCellHeight() * GetRows()));
        ring.addPoint(transform[0] + (GetCellWidth() * GetCols()), transform[3]);
        ring.closeRings();
    }
    else
    {
        if ((GetLeft() + GetCols() > band->GetXSize()) || (GetTop() + GetRows() > band->GetYSize()))
        {
            throw RMException("Invalid origin (" + stringify(GetLeft()) + "," +
                              stringify(GetTop()) + " and size (" + stringify(GetCols()) + "," + stringify(GetRows()) +
                              " for file " + FilePath());
        }
        double xMapOrigin = transform[0] + (GetLeft() * GetCellWidth());
        double yMapOrigin = transform[3] + (GetTop() * GetCellHeight());
        ring.addPoint(xMapOrigin, yMapOrigin);
        ring.addPoint(xMapOrigin, yMapOrigin + (GetCellHeight() * GetRows()));
        ring.addPoint(xMapOrigin + (GetCellWidth() * GetCols()), yMapOrigin + (GetCellHeight() * GetRows()));
        ring.addPoint(xMapOrigin + (GetCellWidth() * GetCols()), yMapOrigin);
        ring.closeRings();
    }
    GDALClose(ds);
    OGRPolygon* tmp = new OGRPolygon();
    tmp->addRingDirectly(&ring);
    //pExtent = (OGRPolygon*)tmp->clone();
}

void Raster::CSVCellClean(std::string & value){
    std::string::size_type pos = value.find_last_not_of('"');
    if(pos != std::string::npos) {
      value.erase(pos + 1);
      pos = value.find_first_not_of('"');
      if(pos != std::string::npos) value.erase(0, pos);
    }
    else value.erase(value.begin(), value.end());

    pos = value.find_last_not_of(' ');
    if(pos != std::string::npos) {
      value.erase(pos + 1);
      pos = value.find_first_not_of(' ');
      if(pos != std::string::npos) value.erase(0, pos);
    }
    else value.erase(value.begin(), value.end());
}


void Raster::CSVtoRaster(const char * sCSVSourcePath,
                         const char * psOutput,
                         const char * sXField,
                         const char * sYField,
                         const char * sFieldName,
                         RasterMeta * p_rastermeta){

    // Create the output dataset for writing
    GDALDataset * pDSOutput = CreateOutputDS(psOutput, GDT_Float32, true, p_rastermeta->GetNoDataValue(),
                                             p_rastermeta->GetCols(), p_rastermeta->GetRows(), NULL, NULL);

    // open the csv file and count the lines
    int csvNumLines = 0;
    std::ifstream inputCSVFile(sCSVSourcePath);
    if (!inputCSVFile)
    {
        throw "ERROR: couldn't open csv file.";
    }

    std::string unused;
    while ( std::getline(inputCSVFile, unused) )
        ++csvNumLines;

    // Reset the pointer back to the top
    inputCSVFile.seekg(0);


    double * csvX = (double*) calloc(csvNumLines, sizeof(double));
    double * csvY = (double*) calloc(csvNumLines, sizeof(double));
    double * csvZ = (double*) calloc(csvNumLines, sizeof(double));

    int xcol, ycol, zcol;

    // Read CSV file into 3 different arrays
    std::string fsLine;
    int nlinenumber = 0;

    if (inputCSVFile.is_open()) {
        while (getline(inputCSVFile,fsLine)) {

            nlinenumber++;

            std::istringstream isLine (fsLine);
            int ncolnumber = 0;
            std::string uncleanCell;
            while(getline(isLine, uncleanCell, ',')) {

                ncolnumber++;
                std::string csvItem = uncleanCell;
                Raster::CSVCellClean(csvItem);

                //Strip the quotes off the
                // First line is the header
                if (nlinenumber == 1){
                    if (csvItem.compare(sXField)){
                        xcol = ncolnumber;
                    }
                    else if (csvItem.compare(sYField)){
                        ycol = ncolnumber;
                    }
                    else if (csvItem.compare(sFieldName)){
                        zcol = ncolnumber;
                    }
                }
                // Not the first line read values if they apply
                else{
                    double dVal = std::stod(csvItem);
                    if (xcol == nlinenumber)
                        csvX[nlinenumber] = dVal;
                    else if (xcol == nlinenumber)
                        csvY[nlinenumber] = dVal;
                    else if (xcol == nlinenumber)
                        csvZ[nlinenumber] = dVal;

                }
            }
        }
    }
    inputCSVFile.close();

    // http://www.gdal.org/gdal_grid.html
    // http://www.gdal.org/gdal__alg_8h.html#a1fdef40bcdbc98eff2328b0d093d3a22

    GDALGridCreate(GGA_NearestNeighbor,
                   NULL,
                   nlinenumber,
                   csvX,
                   csvY,
                   csvZ,
                   p_rastermeta->GetLeft(),
                   p_rastermeta->GetRight(),
                   p_rastermeta->GetBottom(),
                   p_rastermeta->GetTop(),
                   p_rastermeta->GetCols(),
                   p_rastermeta->GetRows(),
                   p_rastermeta->GetGDALDataType(),
                   & pDSOutput,
                   NULL, NULL);

    GDALClose(pDSOutput);

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
    hasNoData = src.hasNoData;

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

void Raster::GetInfo()
{
    printf("\n");
    printf("Raster: %s\n", m_sFilePath);
    printf("X Origin: %0.5f\n", GetLeft() );
    printf("Y Origin: %.5f\n", GetTop());
    printf("Cell Heigh: %.5f\n",GetCellHeight() );
    printf("Cell Width: %.5f\n", GetCellWidth() );
    printf("X Size: %d\n", GetCols() );
    printf("Y Size %d\n", GetRows() );
    printf("X BLock Size: %d\n", xBlockSize);
    printf("Y Block Size: %d\n", yBlockSize);
    printf("Has NoData: %d\n",  hasNoData );
    printf("NoData Value: %f\n", GetNoDataValue());
    printf("\n");
}

int  Raster::Copy(const char *pOutputRaster,
                  double fNewCellSize,
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

    GDALAllRegister();

    // Open the original dataset
    GDALDataset * pDSOld = (GDALDataset*) GDALOpen(m_sFilePath, GA_ReadOnly);
    if (pDSOld  == NULL)
        return INPUT_FILE_ERROR;

    GDALRasterBand * pRBInput = pDSOld->GetRasterBand(1);

    // Get the spatial reference information for the original dataset.
    double oldTransform[6];
    pDSOld ->GetGeoTransform(oldTransform);
    double fOldLeft = oldTransform[0];
    double fOldYOrigin = oldTransform[3];
    double fOldCellHeight = fNewCellSize; // oldTransform[5];
    double fOldCellWidth = fNewCellSize; //oldTransform[1];

    /* Create the new dataset. Determine the driver from the output file extension.
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
//            papszOptions = CSLSetNameValue(papszOptions, "PREDICTOR", "3");
        }
        else if (strcmp(pSuffix, ".img") == 0)
            pDR = GetGDALDriverManager()->GetDriverByName("HFA");
        else
            return OUTPUT_UNHANDLED_DRIVER;
    }

    //const char * pC = pDR->GetDescription();
    GDALDataset * pDSOutput = pDR->Create(pOutputRaster, nCols, nRows, 1, GetGDALDataType(), papszOptions);
    if (pDSOutput == NULL)
        return OUTPUT_FILE_ERROR;

    if (HasNoDataValue())
    {
        CPLErr er = pDSOutput->GetRasterBand(1)->SetNoDataValue(GetNoDataValue());
        if (er == CE_Failure || er == CE_Fatal)
            return OUTPUT_NO_DATA_ERROR;
    }

    double newTransform[6];
    newTransform[0] = fLeft;
    newTransform[1] = fNewCellSize;
    newTransform[2] = 0;
    newTransform[3] = fTop;
    newTransform[4] = 0;
    newTransform[5] = -1 * fNewCellSize;

    pDSOutput->SetGeoTransform(newTransform);
    pDSOutput->SetProjection(pDSOld->GetProjectionRef());

    void * pInputLine;
    void * pOutputLine;

    switch ((GDALDataType) GetGDALDataType())
    {
    case GDT_Byte:
        pInputLine = CPLMalloc(pRBInput->GetXSize());
        pOutputLine = CPLMalloc(pDSOutput->GetRasterBand(1)->GetXSize());
        break;

    case GDT_Int16:
        pInputLine = CPLMalloc(sizeof(short int)*pRBInput->GetXSize());
        pOutputLine = CPLMalloc(sizeof(short int) *pDSOutput->GetRasterBand(1)->GetXSize());
        break;

    case GDT_Int32:
        pInputLine = CPLMalloc(sizeof(int)*pRBInput->GetXSize());
        pOutputLine = CPLMalloc(sizeof(int) *pDSOutput->GetRasterBand(1)->GetXSize());
        break;

    case GDT_Float32:
        pInputLine = CPLMalloc(sizeof(float)*pRBInput->GetXSize());
        pOutputLine = CPLMalloc(sizeof(float) *pDSOutput->GetRasterBand(1)->GetXSize());
        break;

    case GDT_Float64:
        pInputLine = CPLMalloc(sizeof(double)*pRBInput->GetXSize());
        pOutputLine = CPLMalloc(sizeof(double) *pDSOutput->GetRasterBand(1)->GetXSize());
        break;

    default:
        throw std::runtime_error("Unhandled raster data type.");
    }



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
        nOldRow = i +  (int) ( (fTop - fOldYOrigin) / oldTransform[5] );

        if (nOldRow >= 0 && nOldRow < pRBInput->GetYSize())
        {
            pRBInput->RasterIO(GF_Read, 0, nOldRow, pRBInput->GetXSize(), 1, pInputLine, pRBInput->GetXSize(), 1, GetGDALDataType(), 0, 0);

            for (j = 0; j < nCols; j++)
            {
                nOldCol = j + (int) ( (fLeft - fOldLeft) / fOldCellWidth );

                if (nOldCol >=0 && nOldCol < pRBInput->GetXSize())
                {
                    switch (GetGDALDataType())
                    {
                    case GDT_Byte:
                        ((char *) pOutputLine)[j] = ((char *) pInputLine)[nOldCol];
                        break;

                    case GDT_Int16:
                        ((short int *) pOutputLine)[j] = ((short int *) pInputLine)[nOldCol];
                        break;

                    case GDT_Int32:
                        ((int *) pOutputLine)[j] = ((int *) pInputLine)[nOldCol];
                        break;

                    case GDT_Float32:
                        ((float *) pOutputLine)[j] = ((float *) pInputLine)[nOldCol];
                        break;

                    case GDT_Float64:
                        ((double *) pOutputLine)[j] = ((double *) pInputLine)[nOldCol];
                        break;
                    }
                }
                else
                {
                    if (HasNoDataValue()) {
                        switch ( GetGDALDataType())
                        {
                        case GDT_Byte:
                            ((char *) pOutputLine)[j] = (char) GetNoDataValue();
                            break;

                        case GDT_Int16:
                            ((short int *) pOutputLine)[j] = (short int) GetNoDataValue();
                            break;

                        case GDT_Int32:
                            ((int *) pOutputLine)[j] = (int) GetNoDataValue();
                            break;

                        case GDT_Float32:
                            ((float *) pOutputLine)[j] = (float) GetNoDataValue();
                            break;

                        case GDT_Float64:
                            ((double *) pOutputLine)[j] = (double) GetNoDataValue();
                            break;
                        }
                    }
                    else
                    {
                        switch ( GetGDALDataType())
                        {
                        case GDT_Byte:
                            ((char *) pOutputLine)[j] = (char) 0;
                            break;

                        case GDT_Int16:
                            ((short int *) pOutputLine)[j] = (short int) 0;
                            break;

                        case GDT_Int32:
                            ((int *) pOutputLine)[j] = (int) 0;
                            break;

                        case GDT_Float32:
                            ((float *) pOutputLine)[j] = (float) 0;
                            break;

                        case GDT_Float64:
                            ((double *) pOutputLine)[j] = (double) 0;
                            break;
                        }
                    }
                }
            }
        }
        else
        {
            // Outside the bounds of the input image. Loop over all cells in current output row and set to NoData.
            for (j = 0; j < nCols; j++)
            {
                switch ( GetGDALDataType())
                 {
                 case GDT_Byte:
                     ((char *) pOutputLine)[j] = (char) GetNoDataValue();
                     break;

                 case GDT_Int16:
                     ((short int *) pOutputLine)[j] = (short int) GetNoDataValue();
                     break;

                 case GDT_Int32:
                     ((int *) pOutputLine)[j] = (int) GetNoDataValue();
                     break;

                 case GDT_Float32:
                     ((float *) pOutputLine)[j] = (float) GetNoDataValue();
                     break;

                 case GDT_Float64:
                     ((double *) pOutputLine)[j] = (double) GetNoDataValue();
                     break;
                 }
            }
        }
        pDSOutput->GetRasterBand(1)->RasterIO(GF_Write, 0, i, pDSOutput->GetRasterBand(1)->GetXSize(), 1,
                                              pOutputLine, pDSOutput->GetRasterBand(1)->GetXSize(), 1,
                                              GetGDALDataType(), 0, 0);
    }

    CPLFree(pInputLine);
    CPLFree(pOutputLine);

    GDALClose(pDSOld);
    GDALClose(pDSOutput);

    GDALDumpOpenDatasets(stderr);
    GDALDestroyDriverManager();

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

    GDALAllRegister();

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
            papszOptions = CSLSetNameValue(papszOptions, "PREDICTOR", "3");
        }
        else if (strcmp(pSuffix, ".img") == 0)
            pDR = GetGDALDriverManager()->GetDriverByName("HFA");
        else
            return OUTPUT_UNHANDLED_DRIVER;
    }

    GDALDataset * pDSOutput = pDR->Create(pOutputRaster, nNewCols, nNewRows, 1,  GetGDALDataType(), papszOptions);
    if (pDSOutput == NULL)
        return OUTPUT_FILE_ERROR;

    GDALRasterBand * pRBOutput = pDSOutput->GetRasterBand(1);

    if (this->HasNoDataValue())
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
    pDSOutput->SetProjection(pDSOld->GetProjectionRef());


    /* Call the particular version of the resample routine depending on
     * the type of raster data involved.
     */
    switch (GetGDALDataType())
    {
    case GDT_Byte:
    case GDT_Int32:

    case GDT_Int16:

    case GDT_Float32:
        ReSample_Float32(pRBInput, pRBOutput, fNewCellSize, fNewLeft, fNewTop, nNewRows, nNewCols);
        break;

    case GDT_Float64:
        ReSample_Float64(pRBInput, pRBOutput, fNewCellSize, fNewLeft, fNewTop, nNewRows, nNewCols);
        break;

    default:
        std::runtime_error("Unhandled raster data type.");
    };

    GDALClose(pDSOld);
    GDALClose(pDSOutput);

    return PROCESS_OK;
}


int Raster::Slope(const char *pOutputRaster, bool bDegrees)
{
    // TODO: James to put slope implementation here.

    // Remember that the DEM raster is the raster object itself.


    // Determine appropriate return values if needed.
    return 1;
}

} // Namespace
