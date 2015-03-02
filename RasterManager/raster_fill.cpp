#define MY_DLL_EXPORT
/*
 * Raster Fill -- Optimized Pit Removal
 *
 * MArch 2, 2015
 *
 *  This code is adapted from https://github.com/crwr/OptimizedPitRemoval
 *
 * ============== OPTIMIZED PIT REMOVAL V1.5.1 ================================
 * Stephen Jackson, Center for Research in Water Resources, University of Texas at Austin;
 * srj9@utexas.edu
 * Acknowledgements: Soille, P. (2004), Optimal removal of spurious pits in grid digital elevation models, Water Resour. Res., 40, W12509;
 * Dr. David Maidment, University of Texas at Austin;
 * Dr. David Tarboton, Utah State University;
 * TauDEM (Terrain Analysis Using Digital Elevation Models), Hydrology Research Group, Utah State University
 *
 *
*/

#include "rastermanager_interface.h"
#include "rastermanager_exception.h"
#include "raster.h"
#include "gdal.h"
#include "gdal_priv.h"
#include "rastermanager.h"

namespace RasterManager {

int Raster::RasterPitRemoval(const char * sRasterInput,
                             const char * sRasterOutput,
                             RasterManagerFillMode eMethod){



    return PROCESS_OK;

}

}
