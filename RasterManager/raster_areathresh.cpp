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

    // TODO: NEED Console application
    //       NEED C# interface

    // Is map of Areas. The value is an index to a feature.
    std::vector<size_t> AreaMap;
    QHash<int, double> AreaFeatures;

    AreaMap.resize(GetTotalCells());

    double debugCounter = 0;

    // This for loop makes sure we touch every cell.
    int CurrentFeatureID = 1;
    size_t TotalCellsInArea;
    for (size_t i = 0; i < GetTotalCells(); i++){
        if (!Checked.at(i)){
            TotalCellsInArea = 0;
            if (AreaThresholdWalker(i, &CurrentFeatureID, &TotalCellsInArea, &AreaMap)){
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
//    WriteArraytoRaster(appendToBaseFileName(psOutputRaster, "_DEBUG-Checked"), &Checked); // DEBUG ONLY

    WriteArraytoRaster(appendToBaseFileName(psOutputRaster, "_DEBUG-AreaMap"), &AreaMap); // DEBUG ONLY
    WriteArraytoRaster(psOutputRaster, &Terrain);
    return PROCESS_OK;

}

bool RasterArray::AreaThresholdWalker(size_t ID,
                                     int * CurrentFeatureID,
                                     size_t * pdCellsInArea,
                                     std::vector<size_t> * pAreaMap){

    // Return if we've been here already
    if (Checked.at(ID))
        return true;

    Checked.at(ID) = true;

    // This is a dead end if it's Nodata. Check it off and return
    if (Terrain.at(ID) == GetNoDataValue()){
        pAreaMap->at(ID) = GetNoDataValue();
        return false;
    }

    // Mark this feature on the map
    pAreaMap->at(ID) = *CurrentFeatureID;
    // Count this as part of the feature's total area
    (*pdCellsInArea)++;

    // Now see if we have neighbours
    GetNeighbors(ID);
    for (int d = DIR_NW; d <= DIR_W; d++)
    {
        if ( Neighbors.at(d) != ENTRYPOINT) {
             AreaThresholdWalker(Neighbors.at(d), CurrentFeatureID, pdCellsInArea, pAreaMap);
        }
    }
    return true;

}

}
