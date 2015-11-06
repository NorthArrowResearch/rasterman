#define MY_DLL_EXPORT

#include "gdal_priv.h"
#include "rastermanager_exception.h"
#include "rastermanager_global.h"
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QDebug>

namespace RasterManager {

void RM_DLL_API CalculateStats(GDALRasterBand * pRasterBand){
    pRasterBand->ComputeStatistics(0, NULL, NULL, NULL, NULL, NULL, NULL);
}

void RM_DLL_API LibCheck(){

}

/* Some basic file ops we use often. */

QString appendToBaseFileName(QString sFullFilePath, QString sAppendStr){

    QFileInfo sNewFileInfo(sFullFilePath);
    QString sFilePath = sNewFileInfo.absolutePath();
    QString sBaseName = sNewFileInfo.baseName();
    QString sSuffix = sNewFileInfo.completeSuffix();

    QString newPath = QDir(sFilePath).absoluteFilePath( sBaseName + sAppendStr + QString(".") + sSuffix);

    return newPath;
}

int GetPrecision(double num){
    int count = 0;
    num = fabs(num);
    num = num - int(num);
    while ( !qFuzzyIsNull( fabs(num) ) ){
        num = num * 10;
        num = num - int(num);
        count++;
    }
    return count;
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
