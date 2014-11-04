#ifndef RASTERMANENGINE_H
#define RASTERMANENGINE_H

#include <QString>

namespace RasterManager {

class RasterManEngine
{
public:
    RasterManEngine(int argc, char * argv[]);

private:
    /**
     * @brief RasterProperties
     * @param argc
     * @param argv
     */
    void RasterProperties(int argc, char *argv[]);
    /**
     * @brief RasterCopy
     * @param argc
     * @param argv
     */
    void RasterCopy(int argc, char * argv[]);
    /**
     * @brief RasterAdd
     * @param argc
     * @param argv
     */
    void RasterAdd(int argc, char * argv[]);
    /**
     * @brief RasterSubtract
     * @param argc
     * @param argv
     */
    void RasterSubtract(int argc, char * argv[]);
    /**
     * @brief RasterDivide
     * @param argc
     * @param argv
     */
    void RasterDivide(int argc, char * argv[]);
    /**
     * @brief RasterMultiply
     * @param argc
     * @param argv
     */
    void RasterMultiply(int argc, char * argv[]);
    /**
     * @brief RasterPower
     * @param argc
     * @param argv
     */
    void RasterPower(int argc, char * argv[]);
    /**
     * @brief RasterSqrt
     * @param argc
     * @param argv
     */
    void RasterSqrt(int argc, char * argv[]);
    /**
     * @brief Mosaic
     * @param argc
     * @param argv
     */
    void Mosaic(int argc, char *argv[]);
    /**
     * @brief DoDThresholdPropError
     * @param argc
     * @param argv
     */

    void BiLinearResample(int argc, char * argv[]);
    /**
     * @brief DoDMinLoD
     * @param argc
     * @param argv
     */

    void Slope(int argc, char * argv[]);
    /**
     * @brief GetFile
     * @param argc
     * @param argv
     * @param nIndex
     * @param bMustExist
     * @return
     */

    void CSVToRaster(int argc, char * argv[]);

    /**
     * @brief Mask
     * @param argc
     * @param argv
     */
    void Mask(int argc, char *argv[]);

    /**
     * @brief GetFile
     * @param argc
     * @param argv
     * @param nIndex
     * @param bMustExist
     * @return
     */
    QString GetFile(int argc, char *argv[], int nIndex, bool bMustExist);

    /**
     * @brief GetFile
     * @param sFile
     * @param bMustExist
     * @return
     */
    QString GetFile(QString sFile, bool bMustExist);
    /**
     * @brief GetInteger
     * @param argc
     * @param argv
     * @param nIndex
     * @return
     */
    int GetInteger(int argc, char * argv[], int nIndex);
    /**
     * @brief GetDouble
     * @param argc
     * @param argv
     * @param nIndex
     * @return
     */
    double GetDouble(int argc, char * argv[], int nIndex);
    /**
     * @brief GetOutputRasterProperties
     * @param fLeft
     * @param fTop
     * @param nRows
     * @param nCols
     * @param fCellSize
     * @param argc
     * @param argv
     */
    void GetOutputRasterProperties(double & fLeft, double & fTop, int & nRows, int & nCols, double & fCellSize, int argc, char * argv[], int nStartArg);
    /**
     * @brief MakeConcurrent
     * @param argc
     * @param argv
     */
    void MakeConcurrent(int argc, char *argv[]);

};

}

#endif // RASTERMANENGINE_H
