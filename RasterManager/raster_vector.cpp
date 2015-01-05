#define MY_DLL_EXPORT

#include <gdal_priv.h>
#include <gdal_alg.h>
#include <ogrsf_frmts.h>

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
                   const char * psLayerName,
                   const char * psFieldName,
                   RasterMeta * p_rastermeta ){

    OGRRegisterAll();

    OGRDataSource * pDSVectorInput;
    pDSVectorInput = OGRSFDriverRegistrar::Open( sVectorSourcePath, FALSE );
    if (pDSVectorInput == NULL)
        return INPUT_FILE_ERROR;

    int layerCount = pDSVectorInput->GetLayerCount();
    OGRLayer * poLayer = pDSVectorInput->GetLayerByName( psLayerName );
    int featurecount = poLayer->GetFeatureCount();
    int fieldindex = poLayer->GetFeature(0)->GetFieldIndex(psFieldName);

    // The type of the field.
    OGRFieldType fieldType = poLayer->GetFeature(0)->GetFieldDefnRef(fieldindex)->GetType();

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
    char *pszWKT = NULL;

    poLayer->GetSpatialRef()->exportToWkt(&pszWKT);
    p_rastermeta->SetProjectionRef(pszWKT);
    CPLFree(pszWKT);

    // Create the output dataset for writing
    GDALDataset * pDSOutput = CreateOutputDS(sRasterOutputPath, p_rastermeta);
    double * pOutputLine = (double *) CPLMalloc(sizeof(double)*p_rastermeta->GetCols());

    int i, j;
    for (i = 0; i < p_rastermeta->GetRows(); i++)
    {
        for (j = 0; j < p_rastermeta->GetCols(); j++)
        {
            bool FoundIntersect = FALSE;
            int counter = 0;
            OGRGeometry * XYPoint = new OGRPoint(1,1);
            while (counter < featurecount && !FoundIntersect){
                OGRFeature * psFeat = poLayer->GetFeature(counter);
                OGRGeometry * psGeom =  psFeat->GetGeometryRef();
                if (psGeom->Contains(XYPoint)){

                    FoundIntersect = TRUE;
                    // Handle field types according to their type:
                    if (fieldType == OFTString){
                        pOutputLine[j] = (double) counter;
                    }
                    else {
                        pOutputLine[j] = psFeat->GetFieldAsDouble(psFieldName);
                    }
                }

            }
            delete XYPoint;
        }
        pDSOutput->GetRasterBand(1)->RasterIO(GF_Write, 0,  i, p_rastermeta->GetCols(), 1, pOutputLine, p_rastermeta->GetCols(), 1, OutputDataType, 0, 0);
    }


    // Done. Calculate stats and close file
    CalculateStats(pDSOutput->GetRasterBand(1));

//    CSLDestroy(options);
    GDALClose(pDSOutput);
    GDALClose(pDSVectorInput);

    PrintRasterProperties(sRasterOutputPath);

    //This is where the implementation actually goes
    return PROCESS_OK;

}

int Raster::VectortoRaster(const char * sVectorSourcePath,
                           const char * sRasterOutputPath,
                           const char * sRasterTemplate,
                           const char * psLayerName,
                           const char * psFieldName){

    RasterMeta TemplateRaster(sRasterTemplate);
    return VectortoRaster(sVectorSourcePath, sRasterOutputPath, psLayerName, psFieldName, &TemplateRaster);

}

int Raster::VectortoRaster(const char * sVectorSourcePath,
                           const char * sRasterOutputPath,
                           double dCellWidth,
                           const char * psLayerName,
                           const char * psFieldName){

    OGRRegisterAll();
    OGRDataSource * pDSVectorInput;
    pDSVectorInput = OGRSFDriverRegistrar::Open( sVectorSourcePath, FALSE );
    if (pDSVectorInput == NULL)
        return INPUT_FILE_ERROR;

    OGRLayer * poLayer = pDSVectorInput->GetLayerByName( psLayerName );

    if (poLayer == NULL)
        return VECTOR_LAYER_NOT_FOUND;

    OGREnvelope psExtent;
    poLayer->GetExtent(&psExtent, TRUE);

    int nRows = (int)((psExtent.MaxY - psExtent.MinY) / fabs(dCellWidth));
    int nCols = (int)((psExtent.MaxX - psExtent.MinX) / fabs(dCellWidth));
\
    // We're going to create them without projections but the projection will need to be set int he next step.

    // For floats.
    double fNoDataValue = (double) std::numeric_limits<float>::lowest();
    RasterMeta TemplateRaster(psExtent.MaxY, psExtent.MinX, nRows, nCols, dCellWidth, dCellWidth, fNoDataValue, "GTiff", GDT_Float32, "");

    GDALClose(pDSVectorInput);
    return VectortoRaster(sVectorSourcePath, sRasterOutputPath, psLayerName, psFieldName, &TemplateRaster);

}

void Raster::OutputCSVFile(OGRLayer * poLayer, const char * psFieldName, const char * sRasterOutputPath){

    int featurecount = poLayer->GetFeatureCount();

    QFileInfo sOutputFileInfo(sRasterOutputPath);
    QDir sNewDir = QDir(sOutputFileInfo.absolutePath());
    QString csvbasename = sOutputFileInfo.completeBaseName();
    QString sCSVFullPath = sNewDir.filePath(csvbasename + ".csv");

    QFile csvFile(sCSVFullPath);

    if (csvFile.open(QFile::WriteOnly|QFile::Truncate))
    {
      QTextStream stream(&csvFile);

      //    Write CSV file header
      stream << "\"index\", " << "\""<< psFieldName << "\""<< "\n"; // this writes first line with two columns

      for (int i= 0; i< featurecount; i++){
          const char * sFieldVal = poLayer->GetFeature(i)->GetFieldAsString(psFieldName);
          //        write line to file
          stream << i << ", " << "\"" << sFieldVal << "\"" << "\n"; // this writes first line with two columns
      }

      csvFile.close();
    }
    return;

}

/**
<PolygonFilePath> <PolygonLayerName> <TemplateRasterPath> <OutputRasterPath>
<PolygonFilePath> <PolygonLayerName> <CellSizeEtc> <OutputRasterPath>

Output type should be detected.
1) If string then byte with CSV legend
2) If integer then byte with no legend
3) if other than float32

Validations:
must be polygon geometry type (and not point or line).
must have spatial reference.

**/




}
