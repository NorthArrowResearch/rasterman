#define MY_DLL_EXPORT

#include <gdal_priv.h>
#include <gdal_alg.h>
#include <ogr_api.h>
#include <QDebug>

#include "raster.h"
#include "rastermeta.h"
#include "rastermanager.h"
#include "rastermanager_interface.h"
#include "rastermanager_exception.h"
#include <QDir>
#include <QTextStream>
#include <string>
#include <limits>
#include <math.h>

namespace RasterManager {

int Raster::VectortoRaster(const char * sVectorSourcePath,
                   const char * sRasterOutputPath,
                   const char * psFieldName,
                   RasterMeta * p_rastermeta ){

    OGRRegisterAll();

    OGRDataSource * pDSVectorInput;
    pDSVectorInput = OGRSFDriverRegistrar::Open( sVectorSourcePath, FALSE );
    if (pDSVectorInput == NULL)
        return INPUT_FILE_ERROR;

    OGRLayer * poLayer = pDSVectorInput->GetLayer(0);

    // The type of the field.
    OGRFeature * feat1 = poLayer->GetFeature(0);
    int fieldindex = feat1->GetFieldIndex(psFieldName);
    OGRFieldType fieldType = feat1->GetFieldDefnRef(fieldindex)->GetType();
    OGRFeature::DestroyFeature( feat1 );

    // The data type we're going to use for the file
    GDALDataType OutputDataType = GDT_Byte;

    // Handle field types according to their type:
    switch (fieldType) {
    case OFTString:
        OutputCSVFile(poLayer, psFieldName, sRasterOutputPath);
        break;
    case OFTInteger:
        break;
    case OFTReal:
        OutputDataType = GDT_Float64;
    default:
        throw RasterManagerException(VECTOR_FIELD_NOT_VALID, "Type of field not recognized.");
        break;
    }

    // Get our projection and set the rastermeta accordingly.
    // -------------------------------------------------------
    char *pszWKT = NULL;

    OGRSpatialReference* poSRS = poLayer->GetSpatialRef();

    poSRS->exportToWkt(&pszWKT);
    p_rastermeta->SetProjectionRef(pszWKT);
    CPLFree(pszWKT);
    OSRDestroySpatialReference(poSRS);

    // Create the output dataset for writing
    GDALDataset * pDSOutput = CreateOutputDS(sRasterOutputPath, p_rastermeta);

    // Create a list of burn-in values
    // -------------------------------------------------------


    std::vector<OGRGeometryH> ogrBurnGeometries;
    std::vector<double> dBurnValues;

    poLayer->ResetReading();
    OGRFeature * ogrFeat = poLayer->GetNextFeature();   // <------- DRMEM!!! UNADDRESSABLE ACCESS of freed memory

    while( ogrFeat != NULL ){

        OGRGeometry * ogrGeom = ogrFeat->GetGeometryRef();

        // No geometry found. Move along.
        if( ogrGeom == NULL )
        {
            OGRFeature::DestroyFeature( ogrFeat );
            continue;
        }

        OGRGeometry * geoClone = ogrGeom->clone();  // <------- DRMEM!!! UNADDRESSABLE ACCESS of freed memory AND LEAK 20 direct bytes

        // Push a clone of this geometry onto the list of shapes to burn
        ogrBurnGeometries.push_back( (OGRGeometryH) geoClone );

        if (fieldType == OFTString){
            // If it's a string type we burn the FID. The value is then placed in a CSV file
            dBurnValues.push_back( ogrFeat->GetFID() );
        }
        else {
            // If it's a float type or a byte type we write it directly.
            dBurnValues.push_back( ogrFeat->GetFieldAsDouble(psFieldName) );
        }
        // GetNextFeature() creates a clone so we must delete it.
        OGRFeature::DestroyFeature( ogrFeat );      // <------- DRMEM!!! UNADDRESSABLE ACCESS beyond heap bounds:
        ogrFeat = poLayer->GetNextFeature();        // <------- DRMEM!!! UNADDRESSABLE ACCESS of freed memory
    }

    // Do the Actual Burning of Geometries.
    // -------------------------------------------------------

    int band = 1;
    char **papszRasterizeOptions = NULL;
    papszRasterizeOptions =
        CSLSetNameValue( papszRasterizeOptions, "ALL_TOUCHED", "TRUE" );

    CPLErr err = GDALRasterizeGeometries( pDSOutput, 1, &band,
                                          ogrBurnGeometries.size(),
                                          &(ogrBurnGeometries[0]),
                                          NULL, NULL,
                                          &(dBurnValues[0]),
                                          NULL,
                                          NULL, NULL );

    if (err) { }
    ogrBurnGeometries.clear();
    dBurnValues.clear();

    // Done. Calculate stats and close file
    CalculateStats(pDSOutput->GetRasterBand(1));

    CSLDestroy(papszRasterizeOptions);
    GDALClose(pDSOutput);
    pDSVectorInput->Release();  // <------- DRMEM!!! UNADDRESSABLE ACCESS beyond heap bounds:

    PrintRasterProperties(sRasterOutputPath);

    //This is where the implementation actually goes
    return PROCESS_OK;

}

int Raster::VectortoRaster(const char * sVectorSourcePath,
                           const char * sRasterOutputPath,
                           const char * sRasterTemplate,
                           const char * psFieldName){

    RasterMeta TemplateRaster(sRasterTemplate);
    return VectortoRaster(sVectorSourcePath, sRasterOutputPath, psFieldName, &TemplateRaster);

}

int Raster::VectortoRaster(const char * sVectorSourcePath,
                           const char * sRasterOutputPath,
                           double dCellWidth,
                           const char * psFieldName){

    OGRRegisterAll();
    OGRDataSource * pDSVectorInput;
    pDSVectorInput = OGRSFDriverRegistrar::Open( sVectorSourcePath, FALSE );
    if (pDSVectorInput == NULL)
        return INPUT_FILE_ERROR;

    // Get the extents of the file before passing it off to the function that actually burns
    // the geometries
    // -------------------------------------------------------
    // Note: we're just grabbing the first layer here. If we get into needing multiple layers
    // Then we'll need to re-think this.
    OGRLayer * poLayer = pDSVectorInput->GetLayer(0);

    if (poLayer == NULL)
        return VECTOR_LAYER_NOT_FOUND;

    OGREnvelope psExtent;
    poLayer->GetExtent(&psExtent, TRUE);

    double dMaxY, dMaxX, dMinY, dMinX;
    double cellWidth =  fabs(dCellWidth);

    dMaxY = ceil(psExtent.MaxY / cellWidth) * cellWidth;
    dMaxX = ceil(psExtent.MaxX / cellWidth) * cellWidth;
    dMinY = floor(psExtent.MinY / cellWidth) * cellWidth;
    dMinX = floor(psExtent.MinX / cellWidth) * cellWidth;

    int nRows = (int)((dMaxY - dMinY) / cellWidth);
    int nCols = (int)((dMaxX - dMinX) / cellWidth);
\
    // We're going to create them without projections. The projections get set later.
    double fNoDataValue = (double) -std::numeric_limits<float>::max();
    GDALDataType nDType = GDT_Float32;
    double dCellHeight = -dCellWidth;
    RasterMeta TemplateRaster(psExtent.MaxY, psExtent.MinX, nRows, nCols, &dCellHeight, &dCellWidth, &fNoDataValue, "GTiff", &nDType, "");

    pDSVectorInput->Release();
    return VectortoRaster(sVectorSourcePath, sRasterOutputPath, psFieldName, &TemplateRaster);

}

void Raster::OutputCSVFile(OGRLayer * poLayer, const char * psFieldName, const char * sRasterOutputPath){

    // use the filename with CSV added onto the end.
    QFileInfo sOutputFileInfo(sRasterOutputPath);
    QDir sNewDir = QDir(sOutputFileInfo.absolutePath());
    QString sCSVFullPath = sNewDir.filePath(sOutputFileInfo.completeBaseName() + ".csv");

    QFile csvFile(sCSVFullPath);

    if (csvFile.open(QFile::WriteOnly|QFile::Truncate))
    {
      QTextStream stream(&csvFile);

      // Write CSV file header
      stream << "\"index\", " << "\""<< psFieldName << "\""<< "\n"; // this writes first line with two columns

      OGRFeature *poFeature;
      poLayer->ResetReading();
      poLayer->SetSpatialFilter(NULL);
      while( (poFeature = poLayer->GetNextFeature()) != NULL){
          const char * sFieldVal = poFeature->GetFieldAsString(psFieldName);
          //  write line to file
          stream << poFeature->GetFID() << ", " << "\"" << sFieldVal << "\"" << "\n"; // this writes first line with two columns
          OGRFeature::DestroyFeature( poFeature );
      }

      csvFile.close();
    }
    return;

}


}
