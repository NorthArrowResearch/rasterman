#define MY_DLL_EXPORT

#include <gdal_priv.h>
#include <gdal_alg.h>
#include <ogrsf_frmts.h>

#include "raster.h"
#include "rastermeta.h"
#include "rastermanager.h"
#include "rastermanager_interface.h"
#include "rastermanager_exception.h"
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

    // Create the output dataset for writing
    GDALDataset * pDSOutput = CreateOutputDS(sRasterOutputPath, p_rastermeta);

    OGRDataSource * pDSVectorInput;
    pDSVectorInput = OGRSFDriverRegistrar::Open( sVectorSourcePath, FALSE );
    if (pDSVectorInput == NULL)
        return INPUT_FILE_ERROR;


    OGRLayer * poLayer = pDSVectorInput->GetLayerByName( psLayerName );

    char *pszWKT = NULL;

    poLayer->GetSpatialRef()->exportToWkt(&pszWKT);
    p_rastermeta->SetProjectionRef(pszWKT);
    CPLFree(pszWKT);

    // http://stackoverflow.com/questionsb/18384217/gdalrasterizelayers-with-all-touched-and-attribute-option
//    char** options = nullptr;

//    options = CSLSetNameValue(options, "ALL_TOUCHED", "TRUE");
//    options = CSLSetNameValue(options, "ATTRIBUTE", psFieldName);

//    int nTargetBand = 1;
//    CPLErr eErr = GDALRasterizeLayers(pDSOutput, 1, &nTargetBand, 1,
//            (OGRLayerH*)&poLayer,
//            NULL, NULL, NULL, options, NULL, NULL);

    double * pOutputLine = (double *) CPLMalloc(sizeof(double)*p_rastermeta->GetCols());

    int i, j;
    for (i = 0; i < p_rastermeta->GetRows(); i++)
    {
        for (j = 0; j < p_rastermeta->GetCols(); j++)
        {
            OGRPoint point();
            pOutputLine[j] = 1;
        }
        pDSOutput->GetRasterBand(1)->RasterIO(GF_Write, 0,  i, p_rastermeta->GetCols(), 1, pOutputLine, p_rastermeta->GetCols(), 1, GDT_Float64, 0, 0);
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
    poLayer->GetExtent(&psExtent, FALSE);

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
