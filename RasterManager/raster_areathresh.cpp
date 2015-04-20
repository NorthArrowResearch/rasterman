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
    std::vector<int> AreaMap;
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

    WriteArraytoRaster(psOutputRaster, &Terrain, NULL);
    return PROCESS_OK;

}

// https://en.wikipedia.org/wiki/Flood_fill
bool RasterArray::AreaThresholdWalker(size_t ID,
                                     int * CurrentFeatureID,
                                     int * pdCellsInArea,
                                     std::vector<int> * pAreaMap){


    if (Terrain.at(ID) == GetNoDataValue() || IsChecked(ID))
        return false;

    SetChecked(ID);

    std::queue<size_t> theQueue;
    theQueue.push(ID);
    while(!theQueue.empty())
    {
        size_t currID = theQueue.front();
        theQueue.pop();

        if (Terrain.at(currID) != GetNoDataValue() ){
            // Mark this feature on the map
            pAreaMap->at(currID) = * CurrentFeatureID;
            // Count this as part of the feature's total area
            (*pdCellsInArea)++;

            PopulateNeighbors(currID);
            // Now see if we have neighbours
            for (int d = DIR_NW; d <= DIR_W; d++)
            {
                eDirection dir = (eDirection)d;
                // Add any neighbour inside bounds to the queue
                // if it's not already checked.
                if ( IsDirectionValid(ID,dir) &&
                     !IsChecked(Neighbors.at(dir)) &&
                     Terrain.at(Neighbors.at(dir)) != GetNoDataValue()){
                    SetChecked(Neighbors.at(dir));
                    theQueue.push(Neighbors.at(d));
                }
            }

        }

    }
    return true;
}


}
