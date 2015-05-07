#ifndef RASTER_GUTPOLYGON_H
#define RASTER_GUTPOLYGON_H
#include <gdal.h>
#include <QDebug>
#include <gdal_alg.h>

#include "rastermanager_global.h"
#include "ogrsf_frmts.h"


namespace RasterManager {

class RM_DLL_API Raster2Polygon
{
public:
    Raster2Polygon();
    static int AddGut(const char *psShpFile, const char *psInput, const char *tier1, const char *tier2);

private:
    static int Initialize(const char *psShpFile, const char *psInput);
    static void CreateField(OGRLayer *poLayer, const char *psName, OGRFieldType fType);

};


}
#endif // RASTER_GUTPOLYGON_H
