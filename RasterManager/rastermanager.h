#ifndef TOOLS_H
#define TOOLS_H

#include "rastermanager_global.h"
#include "gdal_priv.h"
#include <QString>

namespace RasterManager {

/**
 * @brief CalculateStats
 * @param pRasterBand
 */
void CalculateStats(GDALRasterBand * pRasterBand);

/**
 * @brief CheckFile
 * @param sFile
 * @param bMustExist
 */
void DLL_API CheckFile(QString sFile, bool bMustExist);

/**
 * @brief LibCheck
 */
void DLL_API LibCheck();


}

#endif // TOOLS_H
