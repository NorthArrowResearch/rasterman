#include "histogramsclass.h"
#include "rastermanager_exception.h"
#include "raster.h"
#include <iostream>
#include <QDebug>

namespace RasterManager {

/*
     * Default constructor.
     */
HistogramsClass::HistogramsClass()
{
    init();
}

/*
     * Constructor.
     * @param filename The full path and filename to the DoD to calculate histograms for.
     */
HistogramsClass::HistogramsClass(const char* filename)
{
    init(filename);
}

/*
     * Constructor.
     * @param numBins The number of bins.
     * @param minBin The minimum bin value.
     * @param binSize The size of the bins.
     * @param binIncrement The bin label increment to use when plotting the histogram. It's here
     * because I was too lazy to write more code to try to compute one that made sense -- it can be
     * up the the user.
     */
HistogramsClass::HistogramsClass(int numBins, double minBin, double binSize, double binIncrement)
{
    init(numBins, minBin, binSize, binIncrement);
}

/*
     * Constructor.
     * @param filename The full path and filename to the DoD to calculate histograms for.
     * @param numBins The number of bins.
     * @param minBin The minimum bin value.
     * @param binSize The size of the bins.
     * @param binIncrement The bin label increment to use when plotting the histogram. It's here
     * because I was too lazy to write more code to try to compute one that made sense -- it can be
     * up the the user.
     */
HistogramsClass::HistogramsClass(const char* filename, int numBins)
{

    Raster r(filename);
    double min = r.GetMinimum();
    double max = r.GetMaximum();
    if (numBins == 0)
        throw new RasterManagerException(INPUT_FILE_ERROR, "division by zero since max and min are the same.");

    double binSize = (max-min) / numBins;

    init(filename, numBins, min, binSize, 0);
}

/*
     * Constructor.
     * @param filename The full path and filename to the DoD to calculate histograms for.
     * @param numBins The number of bins.
     * @param minBin The minimum bin value.
     * @param binSize The size of the bins.
     * @param binIncrement The bin label increment to use when plotting the histogram. It's here
     * because I was too lazy to write more code to try to compute one that made sense -- it can be
     * up the the user.
     */
HistogramsClass::HistogramsClass(const char* filename, int numBins, double minBin,
                                 double binSize, double binIncrement)
{
    init(filename, numBins, minBin, binSize, binIncrement);
}

/*
     * Copy constructor.
     */
HistogramsClass::HistogramsClass(const HistogramsClass& src)
{
    copy(src);
}

/*
     * Assignment operator.
     */
HistogramsClass& HistogramsClass::operator= (const HistogramsClass& src)
{
    if (this == &src)
        return *this;
    copy(src);
    return *this;
}

/*
     * Destructor.
     */
HistogramsClass::~HistogramsClass(void)
{
    dispose();
}

/*
     * Calculate histograms.
     */
bool HistogramsClass::calculate(void)
{
    GDALDataset* ds = NULL;
    float* data = NULL;
    try
    {
        ds = (GDALDataset*)GDALOpen(filename.c_str(), GA_ReadOnly);
        if (ds == NULL)
            throw RasterManagerException(INPUT_FILE_ERROR, CPLGetLastErrorMsg());
        GDALRasterBand* band = ds->GetRasterBand(1);

        double transform[6];
        ds->GetGeoTransform(transform);
        double cellArea = fabs(transform[1] * transform[5]);
        int yRasterSize = ds->GetRasterYSize();
        int xRasterSize = ds->GetRasterXSize();

        int xBlockSize, yBlockSize;
        band->GetBlockSize(&xBlockSize, &yBlockSize);
        data = (float*)CPLMalloc(sizeof(float) * xBlockSize * yBlockSize);

        int hasNoData;
        float noData = (float)band->GetNoDataValue(&hasNoData);

        int xOffset, yOffset, xValid, yValid, x, y, i, bin;
        for (yOffset=0; yOffset<yRasterSize; yOffset+=yBlockSize)
        {
            if (yOffset + yBlockSize < yRasterSize)
                yValid = yBlockSize;
            else
                yValid = yRasterSize - yOffset;
            for (xOffset=0; xOffset<xRasterSize; xOffset+=xBlockSize)
            {
                if (xOffset + xBlockSize < xRasterSize)
                    xValid = xBlockSize;
                else
                    xValid = xRasterSize - xOffset;

                if (band->RasterIO(GF_Read, xOffset, yOffset, xValid, yValid, data, xValid,
                                   yValid, GDT_Float32, 0, 0) == CE_Failure)
                {
                    throw RasterManagerException(OTHER_ERROR, CPLGetLastErrorMsg());
                }

                if (hasNoData)
                {
                    for (y=0; y<yValid; y++)
                    {
                        i = y * xBlockSize;
                        for (x=0; x<xValid; x++)
                        {
                            if (data[i] != noData)
                            {
                                bin = (int)((data[i] - minBin) / binSize);
                                if (bin < this->numBins && bin >= 0){
                                    countHistogram[bin]++;
                                    areaHistogram[bin] += cellArea;
                                    volumeHistogram[bin] += cellArea * fabs(data[i]);
                                }
                            }
                            i++;
                        }
                    }
                }
                else
                {
                    for (y=0; y<yValid; y++)
                    {
                        i = y * xBlockSize;
                        for (x=0; x<xValid; x++)
                        {
                            bin = (int)((data[i] - minBin) / binSize);
                            countHistogram[bin]++;
                            areaHistogram[bin] += cellArea;
                            volumeHistogram[bin] += cellArea * fabs(data[i]);
                            i++;
                        }
                    }
                }
            }
        }
        GDALClose(ds);
        CPLFree(data);
        return true;
    }
    catch (RasterManagerException e)
    {
        std::cout << e.what() << std::endl;
        if (ds != NULL)
            GDALClose(ds);
        if (data != NULL)
            CPLFree(data);
        return false;
    }
}

/*
     * Computes a sensible number of bins, bin size, minimum bin value, and increment for a
     * histogram.
     * @param filename The full path and filename to the image to compute bin info for.
     * @param numBins the int to put the number of bins in.
     * @param minBin The double to put the minimum bin value in.
     * @param binSize The double to put the bin size in.
     * @param binIncrement The double to put the bin increment in.
     */
bool HistogramsClass::computeBinStats(const char* filename, int& numBins, double& minBin,
                                      double& binSize, double& binIncrement, char* error)
{
    GDALDataset* ds = (GDALDataset*)GDALOpen(filename, GA_ReadOnly);
    if (ds == NULL)
    {
        if (error != NULL)
            strcpy(error, CPLGetLastErrorMsg());
        return false;
    }
    GDALRasterBand* band = ds->GetRasterBand(1);
    double min, max;
    band->GetStatistics(false, true, &min, &max, NULL, NULL);
    GDALClose(ds);
    double range = max - min;
    binSize = nearestCeil(range, 5) / 100;
    binIncrement = nearestCeil(range, 5) / 10;
    double maxBin;
    getEndBins(min, max, binIncrement, true, minBin, maxBin);
    numBins = (int) ((maxBin - minBin) / binSize);
    return true;
}

/*
     * Computes a sensible interval for the Y axis on a histogram.
     * @param max The maximum value in the histogram.
     * @return The Y axis interval to use.
     */
double HistogramsClass::computeYInterval(double max)
{
    if (max <= 10)
        return 2;
    else if (max <= 100)
        return 10;
    else if (max <= 500)
        return 50;
    else
        return 100;
}

/*
     * Copies the object.
     * @param src The object to copy.
     */
void HistogramsClass::copy(const HistogramsClass& src)
{
    init();
    numBins = src.numBins;
    minBin = src.minBin;
    binSize = src.binSize;
    binIncrement = src.binIncrement;
    filename = std::string(src.filename);
    error = std::string(src.error);
    if (numBins > 0)
    {
        countHistogram = (long*) CPLMalloc(sizeof(long) * numBins);
        areaHistogram = (double*) CPLMalloc(sizeof(double) * numBins);
        volumeHistogram = (double*) CPLMalloc(sizeof(double) * numBins);
        for (int i=0; i<numBins; i++)
        {
            countHistogram[i] = src.countHistogram[i];
            areaHistogram[i] = src.areaHistogram[i];
            volumeHistogram[i] = src.volumeHistogram[i];
        }
    }
}

/*
     * Clean up the object.
     */
void HistogramsClass::dispose(void)
{
    if (countHistogram != NULL)
        CPLFree(countHistogram);
    if (areaHistogram != NULL)
        CPLFree(areaHistogram);
    if (volumeHistogram != NULL)
        CPLFree(volumeHistogram);
    countHistogram = NULL;
    areaHistogram = NULL;
    volumeHistogram = NULL;
}

/*
     * Get the area histogram.
     * @return Pointer to the double area histogram.
     */
double* HistogramsClass::getAreaHistogram(void) const
{
    return areaHistogram;
}

/*
     * Get the bin increment.
     * @return The bin increment.
     */
double HistogramsClass::getBinIncrement(void) const
{
    return binIncrement;
}

/*
     * Get the bin size.
     * @return The bin size.
     */
double HistogramsClass::getBinSize(void) const
{
    return binSize;
}

/*
     * Get the count histogram.
     * @return Pointer to the long count histogram.
     */
long* HistogramsClass::getCountHistogram(void) const
{
    return countHistogram;
}

/*
     * Gets the min and max bins to use based on min and max data values, where the end bins will be
     * multiples of increment. If equal is true and min < 0 and max > 0, then bins will be returned
     * such that abs(binMin) = abs(binMax).
     * @param min The minimum data value.
     * @param max The maximum data value.
     * @param increment The increment that bins must fall on (e.g. use 5 if the end bins must be
     * multiples of 5).
     * @param equal True if the min and max bins should be negatives of each other (requires min < 0
     * and max > 0).
     * @param binMin The double to put the min bin in.
     * @param binMax The double to put the max bin in.
     */
void HistogramsClass::getEndBins(double min, double max, double increment, bool equal,
                                 double& binMin, double& binMax)
{
    binMin = (min < 0) ? nearestCeil(min, increment) : nearestFloor(min, increment);
    binMax = (max >= 0) ? nearestCeil(max, increment) : nearestFloor(max, increment);
    if (equal && (binMin < 0) && (binMax > 0))
    {
        if (fabs(binMin) > fabs(binMax))
            binMax = -binMin;
        else
            binMin = -binMax;
    }
}

/*
     * Get the error message.
     */
const char* HistogramsClass::getError(void) const
{
    return error.c_str();
}

/*
     * Get the bin size.
     * @return The bin size.
     */
const char* HistogramsClass::getFilename(void) const
{
    return filename.c_str();
}

/*
     * Get the minimum bin.
     * @return The minimum bin.
     */
double HistogramsClass::getMinimumBin(void) const
{
    return minBin;
}

/*
     * Get the number of bins in the histograms.
     * @return The number of bins in the histogram.
     */
int HistogramsClass::getNumBins(void) const
{
    return numBins;
}

/*
     * Get the volume histogram.
     * @return Pointer to the double volume histogram.
     */
double* HistogramsClass::getVolumeHistogram(void) const
{
    return volumeHistogram;
}

/*
     * Initialize the object.
     */
void HistogramsClass::init(void)
{
    numBins = 0;
    filename = "";
    error = "Histograms have not been initialized.";
    countHistogram = NULL;
    areaHistogram = NULL;
    volumeHistogram = NULL;
}

/*
     * Initialize the object.
     */
void HistogramsClass::init(std::string filename)
{
    init();
    int numBins;
    double minBin, binSize, binInc;
    char* err = new char[1000];
    if (!computeBinStats(filename.c_str(), numBins, minBin, binSize, binInc, err))
        throw RasterManagerException(OTHER_ERROR, err);
    init(filename, numBins, minBin, binSize, binInc);
}

/*
     * Initialize the object.
     */
void HistogramsClass::init(int numBins, double minBin, double binSize, double binIncrement)
{
    init();
    this->numBins = numBins;
    this->minBin = minBin;
    this->binSize = binSize;
    this->binIncrement = binIncrement;
    error = "";
    countHistogram = (long*) CPLMalloc(sizeof(long) * numBins);
    areaHistogram = (double*) CPLMalloc(sizeof(double) * numBins);
    volumeHistogram = (double*) CPLMalloc(sizeof(double) * numBins);
    for (int i=0; i<numBins; i++)
    {
        countHistogram[i] = 0;
        areaHistogram[i] = 0;
        volumeHistogram[i] = 0;
    }
}

/*
     * Initialize the object.
     */
void HistogramsClass::init(std::string filename, int numBins, double minBin, double binSize,
                           double binIncrement)
{
    init(numBins, minBin, binSize, binIncrement);
    this->filename = filename;
    if (!calculate())
        throw RasterManagerException(OTHER_ERROR, error.c_str());
}

/*
     * Convenience function to set an error message and return false.
     * @param msg The error message to set.
     * @return false
     */
bool HistogramsClass::setErrorMsg(std::string msg)
{
    error = msg;
    return false;
}

/*
     * Writes a CSV containing histogram data.
     * @param filename The full path and filename of the CSV to write.
     */
bool HistogramsClass::writeCSV(const char* filename)
{
    if (numBins == 0)
        setErrorMsg("Histograms have not been calculated.");
    std::ofstream csv;
    csv.open(filename);
    if (csv.fail())
        setErrorMsg("Could not open " + std::string(filename) + " for writing.");
    csv << "Lower Range,Upper Range,Total Area," <<
           "Total Volume,Number of Cells" << std::endl;
    double min = minBin;
    double max = min + binSize;
    for (int i=0; i<numBins; i++)
    {
        csv << std::setiosflags(std::ios::fixed) << std::setprecision(3) <<
               min << "," << max << "," << areaHistogram[i] << "," << volumeHistogram[i] << "," <<
               countHistogram[i] << std::endl;
        min = max;
        max += binSize;
    }
    csv.close();
    return true;
}

}
