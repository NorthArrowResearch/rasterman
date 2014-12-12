#ifndef RASTERMANENGINE_H
#define RASTERMANENGINE_H

#include <QString>

namespace RasterManager {

class RasterManEngine
{
public:
    RasterManEngine();

    int Run(int argc, char *argv[]);


private:
    /**
     * @brief CheckRasterManVersion
     */
    void CheckRasterManVersion();
    /**
     * @brief RasterProperties
     * @param argc
     * @param argv
     */
    int RasterProperties(int argc, char *argv[]);
    /**
     * @brief RasterCopy
     * @param argc
     * @param argv
     */
    int RasterCopy(int argc, char * argv[]);
    /**
     * @brief RasterAdd
     * @param argc
     * @param argv
     */
    int RasterAdd(int argc, char * argv[]);
    /**
     * @brief RasterSubtract
     * @param argc
     * @param argv
     */
    int RasterSubtract(int argc, char * argv[]);
    /**
     * @brief RasterDivide
     * @param argc
     * @param argv
     */
    int RasterDivide(int argc, char * argv[]);
    /**
     * @brief RasterMultiply
     * @param argc
     * @param argv
     */
    int RasterMultiply(int argc, char * argv[]);
    /**
     * @brief RasterPower
     * @param argc
     * @param argv
     */
    int RasterPower(int argc, char * argv[]);
    /**
     * @brief RasterSqrt
     * @param argc
     * @param argv
     */
    int RasterSqrt(int argc, char * argv[]);
    /**
     * @brief Mosaic
     * @param argc
     * @param argv
     */
    int Mosaic(int argc, char *argv[]);
    /**
     * @brief DoDThresholdPropError
     * @param argc
     * @param argv
     */

    int BiLinearResample(int argc, char * argv[]);

    /**
     * @brief Slope
     * @param argc
     * @param argv
     */
    int Slope(int argc, char * argv[]);
    /**
     * @brief Hillshade
     * @param argc
     * @param argv
     */
    int Hillshade(int argc, char *argv[]);

    /**
     * @brief CSVToRaster
     * @param argc
     * @param argv
     */
    int CSVToRaster(int argc, char * argv[]);

    /**
     * @brief Mask
     * @param argc
     * @param argv
     */
    int Mask(int argc, char *argv[]);

    /**
     * @brief Mask
     * @param argc
     * @param argv
     */
    int PNG(int argc, char *argv[]);

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
    int MakeConcurrent(int argc, char *argv[]);
    /**
     * @brief isNumeric
     * @param pszInput
     * @param nNumberBase
     * @return
     */
    bool isNumeric(const char *pszInput, int nNumberBase);



};

}

#endif // RASTERMANENGINE_H
