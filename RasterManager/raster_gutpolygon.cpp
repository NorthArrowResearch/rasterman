#include "raster_gutpolygon.h"

#include <gdal_priv.h>
#include <gdal_alg.h>
#include <ogr_api.h>
#include <QDebug>

#include "rastermanager_exception.h"
#include "rastermanager_interface.h"
#include "raster.h"
#include "rastermanager.h"

namespace RasterManager {

Raster2Polygon::Raster2Polygon()
{


}


int Raster2Polygon::Initialize(const char * psShpFile)
{
    CheckFile(psShpFile, true);
    poDS = poDriver->Create( "point_out.shp", 0, 0, 0, GDT_Unknown, NULL );
    if( poDS == NULL )
    {
        throw RasterManagerException( "Creation of output file failed." );
    }
}


int Raster2Polygon::AddGut(const char * psShpFile, const char * psInput, const char * psName)
{

    CheckFile(psInput, true);
    GDALDataset *poDS;


}



}
