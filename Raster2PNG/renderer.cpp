#include "renderer.h"


Renderer::Renderer(const char *inputRasterPath,
                   ColorRamp ramp,
                   int nTransparency,
                   bool zeroNoData)
{
    pngOutPath = NULL;

    colorTable = new GDALColorTable(GPI_RGB);

    pDriverTiff = GetGDALDriverManager()->GetDriverByName("GTiff");
    pDriverPNG = GetGDALDriverManager()->GetDriverByName("PNG");

    setRendererColorTable(ramp, nTransparency);
    setupRaster(inputRasterPath);
    setZeroNoData(zeroNoData);
}

Renderer::~Renderer()
{
    if (pRaster != NULL)
    {
        GDALClose(pRaster);
        pRaster = NULL;
    }
    GDALDestroyColorTable(colorTable);
}

void Renderer::printLegend()
{
    setLegendPath();
    createLegend();
}

void Renderer::printLegend(const char *path)
{
    setLegendPath(path);
    createLegend();
}

int Renderer::rasterToPNG(const char *pngPath, int nQuality, int nLength)
{
    pngOutPath = pngPath;

    setup();

    createByteRaster();

    pTempRaster->GetRasterBand(1)->SetColorTable(colorTable);

    pPngDS = pDriverPNG->CreateCopy(pngPath,pTempRaster,FALSE,NULL,NULL,NULL);
    pPngDS->SetGeoTransform(transform);

    cleanUp();

    //resize and compress
    resizeAndCompressPNG(pngPath, nLength, nQuality);

    return 0;
}

void Renderer::setPrecision(int prec)
{
    precision = prec;
}

int Renderer::setRendererColorTable(ColorRamp rampStyle, int nTransparency)
{
    GDALColorEntry trans;
    trans.c1 = 255, trans.c2 = 255, trans.c3 = 255, trans.c4 = 0;
    switch (rampStyle)
    {
    case CR_BlackWhite:
    {
        GDALColorEntry blk, wht;
        blk.c1 = 0, blk.c2 = 0, blk.c3 = 0, blk.c4 = nTransparency;
        wht.c1 = 255, wht.c2 = 255, wht.c3 = 255, wht.c4 = nTransparency;

        colorTable->CreateColorRamp(1, &blk, 255, &wht);
        colorTable->SetColorEntry(0, &trans);
        break;
    }
    case CR_DEM:
    {
        GDALColorEntry tan, brn, grn, wht;
        tan.c1 = 255, tan.c2 = 235, tan.c3 = 176, tan.c4 = nTransparency;
        brn.c1 = 115, brn.c2 = 77, brn.c3 = 0, brn.c4 = nTransparency;
        grn.c1 = 38, grn.c2 = 115, grn.c3 = 0, grn.c4 = nTransparency;
        wht.c1 = 255, wht.c2 = 255, wht.c3 = 255, wht.c4 = nTransparency;

        colorTable->CreateColorRamp(1, &tan, 85, &grn);
        colorTable->CreateColorRamp(85, &grn, 170, &brn);
        colorTable->CreateColorRamp(170, &brn, 255, &wht);
        colorTable->SetColorEntry(0, &trans);
        break;
    }
    case CR_DoD:
    {
        GDALColorEntry red, wht, blu;
        red.c1 = 230, red.c2 = 0, red.c3 = 0, red.c4 = nTransparency;
        wht.c1 = 255, wht.c2 = 255, wht.c3 = 255, wht.c4 = nTransparency;
        blu.c1 = 0, blu.c2 = 76, blu.c3 = 168, blu.c4 = nTransparency;

        colorTable->CreateColorRamp(1, &red, 128, &wht);
        colorTable->CreateColorRamp(128, &wht, 255, &blu);
        //set transparent entry
        colorTable->SetColorEntry(0, &trans);
        break;
    }
    case CR_GrainSize:
    {
        break;
    }
    case CR_GreenBlue:
    {
        GDALColorEntry one, two, three, four, five;
        one.c1 = 2, one.c2 = 32, one.c3 = 227, one.c4 = nTransparency;
        two.c1 = 50, two.c2 = 154, two.c3 = 240, two.c4 = nTransparency;
        three.c1 = 0, three.c2 = 242, three.c3 = 242, three.c4 = nTransparency;
        four.c1 = 50, four.c2 = 219, four.c3 = 110, four.c4 = nTransparency;
        five.c1 = 32, five.c2 = 204, five.c3 = 16, five.c4 = nTransparency;

        colorTable->CreateColorRamp(1, &one, 93, &two);
        colorTable->CreateColorRamp(93, &two, 139, &three);
        colorTable->CreateColorRamp(139, &three, 208, &four);
        colorTable->CreateColorRamp(208, &four, 255, &five);
        break;
    }
    case CR_LtBlueDkBlue:
    {
        GDALColorEntry ltb, dkb;
        ltb.c1 = 182, ltb.c2 = 237, ltb.c3 = 240, ltb.c4 = nTransparency;
        dkb.c1 = 9, dkb.c2 = 9, dkb.c3 = 145, dkb.c4 = nTransparency;

        colorTable->CreateColorRamp(1, &ltb, 255, &dkb);
        colorTable->SetColorEntry(0, &trans);
        break;
        break;
    }
    case CR_PartialSpectrum:
    {
         QVector<int> red(11), grn(11), blu(11);

        red[0] = 242, red[1] = 250, red[2] = 255, red[3] = 255, red[4] = 255, red[5] = 255, red[6] = 245, red[7] = 224, red[8] = 145, red[9] = 88, red[10] = 12;
        grn[0] = 241, grn[1] = 245, grn[2] = 252, grn[3] = 213, grn[4] = 128, grn[5] = 0, grn[6] = 5, grn[7] = 7, grn[8] = 18, grn[9] = 28, grn[10] = 28;
        blu[0] = 162, blu[1] = 112, blu[2] = 56, blu[3] = 0, blu[4] = 0, blu[5] = 0, blu[6] = 165, blu[7] = 240, blu[8] = 224, blu[9] = 199, blu[10] = 173;

        GDALColorEntry one, two, three, four, five, six, seven;
        one.c1 = red[0], one.c2 = grn[0], one.c3 = blu[0], one.c4 = nTransparency;
        two.c1 = red[2], two.c2 = grn[2], two.c3 = blu[2], two.c4 = nTransparency;
        three.c1 = red[3], three.c2 = grn[3], three.c3 = blu[3], three.c4 = nTransparency;
        four.c1 = red[5], four.c2 = grn[5], four.c3 = blu[5], four.c4 = nTransparency;
        five.c1 = red[7], five.c2 = grn[7], five.c3 = blu[7], five.c4 = nTransparency;
        six.c1 = red[9], six.c2 = grn[9], six.c3 = blu[9], six.c4 = nTransparency;
        seven.c1 = red[10], seven.c2 = grn[10], seven.c3 = blu[10], seven.c4 = nTransparency;

        colorTable->CreateColorRamp(1, &one, 70, &two);
        colorTable->CreateColorRamp(70, &two, 93, &three);
        colorTable->CreateColorRamp(93, &three, 129, &four);
        colorTable->CreateColorRamp(129, &four, 185, &five);
        colorTable->CreateColorRamp(185, &five, 232, &six);
        colorTable->CreateColorRamp(232, &six, 255, &seven);

        colorTable->SetColorEntry(0, &trans);

        break;
    }
    case CR_Precipitation:
    {
        GDALColorEntry red, yel, gn1, gn2, gb1, gb2, gb3;
        red.c1 = 194, red.c2 = 82, red.c3 = 60, red.c4 = nTransparency;
        yel.c1 = 252, yel.c2 = 240, yel.c3 = 3, yel.c4 = nTransparency;
        gn1.c1 = 123, gn1.c2 = 237, gn1.c3 = 0, gn1.c4 = nTransparency;
        gn2.c1 = 6, gn2.c2 = 212, gn2.c3 = 27, gn2.c4 = nTransparency;
        gb1.c1 = 27, gb1.c2 = 168, gb1.c3 = 124, gb1.c4 = nTransparency;
        gb2.c1 = 24, gb2.c2 = 117, gb2.c3 = 140, gb2.c4 = nTransparency;
        gb3.c1 = 11, gb3.c2 = 44, gb3.c3 = 122, gb3.c4 = nTransparency;

        colorTable->CreateColorRamp(1, &red, 113, &yel);
        colorTable->CreateColorRamp(113, &yel, 142, &gn1);
        colorTable->CreateColorRamp(142, &gn1, 170, &gn2);
        colorTable->CreateColorRamp(170, &gn2, 198, &gb1);
        colorTable->CreateColorRamp(198, &gb1, 227, &gb2);
        colorTable->CreateColorRamp(227, &gb2, 255, &gb3);
        colorTable->SetColorEntry(0, &trans);
        break;
    }
    case CR_Slope:
    {
        GDALColorEntry red, grn, grn2, org, yel;
        grn.c1 = 56, grn.c2 = 168, grn.c3 = 0, grn.c4 = nTransparency;
        grn2.c1 = 139, grn2.c2 = 209, grn2.c3 = 0, grn2.c4 = nTransparency;
        org.c1 = 255, org.c2 = 128, org.c3 = 0, org.c4 = nTransparency;
        red.c1 = 255, red.c2 = 34, red.c3 = 0, red.c4 = nTransparency;
        yel.c1 = 255, yel.c2 = 255, yel.c3 = 0, yel.c4 = nTransparency;

        colorTable->CreateColorRamp(1, &grn, 64, &grn2);
        colorTable->CreateColorRamp(64, &grn2, 128, &yel);
        colorTable->CreateColorRamp(128, &yel, 191, &org);
        colorTable->CreateColorRamp(191, &org, 255, &red);
        colorTable->SetColorEntry(0, &trans);
        break;
    }
    case CR_SlopeGCD:
    {
        QVector<int> red(10), grn(10), blu(10);
        GDALColorEntry entry1, entry2;
        int index1, index2;

        red[0] = 255, red[1] = 255, red[2] = 255, red[3] = 255, red[4] = 255, red[5] = 255, red[6] = 255, red[7] = 255, red[8] = 161, red[9] = 130;
        grn[0] = 235, grn[1] = 219, grn[2] = 202, grn[3] = 186, grn[4] = 170, grn[5] = 128, grn[6] = 85, grn[7] = 42, grn[8] = 120, grn[9] = 130;
        blu[0] = 176, blu[1] = 135, blu[2] = 97, blu[3] = 59, blu[4] = 0, blu[5] = 0, blu[6] = 0, blu[7] = 0, blu[8] = 120, blu[9] = 130;

        for (int i=0; i<red.size(); i++)
        {
            entry1.c1 = red[i], entry1.c2 = grn[i], entry1.c3 = blu[i], entry1.c4 = nTransparency;
            entry2.c1 = red[i+1], entry2.c2 = grn[i+1], entry2.c3 = blu[i+1], entry2.c4 = nTransparency;
            index1 = round((i*1.0 / red.size()) * 254) + 1;
            index2 = round(((i+1)*1.0 / red.size()) * 254) + 1;
            if (i+1 == red.size())
            {
                index2 = 255;
            }
            colorTable->CreateColorRamp(index1, &entry1, index2, &entry2);
        }

        colorTable->SetColorEntry(0, &trans);
        break;
    }
    case CR_WhiteRed:
    {
        GDALColorEntry red, wht;
        red.c1 = 230, red.c2 = 0, red.c3 = 0, red.c4 = nTransparency;
        wht.c1 = 255, wht.c2 = 255, wht.c3 = 255, wht.c4 = nTransparency;

        colorTable->CreateColorRamp(1, &wht, 255, &red);
        colorTable->SetColorEntry(0, &trans);
        break;
        break;
    }
    }

    return 0;
}

void Renderer::setZeroNoData(bool bValue)
{
    zeroNoData = bValue;

    if (zeroNoData)
    {
        noData2 = 0.0;
    }
    else
    {
        noData2 = noData;
    }
}

void Renderer::stackImages(const char *inputList, const char *outputImage, int nQuality)
{
    QString delimInput = QString::fromUtf8(inputList);
    //split input list of paths
    QStringList pngPaths = delimInput.split(";", QString::SkipEmptyParts);

    //base image is the first path in the list
    QImage base = QImage(pngPaths[0]);

    //create the stacked image
    QImage layered = QImage(base.size(), QImage::Format_ARGB32);
    QPainter painter(&layered);

    //assign the stacked image to the painter
    painter.setCompositionMode(QPainter::CompositionMode_Source);
    painter.fillRect(layered.rect(), Qt::transparent);

    //add the base image
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    painter.drawImage(0,0,base);

    for (int i=1; i<pngPaths.size(); i++)
    {
        //add another image to the stacked image
        QImage overlay = QImage(pngPaths[i]);
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.drawImage(0,0,overlay);
    }

    //save the stacked image
    layered.save(QString::fromUtf8(outputImage), 0, nQuality);
}

void Renderer::cleanUp()
{
    GDALClose(pTempRaster);
    GDALClose(pPngDS);
    pDriverTiff->Delete(tempRasterPath.toStdString().c_str());

    CPLFree(oldRow);
    CPLFree(newRow);
    oldRow = NULL;
    newRow = NULL;
}



int Renderer::resizeAndCompressPNG(const char *inputImage, int nLength, int nQuality)
{
    QImage image = QImage(QString::fromUtf8(inputImage));

    //determine if height or width is greater and rescale
    if (image.height() > image.width())
    {
        image = image.scaledToHeight(nLength, Qt::SmoothTransformation);
    }
    else
    {
        image = image.scaledToWidth(nLength, Qt::SmoothTransformation);
    }

    //save and compress the image
    image.save(QString::fromUtf8(inputImage), 0, nQuality);

    return 0;
}

void Renderer::setLegendPath()
{
    QString legendName, legendDir;
    QFile pngFile(QString::fromUtf8(pngOutPath));
    QFileInfo pngInfo(pngFile);
    legendDir = pngInfo.absolutePath();
    legendName = pngInfo.baseName() + "_legend.png";
    legendPath = legendDir + "/" + legendName;
}

void Renderer::setLegendPath(const char *path)
{
    legendPath = QString::fromUtf8(path);
}

void Renderer::setPrecision()
{
    double value;

    value = fabs(adjMax);

    if (value < 0.000001)
    {
        precision = 8;
    }
    else if (value < 0.00001)
    {
        precision = 7;
    }
    else if (value < 0.0001)
    {
        precision = 6;
    }
    else if (value < 0.001)
    {
        precision = 5;
    }
    else if (value < 0.01)
    {
        precision = 4;
    }
    else if (value < 0.1)
    {
        precision = 2;
    }
    else if (value < 1.0)
    {
        precision = 1;
    }
    else if (value < 10.0)
    {
        precision = 1;
    }
    else if (value < 100.0)
    {
        precision = 1;
    }
    else
    {
        precision = 0;
    }
}

int Renderer::setTempRasterPath(const char *rasterPath)
{
    QDateTime dtCurrent = QDateTime::currentDateTime();
    QString name, dirName;
    QFileInfo fileInfo(QString::fromUtf8(rasterPath));
    dirName = fileInfo.absolutePath();
    name = fileInfo.baseName();
    name = name + "_" + dtCurrent.toString("yyyyMMddhhmmss") + ".tif";
    tempRasterPath = dirName + "/" + name;

    return 0;
}

void Renderer::setup()
{
    pTempRaster = pDriverTiff->Create(tempRasterPath.toStdString().c_str(), nCols, nRows, 1, GDT_Byte, NULL);
    pTempRaster->GetRasterBand(1)->SetNoDataValue(0);
    pTempRaster->GetRasterBand(1)->SetColorInterpretation(GCI_PaletteIndex);

    oldRow = (float*) CPLMalloc(sizeof(float)*nCols);
    newRow = (unsigned char*) CPLMalloc(sizeof(int)*nCols);
}

int Renderer::setupRaster(const char *inputRasterPath)
{
    rasterPath = inputRasterPath;
    pRaster = (GDALDataset*) GDALOpen(rasterPath, GA_ReadOnly);

    pRaster->GetRasterBand(1)->GetStatistics(FALSE, TRUE, &min, &max, &mean, &stdev);
    adjMin = min, adjMax = max, adjMean = mean;
    pRaster->GetGeoTransform(transform);
    rasterType = pRaster->GetRasterBand(1)->GetRasterDataType();
    nRows = pRaster->GetRasterBand(1)->GetYSize();
    nCols = pRaster->GetRasterBand(1)->GetXSize();
    noData = pRaster->GetRasterBand(1)->GetNoDataValue();

    setTempRasterPath(rasterPath);

    return 0;
}
