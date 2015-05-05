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

/**
 * @brief RasterArray::AreaThresholdRaster
 * @param psOutputRaster
 * @param dArea
 * @return
 */
int RasterArray::AreaThresholdRaster(const char * psOutputRaster, double dArea){

    // Is map of Areas. The value is an index to a feature.
    std::vector<int> AreaMap;
    QHash<int, double> AreaFeatures;

    FindAreas(&AreaMap, &AreaFeatures, false);

    // Decide which features to remove because they're too small
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

int RasterArray::FindAreas(std::vector<int> * AreaMap, QHash<int, double> * AreaFeatures, bool bValDelim){

    AreaMap->resize(GetTotalCells());

    // This for loop makes sure we touch every cell.
    int CurrentFeatureID = 1;
    int TotalCellsInArea;
    for (size_t ID = 0; ID < GetTotalCells(); ID++){
        if (!IsChecked(ID)){
            TotalCellsInArea = 0;
            if ( AreaWalker(ID, &CurrentFeatureID, &TotalCellsInArea, AreaMap, bValDelim) ){
                // Write the accumulation of area cells to the feature hash
                AreaFeatures->insert(CurrentFeatureID, TotalCellsInArea);
                CurrentFeatureID++;
            }
        }
    }

}

bool RasterArray::AreaWalker(size_t ID,
                             int * CurrentFeatureID,
                             int * pdCellsInArea,
                             std::vector<int> * pAreaMap,
                             bool bValDelim){


    if (Terrain.at(ID) == GetNoDataValue() || IsChecked(ID))
        return false;

    SetChecked(ID);

    std::queue<size_t> theQueue;
    theQueue.push(ID);

    while(!theQueue.empty())
    {
        size_t currID = theQueue.front();
        theQueue.pop();

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
            // if it's not already checked and it's confirmed as part of this area
            if ( IsDirectionValid(ID,dir) && !IsChecked(Neighbors.at(dir)) ){
                // NODATA is aleays an area delineator
                if ( Terrain.at(Neighbors.at(dir)) != GetNoDataValue()){
                    // if the bValDelim flag is set then different values become delineators
                    if (!bValDelim || qFuzzyCompare(Terrain.at(Neighbors.at(dir)), Terrain.at(currID)) ){
                        SetChecked(Neighbors.at(dir));
                        theQueue.push(Neighbors.at(d));
                    }
                }
            }
        }

    }
    return true;
}


}
