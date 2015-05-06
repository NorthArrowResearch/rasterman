#include "raster_gutpolygon.h"

#include <gdal.h>
#include <QDebug>
#include <gdal_alg.h>

#include "ogrsf_frmts.h"
#include "rastermanager_exception.h"
#include "rastermanager_interface.h"
#include "raster.h"
#include "rastermanager.h"

namespace RasterManager {

const int FIELDWIDTH = 64;

Raster2Polygon::Raster2Polygon()
{


}

int Raster2Polygon::Initialize(const char * psShpFile)
{
    CheckFile(psShpFile, false);

    // Get the driver for the job
    OGRSFDriver *poDriver;
    const char *pszDriverName = "ESRI Shapefile";
    poDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(pszDriverName);

    // Create a dataset
    OGRDataSource * poDS;
    poDS = poDriver->CreateDataSource(psShpFile, NULL );
    if( poDS == NULL )
        throw RasterManagerException( OUTPUT_FILE_ERROR, "Creation of output file failed." );

    // Create a layer in this dataset.
    OGRLayer *poLayer;
    poLayer = poDS->CreateLayer( "point_out", NULL, wkbPolygon, NULL );
    if( poLayer == NULL )
        throw RasterManagerException( VECTOR_FIELD_NOT_VALID, "Creation of OGR Layer failed." );

    // Create all the fields we are going to need
    OGRFieldDefn oTier1( "Tier1", OFTString );
    OGRFieldDefn oTier2( "Tier2", OFTString );
    OGRFieldDefn oTier3( "Tier3", OFTString );
    OGRFieldDefn oArea( "Area", OFTReal );
    OGRFieldDefn oOrient( "Orientation", OFTReal );
    OGRFieldDefn oEccent( "Eccentricity", OFTString );

    oTier1.SetWidth(FIELDWIDTH);
    oTier2.SetWidth(FIELDWIDTH);
    oTier3.SetWidth(FIELDWIDTH);
    oArea.SetPrecision(10);
    oOrient.SetPrecision(10);
    oEccent.SetPrecision(10);

    if( poLayer->CreateField( &oTier1 ) != OGRERR_NONE ||
            poLayer->CreateField( &oTier2 ) != OGRERR_NONE ||
            poLayer->CreateField( &oTier3 ) != OGRERR_NONE ||
            poLayer->CreateField( &oArea ) != OGRERR_NONE ||
            poLayer->CreateField( &oOrient ) != OGRERR_NONE ||
            poLayer->CreateField( &oEccent ) != OGRERR_NONE )
    {
        throw RasterManagerException( VECTOR_FIELD_NOT_VALID, "Creating vector fields failed." );
    }

    OGRDataSource::DestroyDataSource( poDS );

    return PROCESS_OK;
}


void CreateField();

int Raster2Polygon::AddGut(const char * psShpFile, const char * psInput, const char * tier1, const char * tier2)
{
    OGRRegisterAll();
    CheckFile(psInput, true);

    // Initialize the shp file if we haven't already
    if (!QFile::exists(psShpFile)){
        Initialize(psShpFile);
    }

    const char *pszDriverName = "ESRI Shapefile";
    OGRSFDriver *poDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(pszDriverName);

    OGRDataSource *poDS = poDriver->Open(psShpFile, NULL);

    GDALDataset * pDSInput = (GDALDataset*) GDALOpen(psInput, GA_ReadOnly);
    if (pDSInput == NULL)
        throw RasterManagerException( INPUT_FILE_ERROR, "Input file could not be opened");
    GDALRasterBand * pRBInput = pDSInput->GetRasterBand(1);

    OGRLayer *poLayer = poDS->GetLayer(0);

    //    papszOptions 	a name/value list of additional options
    //    "8CONNECTED": May be set to "8" to use 8 connectedness. Otherwise 4 connectedness will be applied to the algorithm
    char **papszOptions = NULL;
    papszOptions = CSLAddString(papszOptions, "8CONNECTED=8");

    GDALPolygonize(pRBInput, NULL, 0, NULL, NULL, NULL, NULL);

    qDebug() << "booyah";

    return PROCESS_OK;
}



}
