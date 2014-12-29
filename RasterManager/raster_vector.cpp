#define MY_DLL_EXPORT

#include "gdal_priv.h"
#include "raster.h"
#include "rastermanager_interface.h"
#include "rastermanager_exception.h"

#include <limits>
#include <math.h>

namespace RasterManager {

int Raster::VectortoRaster(const char * sVectorSourcePath,
                   const char * sRasterOutputPath,
                   const char * LayerName,
                   RasterMeta * p_rastermeta ){

    //This is where the implementation actually goes
    return PROCESS_OK;
}

int Raster::VectortoRaster(const char * sVectorSourcePath,
                   const char * sRasterOutputPath,
                   const char * sRasterTemplate,
                   const char * LayerName ){

    RasterMeta TemplateRaster(sRasterTemplate);
    return VectortoRaster(sVectorSourcePath, sRasterOutputPath, LayerName, &TemplateRaster);

}

int Raster::VectortoRaster(const char * sVectorSourcePath,
                   const char * sRasterOutputPath,
                   double dCellWidth,
                   const char * LayerName){

    // TODO: Calculate the extent of the shapefile layer and
    // Create a template raster with those specs.

    RasterMeta TemplateRaster(sRasterOutputPath);
    return VectortoRaster(sVectorSourcePath, sRasterOutputPath, LayerName, &TemplateRaster);

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
