#define MY_DLL_EXPORT

#include "gdal_priv.h"
#include "raster.h"
#include "rastermanager_interface.h"
#include "rastermanager_exception.h"
#include "gdalgrid.h"
#include "helpers.h"

#include <limits>
#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>

namespace RasterManager {

int Raster::CSVtoRaster(const char * sCSVSourcePath,
                         const char * sOutput,
                         double dTop,
                         double dLeft,
                         int nRows,
                         int nCols,
                         double dCellWidth,
                         double dNoDataVal,
                         const char * sXField,
                         const char * sYField,
                         const char * sDataField){

    double dCellHeight = dCellWidth * -1;
    const char * psDriver = GetDriverFromFileName(sOutput);

    RasterMeta inputRasterMeta(dTop, dLeft, nRows, nCols, dCellHeight, dCellWidth,
                               dNoDataVal, psDriver, GDT_Float32, NULL);

    CSVtoRaster(sCSVSourcePath, sOutput, sXField, sYField, sDataField, &inputRasterMeta);

    return PROCESS_OK;
}

int Raster::CSVtoRaster(const char * sCSVSourcePath,
                         const char * psOutput,
                         const char * sRasterTemplate,
                         const char * sXField,
                         const char * sYField,
                         const char * sDataField ){

    RasterMeta inputRasterMeta(sRasterTemplate);

    int eResult = CSVtoRaster(sCSVSourcePath, psOutput, sXField, sYField, sDataField, &inputRasterMeta);

    return eResult;

}

int Raster::CSVtoRaster(const char * sCSVSourcePath,
                         const char * psOutput,
                         const char * sXField,
                         const char * sYField,
                         const char * sDataField,
                         RasterMeta * p_rastermeta){

    // Validate that the files are there
    CheckFile(sCSVSourcePath, true);

    // Create the output dataset for writing
    GDALDataset * pDSOutput = CreateOutputDS(psOutput, p_rastermeta);

    // open the csv file and count the lines
    int csvNumLines = 0;
    std::ifstream inputCSVFile(sCSVSourcePath);
    if (!inputCSVFile)
    {
        throw RasterManagerException(INPUT_FILE_ERROR, "Couldn't open csv file.");
    }

    std::string unused;
    while ( std::getline(inputCSVFile, unused) )
        ++csvNumLines;

    // Reset the pointer back to the top
    inputCSVFile.clear();
    inputCSVFile.seekg(0);

    int xcol=-1, ycol=-1, zcol=-1;

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
                    // Basic checking to make sure we have parameters
                    if (xcol == -1){
                        QString sErr = QString("X Field '%1' not found").arg(sXField);
                        throw RasterManagerException(MISSING_ARGUMENT, sErr);
                    }
                    else if (ycol == -1){
                        QString sErr = QString("Y Column '%1' not found").arg(sYField);
                        throw RasterManagerException(MISSING_ARGUMENT, sErr);
                    }
                    else if (zcol == -1){
                        QString sErr = QString("Data Column '%1' not found").arg(sDataField);
                        throw RasterManagerException(MISSING_ARGUMENT, sErr);
                    }

                    // Assign our CSV values to an appropriate place in the raster
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

    CalculateStats(pDSOutput->GetRasterBand(1));

    inputCSVFile.close();
    GDALClose(pDSOutput);

    PrintRasterProperties(psOutput);

    return PROCESS_OK;

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

}
