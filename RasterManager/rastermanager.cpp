#define MY_DLL_EXPORT

#include "gdal_priv.h"
#include "rastermanager_exception.h"
#include "rastermanager_global.h"
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QDebug>

namespace RasterManager {

void CalculateStats(GDALRasterBand * pRasterBand){
    pRasterBand->ComputeStatistics(0, NULL, NULL, NULL, NULL, NULL, NULL);
}

void RM_DLL_API LibCheck(){

}


void RM_DLL_API CheckFile(QString sFile, bool bMustExist)
{
    // Enough arguments
    if (sFile.isNull() || sFile.isEmpty())
        throw RasterManagerException(PATH_ERROR, "Command line missing a file path.");
    else
    {
        // Check if the directory the file exists in is actually there
        QDir sFilePath = QFileInfo(sFile).absoluteDir();
        if (!sFilePath.exists()){
            QString sErr = "The directory of the file you specified does not exist: " + sFilePath.absolutePath();
            throw  RasterManagerException(PATH_ERROR, sErr);
        }

        sFile = sFile.trimmed();
        sFile = sFile.replace("\"","");
        if (bMustExist)
        {
            if (!QFile::exists(sFile)){
                QString sErr = "The specified input file does not exist: " + sFile;
                throw  RasterManagerException(INPUT_FILE_ERROR, sErr);
            }
        }
        else {
            if (QFile::exists(sFile)){
                QString sErr = "The specified output file already exists." + sFile;

#ifdef QT_DEBUG
                // Delete a file if it already exists. ONLY IN DEBUG MODE
                if (QFileInfo(sFile).exists()){
                    QFile::remove(QFileInfo(sFile).absoluteFilePath());
                }
                qWarning() << sErr << QString("\n<<--:::DEBUG MODE::::-->>  Deleting existing file: %1").arg(sFile);;
#else
                throw  RasterManagerException(INPUT_FILE_ERROR, sErr);
#endif
            }
        }
    }
}


}
