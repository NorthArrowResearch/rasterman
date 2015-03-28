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
    std::vector<int> AreaMap;
    QHash<int, double> AreaFeatures;

    AreaMap.resize(GetTotalCells());

    // This for loop makes sure we touch every cell.
    int CurrentArea = 1;
    for (size_t i = 0; i < GetTotalCells(); i++){
        if ( AreaThresholdWalker(i, CurrentArea, &AreaFeatures, &AreaMap) ){
            CurrentArea++;
            AreaFeatures.insert(CurrentArea,0);
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
        if (!AreaFeatures.contains(AreaMap.at(i))){
            Terrain.at(i) = GetNoDataValue();
        }
    }

    WriteArraytoRaster(psOutputRaster, &Terrain);
    return PROCESS_OK;

}

bool RasterArray::AreaThresholdWalker(size_t ID,
                                     int CurrentFeature,
                                     QHash<int, double> * pAreaFeatures,
                                     std::vector<int> * pAreaMap){

    // Return if we've been here already
    if (Checked.at(ID))
        return false;

    Checked.at(ID) = true;

    // This is a dead end if it's Nodata. Check it off an return
    if (Terrain.at(ID) == GetNoDataValue())
        return false;

    // Mark this feature on the map
    pAreaMap->at(ID) = CurrentFeature;
    // Count this as part of the feature's total area
    pAreaFeatures->find(CurrentFeature).value()++;

    // Now see if we have neighbours
    GetNeighbors(ID);
    for (int d = DIR_NW; d <= DIR_W; d++)
    {
        if ( Neighbors.at(d) != ENTRYPOINT) {
             AreaThresholdWalker(Neighbors.at(d), CurrentFeature, pAreaFeatures, pAreaMap);
            break;
        }
    }
    return true;

}

}
