#ifndef RASTER_H
#define RASTER_H

#include "rastermeta.h"
#include "rastermanager_global.h"

//#include "cpl_list.h"
#include "gdal_priv.h"
//#include "ogrsf_frmts.h"

//class OGRPolygon;

class GDALRasterBand;

namespace RasterManager {

/**
 * @brief Represents a [GDAL](http://www.gdal.org/) compatible raster on disk
 *
 * This is the main class that provides methods for reading an existing raster
 * data source on disk. The raster must be in [GDAL](http://www.gdal.org/) compatible format.
 * The members and methods of this class provide access to the common properties of a
 * file based raster such as columns, rows etc.
 */
class DLL_API Raster : public RasterMeta
{
public:
    /**
     * @brief Construct a raster from a full, absolute file path to a raster.
     *
     * @param psFilePath Full, absolute file path to a GDAL compatible raster.
     *
     * Note that before creating a raster object that you must have called the GDAL
     * Driver Register. This is exposted from the RasterManager library as an
     * external C method.
     */
    Raster(const char * psFilePath);

    /**
     * @brief Construct a raster from a folder path and a file name.
     *
     * @param psFolder Full, absolute path to the folder containing the raster.
     * @param psFile File name of the GDAL compatible raster. Includes file extension.
     */
    //Raster(const char * psFolder, const char * psFile);

    /**
     * @brief Gets the full, absolute file path to the raster file.
     *
     * @return Returns the full, absolute file path to the raster file.
     */
    inline const char * FilePath() const {return m_sFilePath;}

    /**
     * @brief Prints the basic properties (rows, cols etc) about the raster to the standard output
     *
     * Note that the exported RasterProperties() method from the RasterManager library exposes
     * many of the same raster properties to calling assemblies.
     */
    void GetInfo();

    /**
     * @brief Copy constructor that creates a new raster object from an existing one.
     *
     * @param src Existing raster object.
     */
    Raster(Raster &src);

    /**
     * @brief operator = Assignment operator
     *
     * @param src Existing raster object
     * @return Updated raster object with the properties from the argument raster.
     */
    Raster& operator= (Raster& src);

    /**
     * @brief Destructor for cleaning up memory associated with a raster object.
     *
     */
    virtual ~Raster();

    /**
     * @brief Retrieve the X and Y block size of the raster.
     *
     * This is the size of the block that GDAL prescribes for reading and writing
     * the raster data and on which operations should be performed. The block size
     * might be the entire raster for small files on computers with lots of resources.
     * Or it might be a subset of the raster if the file is large.
     *
     * @param xBlockSize Returned number of cells horizontally across the block.
     * @param yBlockSize Returned number of cells vertically down the block.
     */
    void BlockSize(int& xBlockSize, int& yBlockSize) const;

    //OGRPolygon* Overlap(Raster& ds) const;
    //void Overlap(OGRPolygon* poly) const;
    /**
     * @brief
     *
     * @param xSize
     * @param ySize
     */
    void Size(int& xSize, int& ySize);

    /**
     * @brief HasNoDataValue
     * @return
     */
    inline bool HasNoDataValue() const {return !(hasNoData == 0);}

    /**
     * @brief ReSample
     * @param pOutputRaster
     * @param fNewCellSize
     * @param fNewLeft
     * @param fNewTop
     * @param nNewRows
     * @param nNewCols
     * @return
     */
    int ReSample(const char * pOutputRaster, double fNewCellSize,
                 double fNewLeft, double fNewTop, int nNewRows, int nNewCols);

    /**
     * @brief Copy
     * @param pOutputRaster
     * @param fNewCellSize
     * @param fLeft
     * @param fTop
     * @param nRows
     * @param nCols
     * @return
     */
    int Copy(const char * pOutputRaster, double fNewCellSize, double fLeft, double fTop, int nRows, int nCols);

    /**
     * @brief Slope
     * @param pOutputRaster
     * @param bDegrees
     * @return
     */
    int Slope(const char * pOutputRaster, bool bDegrees);

    /**
     * @brief CSVtoRaster
     * @param sCSVSourcePath
     * @param rasterOutputPath
     * @param nXField
     * @param nYField
     * @param sFieldName
     * @param nTop
     * @param nLeft
     * @param nRows
     * @param nCols
     * @param dResolution
     * @param gdalDriver
     * @param noDataValue
     */
    static void CSVtoRaster(const char *sCSVSourcePath, const char *rasterOutputPath,
                     int nXField, int nYField, const char *sFieldName, int nTop, int nLeft, int nRows, int nCols, double dResolution, GDALDriver gdalDriver, double noDataValue);

    /**
     * @brief CSVtoRaster
     * @param sCSVSourcePath
     * @param rasterOutputPath
     * @param sXField
     * @param sYField
     * @param sFieldName
     * @param p_rastermeta
     */
    static void CSVtoRaster(const char *sCSVSourcePath, const char *rasterOutputPath,
                     const char *sXField, const char *sYField, const char *sFieldName, RasterMeta *p_rastermeta);


    static void CSVCellClean(std::string &value);
protected:

    /**
     * @brief
     *
     * @param src
     */
    void CopyObject(Raster & src);
    /**
     * @brief
     *
     */
    void Dispose();
    /**
     * @brief
     *
     */
    void Init();
    /**
     * @brief
     *
     * @param bFullImage
     */
    void Init(bool bFullImage);

private:
    char * m_sFilePath; /**< TODO */
    int xBlockSize; /**< TODO */
    int yBlockSize; /**< TODO */
    int hasNoData; /**< TODO */

    double m_fXOrigin; /**< TODO */
    double m_fYOrigin; /**< TODO */

    /* These are the private implementations of the resample and copy raster routines depending on the data size
     * of the raster data values
     */
    /**
     * @brief
     *
     * @param pRBInput
     * @param pRBOutput
     * @param fNewCellSize
     * @param fNewLeft
     * @param fNewTop
     * @param nNewRows
     * @param nNewCols
     * @return int
     */
    int ReSample_Float64(GDALRasterBand * pRBInput, GDALRasterBand * pRBOutput, double fNewCellSize, double fNewLeft, double fNewTop, int nNewRows, int nNewCols);
    /**
     * @brief
     *
     * @param pRBInput
     * @param pRBOutput
     * @param fNewCellSize
     * @param fNewLeft
     * @param fNewTop
     * @param nNewRows
     * @param nNewCols
     * @return int
     */
    int ReSample_Float32(GDALRasterBand * pRBInput, GDALRasterBand * pRBOutput, double fNewCellSize, double fNewLeft, double fNewTop, int nNewRows, int nNewCols);
};

}

#endif // RASTER_H