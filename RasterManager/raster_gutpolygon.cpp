#include "raster_gutpolygon.h"
#include "rastermanager_exception.h"
#include "rastermanager_interface.h"
#include "raster.h"
#include "rastermanager.h"

namespace RasterManager {

const int FIELDWIDTH = 64;
const int FIELDPRECISION = 15;

Raster2Polygon::Raster2Polygon()
{ }

int Raster2Polygon::Initialize(const char * psShpFile, const char * psInput)
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

    // Get the appropriate projection of the layer.
    RasterMeta inputMeta(psInput);
    const char * ref = inputMeta.GetProjectionRef();
    OGRSpatialReference poSRS(ref);

    // Create a layer in this dataset.
    OGRLayer * poLayer = poDS->CreateLayer( "point_out", &poSRS, wkbPolygon, NULL );
    if( poLayer == NULL )
        throw RasterManagerException( VECTOR_FIELD_NOT_VALID, "Creation of OGR Layer failed." );


    // Create all the fields we are going to need
    CreateField( poLayer, "Tier1", OFTString );
    CreateField( poLayer, "Tier2", OFTString );
    CreateField( poLayer, "Tier3", OFTString );
    CreateField( poLayer, "Area", OFTReal );
    CreateField( poLayer, "Orient", OFTReal );
    CreateField( poLayer, "Eccent", OFTReal );
    CreateField( poLayer, "Value", OFTReal );

    // Clean up and close everything
    OGRDataSource::DestroyDataSource( poDS );

    return PROCESS_OK;
}


void Raster2Polygon::CreateField( OGRLayer * poLayer, const char * psName, OGRFieldType fType){

    OGRFieldDefn oField( psName, fType );

    if (fType == OFTString)
        oField.SetWidth(FIELDWIDTH);
    else if(fType == OFTReal)
        oField.SetPrecision(FIELDPRECISION);

    if( poLayer->CreateField( &oField ) != OGRERR_NONE )
    {
        throw RasterManagerException( VECTOR_FIELD_NOT_VALID, "Creating vector field failed." );
    }

}

int Raster2Polygon::AddGut(const char * psShpFile,
                           const char * psInput,
                           const char * tier1,
                           const char * tier2)
{
    OGRRegisterAll();
    CheckFile(psInput, true);

    // Initialize the shp file if we haven't already
    if (!QFile::exists(psShpFile)){
        Initialize(psShpFile, psInput);
    }

    // Open the Shapefile
    const char *pszDriverName = "ESRI Shapefile";
    OGRSFDriver *poDriver = OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName(pszDriverName);

    OGRDataSource *poDS = poDriver->Open(psShpFile, true);

    // Get the raster band
    GDALDataset * pDSInput = (GDALDataset*) GDALOpen(psInput, GA_ReadOnly);
    if (pDSInput == NULL)
        throw RasterManagerException( INPUT_FILE_ERROR, "Input file could not be opened");
    GDALRasterBand * pRBInput = pDSInput->GetRasterBand(1);

    //
    OGRLayer *poLayer = poDS->GetLayer(0);


    // Make a hash of the features that are already in the shapefile
    QList<long> existingGeometry;
    for (int n = 0; n < poLayer->GetFeatureCount(); n++){
        OGRFeature * feat = poLayer->GetFeature(n);
        existingGeometry.append(feat->GetFID());
    }

    // Do our polygonizing
    const int valueField = 1; // NOTE: Counts from right to left using zero-based indexing, but be mindful that drivers like 'ESRI Shapefile', but the coords at the end.
    char **papszOptions = NULL; // We can set this using:  CSLAddString(papszOptions, "8CONNECTED=8");
    CPLErr err = GDALFPolygonize(pRBInput, pRBInput, poLayer, valueField, papszOptions, NULL, NULL);

    // Loop over features // Excluding those that were in there already
    for (int n = 0; n < poLayer->GetFeatureCount(); n++){
        OGRFeature * feat = poLayer->GetFeature(n);
        if (!existingGeometry.contains(feat->GetFID())){
            feat->SetField("Tier1", tier1);
            feat->SetField("Tier2", tier2);
            poLayer->SetFeature(feat);
        }
    }

    qDebug() << QString("Layers: %1 Features: %2").arg(poDS->GetLayerCount()).arg(poLayer->GetFeatureCount());

    OGRDataSource::DestroyDataSource( poDS );

    return PROCESS_OK;
}



}
