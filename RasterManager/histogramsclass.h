#ifndef HISTOGRAMCLASS_H
#define HISTOGRAMCLASS_H

#include "rastermanager_global.h"
#include <fstream>
#include <iomanip>
#include "cpl_conv.h"
#include "gdal_priv.h"

namespace RasterManager {

/**
 * @brief
 *
 */
class RM_DLL_API HistogramsClass
{
private:
    int numBins; 
    double minBin; 
    double binSize; 
    double binIncrement; // TODO:: NOT IN USER
    long* countHistogram; 
    double* areaHistogram; 
    double* volumeHistogram; 
    std::string filename; 
    std::string error; 
protected:
    /**
     * @brief
     *
     * @return bool
     */
    bool calculate(void);
    /**
     * @brief
     *
     * @param src
     */
    void copy(const HistogramsClass& src);
    /**
     * @brief
     *
     */
    void dispose(void);
    /**
     * @brief
     *
     */
    void init(void);
    /**
     * @brief
     *
     * @param filename
     */
    void init(std::string filename);
    /**
     * @brief
     *
     * @param numBins
     * @param minBin
     * @param binSize
     * @param binIncrement
     */
    void init(int numBins, double minBin, double binSize, double binIncrement);
    /**
     * @brief
     *
     * @param filename
     * @param numBins
     * @param minBin
     * @param binSize
     * @param binIncrement
     */
    void init(std::string filename, int numBins, double minBin, double binSize,
              double binIncrement);
    /**
     * @brief
     *
     * @param msg
     * @return bool
     */
    bool setErrorMsg(std::string msg);
    /**
     * @brief
     *
     * @param min
     * @param max
     * @param increment
     * @param equal
     * @param binMin
     * @param binMax
     */
    static void getEndBins(double min, double max, double increment, bool equal, double& binMin,
                           double& binMax);
public:
    /**
     * @brief
     *
     */
    HistogramsClass();
    /**
     * @brief
     *
     * @param filename
     */
    HistogramsClass(const char* filename);

    /**
     * @brief HistogramsClass
     * @param filename
     * @param numBins
     */
    HistogramsClass(const char* filename, int numBins);

    /**
     * @brief
     *
     * @param numBins
     * @param minBin
     * @param binSize
     * @param binIncrement
     */
    HistogramsClass(int numBins, double minBin, double binSize, double binIncrement);
    /**
     * @brief
     *
     * @param filename
     * @param numBins
     * @param minBin
     * @param binSize
     * @param binIncrement
     */
    HistogramsClass(const char* filename, int numBins, double minBin, double binSize,
                    double binIncrement);
    /**
     * @brief
     *
     * @param src
     */
    HistogramsClass(const HistogramsClass& src);
    /**
     * @brief
     *
     * @param src
     * @return HistogramsClass &operator
     */
    HistogramsClass& operator= (const HistogramsClass& src);
    /**
     * @brief
     *
     */
    virtual ~HistogramsClass(void);
    /**
     * @brief
     *
     * @return double
     */
    double* getAreaHistogram(void) const;
    /**
     * @brief
     *
     * @return double
     */
    double getBinIncrement(void) const;
    /**
     * @brief
     *
     * @return double
     */
    double getBinSize(void) const;
    /**
     * @brief
     *
     * @return long
     */
    long* getCountHistogram(void) const;
    /**
     * @brief
     *
     * @return const char
     */
    const char* getError(void) const;
    /**
     * @brief
     *
     * @return const char
     */
    const char* getFilename(void) const;
    /**
     * @brief
     *
     * @return double
     */
    double getMinimumBin(void) const;
    /**
     * @brief
     *
     * @return int
     */
    int getNumBins(void) const;
    /**
     * @brief
     *
     * @return double
     */
    double* getVolumeHistogram(void) const;

    /**
     * @brief
     *
     * @param filename
     * @return bool
     */
    bool writeCSV(const char* filename);
    /**
     * @brief
     *
     * @param filename
     * @param numBins
     * @param minBin
     * @param binSize
     * @param binIncrement
     * @param error
     * @return bool
     */
    static bool computeBinStats(const char* filename, int& numBins, double& minBin,
                                double& binSize, double& binIncrement, char* error=NULL);
    /**
     * @brief
     *
     * @param max
     * @return double
     */
    static double computeYInterval(double max);

    /*
         * Returns the smallest multiple of increment that is greater than n (unless n is negative, then it
         * returns the greatest multiple of increment that is smaller than n, so it's really looking at
         * distance from 0).
         */
    /**
     * @brief
     *
     * @param n
     * @param increment
     * @return double
     */
    inline static double nearestCeil(double n, double increment) {
        increment = (n >= 0) ? fabs(increment) : -fabs(increment);
        return ceil(n / increment) * increment;
    }

    /*
         * Returns the greatest multiple of increment that is smaller than n (unless n is negative, then it
         * returns the smallest multiple of increment that is greater than n, so it's really looking at
         * distance from 0).
         */
    /**
     * @brief
     *
     * @param n
     * @param increment
     * @return double
     */
    inline static double nearestFloor(double n, double increment) {
        increment = (n >= 0) ? fabs(increment) : -fabs(increment);
        return floor(n / increment) * increment;
    }


};
}

#endif // HISTOGRAMCLASS_H
