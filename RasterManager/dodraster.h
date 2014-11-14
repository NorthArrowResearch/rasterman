#ifndef DOD_H
#define DOD_H

#include "rastermanager_global.h"
#include "raster.h"

namespace RasterManager {

/**
 * @brief Represents a DEM of difference raster
 *
 * A DEM of Differenace (DoD) raster is the simple
 * subtraction of two digital elevation models (DEMs). Note that this class
 * can be used to represent a raw or thresholded DoD. The main purpose of this
 * class is to provide access to the methods that calculate the statistics
 * describing the amount of change between the rasters.
 */
class DLL_API DoDRaster : Raster
{
public:
    /**
     * @brief Creates a new instance of a DoD raster.
     *
     * @param psFilePath Full, absolute file path to the [GDAL](http://www.gdal.org/) compatible DoD raster.
     *
     * Note that this can be a raw or thresholded DoD raster.
     */
    DoDRaster(const char * psFilePath);

    /**
     * @brief Retrieve the four core DoD change statistic values.
     *
     * All returned values are in the same units as the DoD raster. So if the raster
     * linear units are metres, then the areas will be in metres squared, and the volumes
     * will be in metres cubed.
     *
     * @param fAreaErosion Area of erosion (output).
     * @param fAreaDeposition Area of deposition (output).
     * @param fVolErosion Volume of erosion (output).
     * @param fVolDeposition Volume of deposition (output).
     */
    void GetChangeStats(double & fAreaErosion, double & fAreaDeposition, double & fVolErosion, double & fVolDeposition);
    /**
     * @brief Retrieve the volume of erosion and deposition just for areas where the argument mask raster are valid.
     *
     * The volume of erosion and deposition are calculated for areas where the mask raster has data (i.e. areas in
     * the mask with NoData are ignored in the calculations.)
     *
     * All returned values are in the same units as the DoD raster. So if the raster
     * linear units are metres, then the areas will be in metres squared, and the volumes
     * will be in metres cubed.
     *
     * @param pMask Mask raster specifying the areas to include in the calculation.
     * @param fVolErosion Volume of erosion (output).
     * @param fVolDeposition Volume of deposition (output).
     */
    void GetChangeStats(Raster & pMask, double & fVolErosion, double & fVolDeposition);
};

}

/*******************************************************************************************************
 *******************************************************************************************************
 * DoD Change Statistics Methods
 */

extern "C" DLL_API void GetDoDMinLoDStats(const char * ppszRawDoD, double fThreshold,
                                                     double * fAreaErosionRaw, double * fAreaDepositonRaw,
                                                     double * fAreaErosionThr, double * fAreaDepositionThr,
                                                     double * fVolErosionRaw, double * fVolDepositionRaw,
                                                     double * fVolErosionThr, double * fVolDepositionThr,
                                                     double * fVolErosionErr, double * fVolDepositonErr);


extern "C" DLL_API void GetDoDPropStats(const char * ppszRawDoD, const char * ppszPropError,
                                                     double * fAreaErosionRaw, double * fAreaDepositonRaw,
                                                     double * fAreaErosionThr, double * fAreaDepositionThr,
                                                     double * fVolErosionRaw, double * fVolDepositionRaw,
                                                     double * fVolErosionThr, double * fVolDepositionThr,
                                                     double * fVolErosionErr, double * fVolDepositonErr);



extern "C" DLL_API void GetDoDProbStats(const char * ppszRawDoD, const char * ppszThrDod,
                                                      const char * ppszPropError,
                                                      double * fAreaErosionRaw, double * fAreaDepositonRaw,
                                                      double * fAreaErosionThr, double * fAreaDepositionThr,
                                                      double * fVolErosionRaw, double * fVolDepositionRaw,
                                                      double * fVolErosionThr, double * fVolDepositionThr,
                                                      double * fVolErosionErr, double * fVolDepositonErr);


#endif // DOD_H
