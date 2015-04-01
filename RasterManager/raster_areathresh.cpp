#define MY_DLL_EXPORT

#include <gdal_priv.h>
#include <gdal_alg.h>
#include <QDebug>

#include "rasterarray.h"
#include "rastermeta.h"
#include "rastermanager.h"
#include "rastermanager_interface.h"
#include "rastermanager_exception.h"

namespace RasterManager {

int RasterArray::AreaThreshold(const char * psOutputRaster, double dArea){

    // Is map of Areas. The value is an index to a feature.
    std::vector<double> AreaMap;
    QHash<int, double> AreaFeatures;

    AreaMap.resize(GetTotalCells());

    // This for loop makes sure we touch every cell.
    int CurrentFeatureID = 1;
    int TotalCellsInArea;
    for (size_t ID = 0; ID < GetTotalCells(); ID++){
        if (!IsChecked(ID)){
            TotalCellsInArea = 0;
            if ( AreaThresholdWalker(ID, &CurrentFeatureID, &TotalCellsInArea, &AreaMap) ){
                // Write the accumulation of area cells to the feature hash
                AreaFeatures.insert(CurrentFeatureID, TotalCellsInArea);
                CurrentFeatureID++;
            }
        }
    }

    // Decide which features to keep because they're big enough
    QHashIterator<int, double> qhiAreaFeatures(AreaFeatures);
    while (qhiAreaFeatures.hasNext()) {
        qhiAreaFeatures.next();
        if (qhiAreaFeatures.value() * GetCellArea() >= dArea){
            AreaFeatures.remove(qhiAreaFeatures.key());
        }
    }

    // Now we loop through again and nullify all the features that are too small
    for (size_t i = 0; i < GetTotalCells(); i++){
        if (AreaFeatures.contains(AreaMap.at(i))){
            Terrain.at(i) = GetNoDataValue();
        }
    }

    GDALDataType debugdt = GDT_Float64;
    WriteArraytoRaster(appendToBaseFileName(psOutputRaster, "_DEBUG-AreaMap"), &AreaMap, &debugdt); // DEBUG ONLY
    WriteArraytoRaster(psOutputRaster, &Terrain, NULL);
    return PROCESS_OK;

}

bool RasterArray::AreaThresholdWalker(size_t ID,
                                     int * CurrentFeatureID,
                                     int * pdCellsInArea,
                                     std::vector<double> * pAreaMap){

    // Return if we've been here already
    if (IsChecked(ID))
        return false;

    SetChecked(ID);

    // This is a dead end if it's Nodata. Check it off the list and return
    if (Terrain.at(ID) == GetNoDataValue()){
        pAreaMap->at(ID) = -1;
        return false;
    }

    // Mark this feature on the map
    pAreaMap->at(ID) = * CurrentFeatureID;
    // Count this as part of the feature's total area
    (*pdCellsInArea)++;

//    qDebug() << QString("------------------------------- CELL: %1").arg(ID);
//    TestChecked(ID);
//    TestNeighbourID(ID);
//    TestNeighbourVal(ID);

    // Now see if we have neighbours
    PopulateNeighbors(ID);
    for (int d = DIR_NW; d <= DIR_W; d++)
    {
        eDirection dir = (eDirection)d;
//        if (ID == 40616){
//            TestChecked(ID);
//            bool dirval = IsDirectionValid(ID, dir);
//            qDebug()<< "hello";
//        }
        // As long as we're inside the bounds, recurse
        if ( IsDirectionValid(ID,dir) ){
            bool validneighbour = AreaThresholdWalker(Neighbors.at(d), CurrentFeatureID, pdCellsInArea, pAreaMap);
        }
    }
    return true;
}


}
