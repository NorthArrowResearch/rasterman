#define MY_DLL_EXPORT
/*
 * Create a PNG image from a GDAL compatible raster
 *
 * 4 December 2014
 *
 * Original code developed by Konrad Hafen at Utah State University
*/

#include "raster.h"
#include "rastermanager_interface.h"
#include "gdal.h"
#include "gdal_priv.h"
#include "helpers.h"
#include <QtCore>
#include <QWidget>
#include <QString>


namespace RasterManager {

int Raster::RastertoPng(const char * outputPNG,
                        int nQuality,
                        int nLongLength,
                        int nTransparency,
                        Raster_SymbologyStyle style)
{
    // Input validation
    // ------------------------
    if (nQuality < 0 || nQuality > 100)
        return RM_PNG_QUALITY;

    if (nTransparency < 0 || nTransparency > 100)
        return RM_PNG_TRANSPARENCY;

    if (nLongLength < -1)
        return RM_PNG_LONG_AXIS;

    CheckFile(outputPNG, false);

    GDALDataset *pOldDS, *pTempDS, *pPngDS;
    GDALDriver *pDriverTiff, *pDriverPNG;
    GDALColorTable colorTable(GPI_RGB);

    pDriverTiff = GetGDALDriverManager()->GetDriverByName("GTiff");
    pDriverPNG = GetGDALDriverManager()->GetDriverByName("PNG");

    //create temp raster file path by appending "_yyyyMMddhhmmss" to the input raster file name
    QDateTime dtCurrent = QDateTime::currentDateTime();
    QFileInfo fileInfo(QString::fromUtf8(m_sFilePath));

    QString sName(fileInfo.baseName() + "_" + dtCurrent.toString("yyyyMMddhhmmss") + ".tif");

    const QByteArray name = sName.toLocal8Bit();
    const QByteArray dirName(fileInfo.absolutePath().toLocal8Bit());
    const QByteArray tempRaster(dirName + "/" + name);

    //get source data from input raster
    pOldDS = (GDALDataset*) GDALOpen(m_sFilePath, GA_ReadOnly);
    double transform[6];
    int nCols = GetCols();
    int nRows = GetRows();
    double noData = GetNoDataValue();
    pOldDS->GetGeoTransform(transform);

    //create temporary GeoTiff dataset of type byte, this is the dataset to which we will apply the symbology
    pTempDS = pDriverTiff->Create(tempRaster.data(), nCols, nRows, 1, GDT_Byte, NULL);

    GetColorTable(colorTable, style, nTransparency);

    //allocate memory for reading and writing datasets
    double * oldRow = (double*) CPLMalloc(sizeof(double)*GetCols());
    unsigned char * newRow = (unsigned char*) CPLMalloc(sizeof(int)*GetCols());

    if (style == GSS_DEM || style == GSS_Unknown)
    {
        int byte;
        double range, scaled;
        double corVal = 0.0;

        double min = GetMinimum();
        double max = GetMaximum();
        if (min < 0.0)
        {
            corVal = fabs(min);
        }

        range = (max + corVal) - (min + corVal);

        for (int i=0; i< GetRows(); i++)
        {
            pOldDS->GetRasterBand(1)->RasterIO(GF_Read, 0, i, nCols, 1, oldRow, nCols, 1, GDT_Float64, 0, 0);
            for (int j=0; j< GetCols(); j++)
            {
                if (oldRow[j] == noData)
                {
                    newRow[j] = 0;
                }
                else
                {
                    scaled = fabs(1 - ((max+corVal) - (oldRow[j]+corVal)) / range);
                    byte = (int) roundf(scaled * 254) + 1;
                    newRow[j] = byte;
                }
            }
            pTempDS->GetRasterBand(1)->RasterIO(GF_Write, 0, i, nCols, 1, newRow, nCols, 1, GDT_Byte, 0, 0);
        }
    }
    else if (style == GSS_DoD)
    {

    }
    else if (style == GSS_Error)
    {

    }
    else if (style == GSS_Hlsd)
    {
        for (int i=0; i<GetRows(); i++)
        {
           pOldDS->GetRasterBand(1)->RasterIO(GF_Read, 0, i, nCols, 1, newRow, nCols, 1, GDT_Byte, 0, 0);
           pTempDS->GetRasterBand(1)->RasterIO(GF_Write, 0, i, nCols, 1, newRow, nCols, 1, GDT_Byte, 0, 0);
        }
    }
    else if (style == GSS_PtDens)
    {

    }
    else if (style == GSS_SlopeDeg)
    {

    }
    else if (style == GSS_SlopePer)
    {

    }
    else if (style == GSS_Unknown)
    {

    }

    //add color table to temporary tiff
    CalculateStats(pTempDS->GetRasterBand(1));
    pTempDS->GetRasterBand(1)->SetColorInterpretation(GCI_PaletteIndex);
    pTempDS->GetRasterBand(1)->SetColorTable(&colorTable);

    //copy temporary tiff to output PNG
    pPngDS = pDriverPNG->CreateCopy(outputPNG, pTempDS, FALSE, NULL, NULL, NULL);
    pPngDS->SetGeoTransform(transform);

    //close all datasets
    GDALClose(pOldDS);
    GDALClose(pTempDS);
    GDALClose(pPngDS);

    //delete temporary tiff
    pDriverTiff->Delete(tempRaster.data());

    //free allocated memory
    CPLFree(oldRow);
    CPLFree(newRow);
    oldRow = NULL;
    newRow = NULL;

    //destroy drivers and color table
    //GDALDestroyColorTable(&colorTable);

    //resize and compress the output PNG
   return ResizeAndCompressImage(outputPNG, nLongLength, nQuality);

}

int Raster::GetColorTable(GDALColorTable &colorTable, Raster_SymbologyStyle style, int nTransparency)
{
    GDALColorEntry trans;
    trans.c1 = 255, trans.c2 = 255, trans.c3 = 255, trans.c4 = 0;

    if (style == GSS_DEM)
    {
        GDALColorEntry tan, brn, grn, wht;
        tan.c1 = 255, tan.c2 = 235, tan.c3 = 176, tan.c4 = nTransparency;
        brn.c1 = 115, brn.c2 = 77, brn.c3 = 0, brn.c4 = nTransparency;
        grn.c1 = 38, grn.c2 = 115, grn.c3 = 0, grn.c4 = nTransparency;
        wht.c1 = 255, wht.c2 = 255, wht.c3 = 255, wht.c4 = nTransparency;

        colorTable.CreateColorRamp(1, &tan, 85, &grn);
        colorTable.CreateColorRamp(85, &grn, 170, &brn);
        colorTable.CreateColorRamp(170, &brn, 255, &wht);
        colorTable.SetColorEntry(0, &trans);
    }
    else if (style == GSS_DoD)
    {
        QVector<int> red(20), grn(20), blu(20);
        GDALColorEntry entry;

        //reds
        red[0] = 230, red[1] = 235, red[2] = 240, red[3] = 242, red[4] = 245, red[5] = 245, red[6] = 245, red[7] = 242, red[8] = 237, red[9] = 230;
        grn[0] = 0, grn[1] = 45, grn[2] = 67, grn[3] = 88, grn[4] = 108, grn[5] = 131, grn[6] = 151, grn[7] = 171, grn[8] = 190, grn[9] = 208;
        blu[0] = 0, blu[1] = 23, blu[2] = 41, blu[3] = 61, blu[4] = 81, blu[5] = 105, blu[6] = 130, blu[7] = 155, blu[8] = 180, blu[9] = 207;
        //blues
        red[10] = 218, red[11] = 197, red[12] = 176, red[13] = 155, red[14] = 135, red[15] = 110, red[16] = 92, red[17] = 72, red[18] = 49, red[19] = 2;
        grn[10] = 218, grn[11] = 201, grn[12] = 183, grn[13] = 166, grn[14] = 150, grn[15] = 131, grn[16] = 118, grn[17] = 105, grn[18] = 91, grn[19] = 77;
        blu[10] = 224, blu[11] = 219, blu[12] = 214, blu[13] = 207, blu[14] = 201, blu[15] = 194, blu[16] = 189, blu[17] = 185, blu[18] = 176, blu[19] = 168;

        //loop through vectors to assign colors to table
        for (int i=0; i<20; i++)
        {
            entry.c1 = red[i], entry.c2 = grn[i], entry.c3 = blu[i], entry.c4 = nTransparency;
            colorTable.SetColorEntry(i+1, &entry);
        }
        //set transparent entry
        colorTable.SetColorEntry(0, &trans);
    }
    else if (style == GSS_Error)
    {

    }
    else if (style == GSS_PtDens)
    {

    }
    else if (style == GSS_SlopeDeg)
    {

    }
    else if (style == GSS_SlopePer)
    {

    }
    else if (style == GSS_Unknown || style == GSS_Hlsd)
    {
        GDALColorEntry blk, wht;
        blk.c1 = 0, blk.c2 = 0, blk.c3 = 0, blk.c4 = nTransparency;
        wht.c1 = 255, wht.c2 = 255, wht.c3 = 255, wht.c4 = nTransparency;

        colorTable.CreateColorRamp(1, &blk, 255, &wht);
        colorTable.SetColorEntry(0, &trans);
    }

    return 0;
}

int Raster::ResizeAndCompressImage(const char* inputImage, int nLongLength, int nQuality)
{
    QString sInputImagePath(inputImage);
    QImage input = QImage(sInputImagePath);

    // -1 disables scaling. So should zero.
    if (nLongLength > 0)
    {
        //determine if height or width is greater and rescale
        if (input.height() > input.width())
        {
            input = input.scaledToHeight(nLongLength, Qt::SmoothTransformation);
        }
        else
        {
            input = input.scaledToWidth(nLongLength, Qt::SmoothTransformation);
        }
    }
    else {
        return RM_PNG_LONG_AXIS;
    }
    //save and compress the image
    input.save(sInputImagePath, 0, nQuality);

    return PROCESS_OK;
}

}
