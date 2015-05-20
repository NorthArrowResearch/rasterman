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
     * @brief RasterMath
     * @param argc
     * @param argv
     */
    int RasterMath(int argc, char * argv[]);
    /**
     * @brief RasterCopy
     * @param argc
     * @param argv
     */
    int RasterCopy(int argc, char * argv[]);
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
     * @brief Uniform
     * @param argc
     * @param argv
     * @return
     */
    int Uniform(int argc, char *argv[]);

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
     * @brief VectorToRaster
     * @param argc
     * @param argv
     * @return
     */
    int VectorToRaster(int argc, char * argv[]);

    /**
     * @brief Mask
     * @param argc
     * @param argv
     */
    int Mask(int argc, char *argv[]);

    /**
     * @brief MaskVal
     * @param argc
     * @param argv
     * @return
     */
    int MaskVal(int argc, char *argv[]);
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
     * @brief invert
     * @param argc
     * @param argv
     * @return
     */
    int invert(int argc, char *argv[]);

    /**
     * @brief normalize
     * @param argc
     * @param argv
     * @return
     */
    int normalize(int argc, char *argv[]);

    /**
     * @brief filter
     * @param argc
     * @param argv
     * @return
     */
    int filter(int argc, char *argv[]);

    /**
     * @brief extractpoints
     * @param argc
     * @param argv
     * @return
     */
    int extractpoints(int argc, char *argv[]);

    /**
     * @brief RasterToCSV
     * @param argc
     * @param argv
     * @return
     */
    int RasterToCSV(int argc, char *argv[]);

    /**
     * @brief fill
     * @param argc
     * @param argv
     * @return
     */
    int fill(int argc, char *argv[]);

    /**
     * @brief dist
     * @param argc
     * @param argv
     * @return
     */
    int dist(int argc, char *argv[]);

    /**
     * @brief stats
     * @param argc
     * @param argv
     * @return
     */
    int Stats(int argc, char *argv[]);

    /**
     * @brief StackStats
     * @param argc
     * @param argv
     * @return
     */
    int StackStats(int argc, char *argv[]);

    /**
     * @brief LinThresh
     * @param argc
     * @param argv
     * @return
     */
    int LinThresh(int argc, char *argv[]);

    /**
     * @brief Combine
     * @param argc
     * @param argv
     * @return
     */
    int Combine(int argc, char *argv[]);

    /**
     * @brief AreaThresh
     * @param argc
     * @param argv
     * @return
     */
    int AreaThresh(int argc, char *argv[]);

    /**
     * @brief SmoothEdges
     * @param argc
     * @param argv
     * @return
     */
    int SmoothEdges(int argc, char *argv[]);

    /**
     * @brief Compare
     * @param argc
     * @param argv
     * @return
     */
    int Compare(int argc, char *argv[]);

    /**
     * @brief GutPoly
     * @param argc
     * @param argv
     * @return
     */
    int GutPoly(int argc, char *argv[]);
    /**
     * @brief SetNull
     * @param argc
     * @param argv
     * @return
     */
    int SetNull(int argc, char *argv[]);

    /**
     * @brief CreateDrain
     * @param argc
     * @param argv
     * @return
     */
    int CreateDrain(int argc, char *argv[]);

};

}

#endif // RASTERMANENGINE_H
