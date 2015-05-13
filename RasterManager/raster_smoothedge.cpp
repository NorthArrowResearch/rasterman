#define MY_DLL_EXPORT
/*
 * Raster Smooth Edges --
 *
 * This algorithm works by removing cells that touch nodata vals and then adding them back in
 * The result is a smoothing of peaks along the edges.
 *
*/

#include <gdal_priv.h>
#include <gdal_alg.h>
#include <QDebug>

#include "rasterarray.h"
#include "rastermeta.h"
#include "rastermanager.h"
#include "rastermanager_interface.h"
#include "rastermanager_exception.h"


namespace RasterManager {

int RasterArray::SmoothEdge(
                            const char * psOutputRaster,
                            int nCells)
{

    std::vector<double> WorkingCopyMap;
    WorkingCopyMap.resize(GetTotalCells());

    // Copy the Terrain to a working copy
    for (size_t id = 0; id < GetTotalCells(); id++){
        WorkingCopyMap.at(id) = Terrain.at(id);
    }


    // Subtract n Cells from the edges
    for (int nCell = 0; nCell < nCells; nCell++){
        ResetChecked();
        // Subtract all cells that touch an edge. Do this nCells times
        for (size_t id = 0; id < GetTotalCells(); id++){
            if (WorkingCopyMap.at(id) != GetNoDataValue()){
                PopulateNeighbors(id);
                // Now see if we have neighbours that have a nodata value
                for (int d = DIR_NW; d <= DIR_W; d++)
                {
                    if ( IsDirectionValid(id,(eDirection)d) &&
                         WorkingCopyMap.at(Neighbors.at((eDirection) d)) == GetNoDataValue() ){

                        SetChecked(id); // Add this cell to the subtraction map
                    }
                }
            }
        }
        // Apply the mask to the Raster
        for (size_t id = 0; id < GetTotalCells(); id++){
            if (IsChecked(id))
                WorkingCopyMap.at(id) = GetNoDataValue();
        }
    }

    //  DEBUG
    //WriteArraytoRaster(appendToBaseFileName(psOutputRaster, "_DEBUG-Subtract"), &WorkingCopyMap, NULL);

    // Add it back one border pixel at a time.
    for (int nCell = 0; nCell < nCells; nCell++){
        // Subtract all cells that touch an edge. Do this nCells times
        for (size_t id = 0; id < GetTotalCells(); id++){
            if ( WorkingCopyMap.at(id) == GetNoDataValue() ){
                PopulateNeighbors(id);
                // Now see if we have neighbours
                for (int d = DIR_NW; d <= DIR_W; d++)
                {
                    eDirection dir =  (eDirection) d;
                    // We don't add back the diagonals. This is experimental.
                    if ( dir == DIR_N || dir == DIR_E || dir == DIR_S || dir == DIR_W ){
                        if ( IsDirectionValid(id,(eDirection)d) &&
                             WorkingCopyMap.at(Neighbors.at((eDirection) d)) != GetNoDataValue() ){

                            SetChecked(id);
                        }
                    }
                }
            }
        }
        // Apply the mask to the Raster and put cells back if we need to
        for (size_t id = 0; id < GetTotalCells(); id++){
            if (IsChecked(id))
                WorkingCopyMap.at(id) = Terrain.at(id);
        }
    }

    WriteArraytoRaster(psOutputRaster, &WorkingCopyMap, NULL);
    return PROCESS_OK;

}

}
