#include <fstream>
#include <iomanip>
#include "cpl_conv.h"
#include "gdal_priv.h"
#include "rastermanager_exception.h"

namespace RasterManager {

/**
 * @brief
 *
 */
class HistogramsClass
{
private:
    int numBins; /**< TODO */
    double minBin; /**< TODO */
    double binSize; /**< TODO */
    double binIncrement; /**< TODO */
    long* countHistogram; /**< TODO */
    double* areaHistogram; /**< TODO */
    double* volumeHistogram; /**< TODO */
    std::string filename; /**< TODO */
    std::string error; /**< TODO */
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
    void init(std::string filename) throw (RasterManagerException);
    /**
     * @brief
     *
     * @param numBins
     * @param minBin
     * @param binSize
     * @param binIncrement
     */
    void init(int numBins, double minBin, double binSize, double binIncrement)
    throw (RasterManagerException);
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
              double binIncrement) throw (RasterManagerException);
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
    friend class BaseGCDClass;
    friend class MaskHistogramsClass;
    /**
     * @brief
     *
     */
    HistogramsClass(void);
    /**
     * @brief
     *
     * @param filename
     */
    HistogramsClass(const char* filename) throw (RasterManagerException);

    /**
     * @brief HistogramsClass
     * @param filename
     * @param numBins
     */
    HistogramsClass(const char* filename, int numBins) throw (RasterManagerException);

    /**
     * @brief
     *
     * @param numBins
     * @param minBin
     * @param binSize
     * @param binIncrement
     */
    HistogramsClass(int numBins, double minBin, double binSize, double binIncrement)
    throw (RasterManagerException);
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
                    double binIncrement) throw (RasterManagerException);
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
     * @param title
     * @return bool
     */
//    bool plotAreaVolume(const char* filename, const char* title);
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
