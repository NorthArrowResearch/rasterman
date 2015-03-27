#define MY_DLL_EXPORT

#include <gdal_priv.h>
#include <gdal_alg.h>
#include <QDebug>

#include "raster.h"
#include "rastermeta.h"
#include "rastermanager.h"
#include "rastermanager_interface.h"
#include "rastermanager_exception.h"

namespace RasterManager {


int Raster::AreaThreshold(const char * psInputRaster,
                                 const char * psOutputRaster,
                                 double area){

    // Import Raster
    // ------------------------------------------------------
    RasterMeta rmInputRaster(psInputRaster);
    int nNumCells = rmInputRaster.GetCols()*rmInputRaster.GetRows();

    std::vector<bool> Checked;
    std::vector<int> Areas;
    std::vector<double> Terrain;

    Checked.resize(nNumCells);
    Areas.resize(nNumCells);
    Terrain.resize(nNumCells);


}


}
