#define MY_DLL_EXPORT

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

#include "extentrectangle.h"
#include "rmexception.h"

#include "gdal.h"
#include "gdal_priv.h"

namespace RasterManager {

ExtentRectangle::ExtentRectangle(){

}

ExtentRectangle::ExtentRectangle(double fTop,
                                 double fLeft,
                                 int nRows,
                                 int nCols,
                                 double dCellHeight,
                                 double dCellWidth)
{
    Init(fTop, fLeft, nRows, nCols, dCellHeight, dCellWidth);
}

ExtentRectangle::ExtentRectangle(ExtentRectangle &source)
{
    Init(source.top, source.left,
          source.rows,  source.cols,
         source.cellHeight, source.cellWidth);
}

void ExtentRectangle::operator=(ExtentRectangle &source)
{
    Init(source.top, source.left,
          source.rows,  source.cols,
         source.cellHeight, source.cellWidth);
}

ExtentRectangle::ExtentRectangle(const char * psFilePath)
{
    GDALAllRegister();

    GDALDataset * pDS = (GDALDataset*) GDALOpen(psFilePath, GA_ReadOnly);
    if (pDS == NULL)
        throw RMException(CPLGetLastErrorMsg());

    GDALRasterBand * pBand = pDS->GetRasterBand(1);
    cols = pBand->GetXSize();
    rows = pBand->GetYSize();

    double transform[6];
    pDS->GetGeoTransform(transform);
    cellHeight = transform[5];
    cellWidth = transform[1];

    left = transform[0];
    top = transform[3];

    GDALClose(pDS);
}

void ExtentRectangle::Init(double fTop,
          double fLeft,
          int nRows,
          int nCols,
          double dCellHeight,
          double dCellWidth){

    top = fTop;
    left = fLeft;

    cols = nCols;
    rows = nRows;

    // Cell height is usually negative but we need it to be positive
    cellHeight = abs(dCellHeight);
    cellWidth = abs(dCellWidth);
}

void ExtentRectangle::Union(ExtentRectangle * aRectangle){

    double right = std::max(this->GetRight(), aRectangle->GetRight());
    double bottom = std::min(this->GetBottom(), aRectangle->GetBottom());

    cols = (int)((right - left) / cellWidth);
    rows = (int)((top - bottom) / cellHeight);

    top = std::max(top, aRectangle->GetTop());
    left = std::min(left, aRectangle->GetLeft());

    cols = (int)((right - left) / cellWidth);
    rows = (int)((top - bottom) / cellHeight);
}

int ExtentRectangle::GetRowTranslation(ExtentRectangle * aRectangle){
    return (int) ((aRectangle->GetTop() - top) / cellHeight ) ;
}

int ExtentRectangle::GetColTranslation(ExtentRectangle * aRectangle){
    return (int) ((aRectangle->GetLeft() - left) / cellWidth );
}
double ExtentRectangle::GetTop(){
    return top;
}
double ExtentRectangle::GetLeft(){
    return left;
}
double ExtentRectangle::GetRight(){
    return left + ( (double)rows * cellWidth);
}
double ExtentRectangle::GetBottom(){
    return top - ((double)cols * cellHeight);
}
double ExtentRectangle::GetWidth(){
    return cols * cellWidth;
}
double ExtentRectangle::GetHeight(){
    return rows * cellHeight;
}

int ExtentRectangle::GetRows(){
    return rows;
}
int ExtentRectangle::GetCols(){
    return cols;
}

} // Namespace

