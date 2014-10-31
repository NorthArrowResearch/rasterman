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

    SetProjectionRef(ds->GetProjectionRef());

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

    std::string::size_type pos;

    // First strip line endings
    pos = value.find_last_not_of('\r');
    if(pos != std::string::npos) {
      value.erase(pos + 1);
    }
    else value.erase(value.begin(), value.end());

    // first strip quotes
    pos = value.find_last_not_of('"');
    if(pos != std::string::npos) {
      value.erase(pos + 1);
      pos = value.find_first_not_of('"');
      if(pos != std::string::npos) value.erase(0, pos);
    }
    else value.erase(value.begin(), value.end());

    // Now strip white space
    pos = value.find_last_not_of(' ');
    if(pos != std::string::npos) {
      value.erase(pos + 1);
      pos = value.find_first_not_of(' ');
      if(pos != std::string::npos) value.erase(0, pos);
    }
    else value.erase(value.begin(), value.end());



}

/**
 * Open up the CSV meta file and read the top, left, rows, cols etc....
 */
void Raster::CSVtoRaster(const char * sCSVSourcePath,
                         const char * psOutput,
                         const char * sCSVMeta,
                         const char * sXField,
                         const char * sYField,
                         const char * sDataField){

    std::ifstream CSVMEtaFile(sCSVMeta);
    if (!CSVMEtaFile)
    {
        throw std::runtime_error("ERROR: couldn't open the meta csv file.");
    }

    double dLeft, dTop, dCellSize, dNoDataVal;
    int nRows, nCols;
    int nEPSGproj;

    // Read CSV file into 3 different arrays
    std::string fsLine;
    getline(CSVMEtaFile,fsLine);

    std::istringstream isLine (fsLine);

    std::string uncleanCell;
    int ncolnumber = 0;
    while(getline(isLine, uncleanCell, ',')){

        ncolnumber++;
        std::string csvItem = uncleanCell;
        Raster::CSVCellClean(csvItem);

        switch (ncolnumber) {
        case 1: dTop = std::stod(csvItem); break;
        case 2: dLeft = std::stod(csvItem); break;
        case 3: nRows = std::stoi(csvItem); break;
        case 4: nCols = std::stoi(csvItem); break;
        case 5: dCellSize = std::stod(csvItem); break;
        case 6:
            // "min" is the convention for minimum float
            if (std::strcmp(csvItem.c_str(), "min") == 0)
                dNoDataVal = (double) std::numeric_limits<float>::lowest();
            else
                dNoDataVal = std::stod(csvItem);
            break;
        case 7: nEPSGproj = std::stoi(csvItem); break;
        default: break;
        }

    }
    CSVMEtaFile.close();

    CSVtoRaster(sCSVSourcePath, psOutput,
                dTop, dLeft, nRows, nCols, dCellSize, dNoDataVal,
                nEPSGproj,
                sXField,
                sYField,
                sDataField);


}

void Raster::CSVtoRaster(const char * sCSVSourcePath,
                         const char * sOutput,
                         double dTop,
                         double dLeft,
                         int nRows,
                         int nCols,
                         double dCellWidth,
                         double dNoDataVal,
                         int nEPSGProj,
                         const char * sXField,
                         const char * sYField,
                         const char * sDataField){

    double dCellHeight = dCellWidth * -1;
    const char * psDriver = GetDriverFromFileName(sOutput);

    OGRSpatialReference oSRS;
    oSRS.importFromEPSG( nEPSGProj );

    char * sProjection = NULL;
    oSRS.exportToWkt(&sProjection);

    RasterMeta inputRasterMeta(dTop, dLeft, nRows, nCols, dCellHeight, dCellWidth,
                               dNoDataVal, psDriver, GDT_Float32, sProjection);

    CSVtoRaster(sCSVSourcePath, sOutput, sXField, sYField, sDataField, &inputRasterMeta);

}


void Raster::CSVtoRaster(const char * sCSVSourcePath,
                         const char * psOutput,
                         const char * sXField,
                         const char * sYField,
                         const char * sDataField,
                         RasterMeta * p_rastermeta){

    // Create the output dataset for writing
    GDALDataset * pDSOutput = CreateOutputDS(psOutput, p_rastermeta);

    /*****************************************************************************************
     * Loop over the output file to make sure every cell gets a value of fNoDataValue
     * Every line is the same so we can have the for loops adjacent
     */
    double * pOutputLine = (double *) CPLMalloc(sizeof(double)*p_rastermeta->GetCols());
    for (int outj = 0; outj < p_rastermeta->GetCols(); outj++){
        pOutputLine[outj] = p_rastermeta->GetNoDataValue();
    }
    for (int outi = 0; outi < p_rastermeta->GetRows(); outi++){
        pDSOutput->GetRasterBand(1)->RasterIO(GF_Write, 0,  outi, p_rastermeta->GetCols(), 1, pOutputLine, p_rastermeta->GetCols(), 1, GDT_Float64, 0, 0);
    }
    CPLFree(pOutputLine);


    // open the csv file and count the lines
    int csvNumLines = 0;
    std::ifstream inputCSVFile(sCSVSourcePath);
    if (!inputCSVFile)
    {
        throw std::runtime_error("ERROR: couldn't open csv file.");
    }

    std::string unused;
    while ( std::getline(inputCSVFile, unused) )
        ++csvNumLines;

    // Reset the pointer back to the top
    inputCSVFile.clear();
    inputCSVFile.seekg(0);

    int xcol, ycol, zcol;

    // Read CSV file into 3 different arrays
    std::string fsLine;
    int nlinenumber = 0;

    // Buffer for Writing
    if (inputCSVFile.is_open()) {
        while (std::getline(inputCSVFile,fsLine)) {

            nlinenumber++;

            // prepare our buffers
            int csvX = -1;
            int csvY = -1;
            double csvDataVal = p_rastermeta->GetNoDataValue();


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
                    if (csvItem.compare(sXField) == 0){
                        xcol = ncolnumber;
                    }
                    else if (csvItem.compare(sYField) == 0){
                        ycol = ncolnumber;
                    }
                    else if (csvItem.compare(sDataField) == 0){
                        zcol = ncolnumber;
                    }
                }
                // Not the first line read values if they apply
                else{
                    double dVal = std::stod(csvItem);
                    if (xcol == ncolnumber){
                        csvX = (int) floor((dVal - p_rastermeta->GetLeft() ) / p_rastermeta->GetCellWidth());
                    }
                    else if (ycol == ncolnumber){
                        csvY = (int) ceil((p_rastermeta->GetTop() - dVal) / p_rastermeta->GetCellHeight() * -1);
                    }
                    else if (zcol == ncolnumber){
                        csvDataVal = dVal;
                    }
                }

            }
            // here's where we need to get the correct row of the output. Replace
            if (csvX >= 0 && csvX < p_rastermeta->GetCols()
                    && csvY >=0 && csvY < p_rastermeta->GetRows() ){
                pDSOutput->GetRasterBand(1)->RasterIO(GF_Write, csvX,  csvY, 1, 1, &csvDataVal, 1, 1, GDT_Float64, 0, 0);
            }

        }
    }
    inputCSVFile.close();

    GDALClose(pDSOutput);

    PrintRasterProperties(psOutput);

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
    printf("Cell Height: %.5f\n",GetCellHeight() );
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

    GDALAllRegister();

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
        nOldRow = i + nRowTrans;

        if (nOldRow >= 0 && nOldRow < nInputRows)
        {
            pRBInput->RasterIO(GF_Read, 0, nOldRow, nInputCols, 1, pInputLine, nInputCols, 1, GDT_Float64, 0, 0);

            for (j = 0; j < nCols; j++)
            {
                nOldCol = j + nColTrans;

                if (nOldCol >=0 && nOldCol < nInputCols)
                {
                    float a = pInputLine[nOldCol];
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
                pOutputLine[j] = (float) GetNoDataValue();
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
    pDSOutput->SetProjection(GetProjectionRef());

    ReSample_Raster(pRBInput, pRBOutput, fNewCellSize, fNewLeft, fNewTop, nNewRows, nNewCols);

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
