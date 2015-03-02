#ifndef RASTER_H
#define RASTER_H

#include "rastermeta.h"
#include "rastermanager_global.h"
#include "rastermanager_interface.h"
#include <ogrsf_frmts.h>
#include <QString>
#include <QFile>
#include <string>

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
class RM_DLL_API Raster : public RasterMeta
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
     * @brief GetMaximum
     * @return
     */
    inline double GetMaximum() { return m_dRasterMax; }

    /**
     * @brief GetMinimum
     * @return
     */
    inline double GetMinimum() { return m_dRasterMin; }

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
    int Copy(const char * pOutputRaster, double *fNewCellSize, double fLeft, double fTop, int nRows, int nCols);

    /**
     * @brief CSVCellClean
     * @param value
     */
    static void CSVCellClean(QString &value);

    /**
     * @brief CSVWriteLine
     * @param sCSVFullPath
     * @param sCSVLine
     */
    static void CSVWriteLine(QFile *csvFile, QString sCSVLine);

    /**
     * @brief CSVtoRaster
     * @param sCSVSourcePath
     * @param psOutput
     * @param sXField
     * @param sYField
     * @param sDataField
     * @param p_rastermeta
     */
    static int CSVtoRaster(const char * sCSVSourcePath,
                           const char * psOutput,
                           const char * sXField,
                           const char * sYField,
                           const char * sDataField,
                           RasterMeta * p_rastermeta );

    /**
     * @brief CSVtoRaster
     * @param sCSVSourcePath
     * @param psOutput
     * @param sRasterTemplate
     * @param sXField
     * @param sYField
     * @param sDataField
     */
    static int CSVtoRaster(const char * sCSVSourcePath,
                           const char * psOutput,
                           const char * sRasterTemplate,
                           const char * sXField,
                           const char * sYField,
                           const char * sDataField );

    /**
     * @brief CSVtoRaster
     * @param sCSVSourcePath
     * @param sOutput
     * @param dTop
     * @param dLeft
     * @param nRows
     * @param nCols
     * @param dCellWidth
     * @param dNoDataVal
     * @param sXField
     * @param sYField
     * @param sDataField
     */
    static int CSVtoRaster(const char * sCSVSourcePath,
                           const char * sOutput,
                           double dTop,
                           double dLeft,
                           int nRows,
                           int nCols,
                           double dCellWidth, double dNoDataVal,
                           const char * sXField,
                           const char * sYField,
                           const char * sDataField);

    /**
     * @brief RasterToCSV
     * @param sRasterSourcePath
     * @param sOutputCSVPath
     * @return
     */
    static int RasterToCSV(const char * sRasterSourcePath,
                           const char * sOutputCSVPath);


    static int VectortoRaster(const char * sVectorSourcePath,
                              const char * sRasterOutputPath, const char *FieldName,
                              RasterMeta * p_rastermeta );

    /**
     * @brief VectortoRaster convenience method that takes a raster template
     * @param sVectorSourcePath
     * @param sRasterOutputPath
     * @param sRasterTemplate
     * @param LayerName
     * @return
     */
    static int VectortoRaster(const char * sVectorSourcePath,
                              const char * sRasterOutputPath,
                              const char * sRasterTemplate, const char *psFieldName);


    /**
     * @brief VectortoRaster Convenience method that takes a cell width
     * @param sVectorSourcePath
     * @param sRasterOutputPath
     * @param dCellWidth
     * @param LayerName
     * @return
     */
    static int VectortoRaster(const char * sVectorSourcePath,
                              const char * sRasterOutputPath,
                              double dCellWidth, const char *psFieldName);



    /**
     * @brief Hillshade
     * @return
     */
    int Hillshade(const char *psOutputHillshade);

    /**
     * @brief Slope
     * @param type
     * @return
     */
    int Slope(const char *psOutputSlope, int nSlpType);

    /**
      * @brief RastertoPng
      * @param psOutputPNG
      * @param nQuality
      * @param nLongLength
      * @param nTransparency
      * @param style
      * @return
      */
     int RastertoPng(const char *psOutputPNG, int nQuality, int nLongLength,
                     int nTransparency, Raster_SymbologyStyle style);

     /**
      * @brief Raster::RasterRootSumSquares
      * @param psRaster1
      * @param psRaster2
      * @param psOutput
      * @return
      */
     static int RasterRootSumSquares(const char * psRaster1,
                                     const char * psRaster2,
                                     const char * psOutput);

     /**
      * @brief MakeRasterConcurrent
      * @param csRasters
      * @param csRasterOutputs
      * @return
      */
     static int MakeRasterConcurrent(const char * csRasters, const char * csRasterOutputs);

     /**
      * @brief RasterMath
      * @param psRaster1
      * @param psRaster2
      * @param dOperator
      * @param iOperation
      * @param psOutput
      * @return
      */
     static int RasterMath(const char * psRaster1,
                    const char * psRaster2,
                    const double *dOperator,
                    const int iOperation,
                    const char * psOutput);

     /**
      * @brief RasterMask
      * @param psInputRaster
      * @param psMaskRaster
      * @param psOutput
      * @return
      */
     static int RasterMask(const char * psInputRaster,
                           const char * psMaskRaster,
                           const char * psOutput);

     /**
      * @brief RasterMosaic
      * @param csRasters
      * @param psOutput
      * @return
      */
     static int RasterMosaic(const char * csRasters, const char * psOutput);

     static void CSVWriteVectorValues(OGRLayer *poLayer, const char *psFieldName, const char *sRasterOutputPath);

     /**
      * @brief Raster::InvertRaster
      * @param psInputRaster
      * @param psOutputRaster
      * @param dValue
      * @return
      */
     static int InvertRaster(const char * psInputRaster,
                              const char * psOutputRaster,
                              double dValue);

     /**
      * @brief NormalizeRaster
      * @param psInputRaster
      * @param psOutputRaster
      * @param dValue
      * @return
      */
     static int NormalizeRaster(const char * psInputRaster,
                              const char * psOutputRaster);

     /**
      * @brief Raster::FilterRaster
      * @param psOperation
      * @param psRaster1
      * @param psRaster2
      * @param psWidth
      * @param psWidth
      * @return
      */
     static int FilterRaster(
             const char * psOperation,
             const char * psInputRaster,
             const char * psOutputRaster,
             int nWindowWidth,
             int nWindowHeight);

     /**
      * @brief ExtractPoints
      * @param sCSVInputSourcePath
      * @param sRasterInputSourcePath
      * @param sCSVOutputPath
      * @param sXField
      * @param sYField
      * @return
      */
     static int ExtractPoints(const char * sCSVInputSourcePath,
                       const char * sRasterInputSourcePath,
                       const char * sCSVOutputPath,
                       QString sXField,
                       QString sYField, QString sNoData);

     /**
      * @brief RasterPitRemoval
      * @param sRasterInput
      * @param sRasterOutput
      * @param sXField
      * @return
      */
     static int RasterPitRemoval(const char * sRasterInput,
                                 const char * sRasterOutput,
                                 RasterManagerFillMode eMethod);


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
    int xBlockSize; /**< TODO: Implement block size optimization for raster ops */
    int yBlockSize; /**< TODO: Implement block size optimization for raster ops */

    char * m_sFilePath;

    double m_dRasterMax;
    double m_dRasterMin;

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
    int ReSampleRaster(GDALRasterBand * pRBInput, GDALRasterBand * pRBOutput,
                       double fNewCellSize, double fNewLeft, double fNewTop,
                       int nNewRows, int nNewCols);

    /**
     * @brief getColorTable
     * @param colorTable
     * @param style
     * @param nTransparency
     * @return
     */
    static int GetColorTable(GDALColorTable &colorTable, Raster_SymbologyStyle style,
                             int nTransparency);

    /**
     * @brief resizeAndCompressImage
     * @param inputImage
     * @param nLongLength
     * @param nQuality
     * @return
     */
    static int ResizeAndCompressImage(const char* inputImage, int nLongLength, int nQuality);

};


}

#endif // RASTER_H
