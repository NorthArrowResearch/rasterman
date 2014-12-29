#define MY_DLL_EXPORT
#include "raster.h"


namespace RasterManager {

int VectortoRaster(const char * sVectorSourcePath,
                   const char * psOutput,
                   const char * sXField,
                   const char * sYField,
                   const char * sDataField,
                   RasterMeta * p_rastermeta ){

}

int VectortoRaster(const char * sVectorSourcePath,
                   const char * psOutput,
                   const char * sRasterTemplate,
                   const char * sXField,
                   const char * sYField,
                   const char * sDataField ){

}

int VectortoRaster(const char * sVectorSourcePath,
                   const char * sOutput,
                   double dTop,
                   double dLeft,
                   int nRows,
                   int nCols,
                   double dCellWidth, double dNoDataVal,
                   const char * sXField,
                   const char * sYField,
                   const char * sDataField){

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
