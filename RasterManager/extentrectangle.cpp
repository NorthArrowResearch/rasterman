#define MY_DLL_EXPORT

#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>

#include "extentrectangle.h"
#include "rastermanager_exception.h"
#include "rastermanager.h"
#include "gdal.h"
#include "gdal_priv.h"

namespace RasterManager {

ExtentRectangle::ExtentRectangle(){
    Init(0, 0, 0, 0, 0.1, 0.1);
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
    Init(source.GetTop(), source.GetLeft(),
         source.GetRows(),  source.GetCols(),
         source.GetCellHeight(), source.GetCellWidth());
}

void ExtentRectangle::operator=(ExtentRectangle &source)
{
    Init(source.GetTop(),   source.GetLeft(),
         source.GetRows(),  source.GetCols(),
         source.GetCellHeight(), source.GetCellWidth());
}

ExtentRectangle::ExtentRectangle(QString psFilePath)
{
    const QByteArray qbFilePath = psFilePath.toLocal8Bit();
    Load(qbFilePath.data());
}

ExtentRectangle::ExtentRectangle(const char * psFilePath)
{
    Load(psFilePath);
}

void ExtentRectangle::Init(double fTop,
                           double fLeft,
                           int nRows,
                           int nCols,
                           double dCellHeight,
                           double dCellWidth){

    cols = nCols;
    rows = nRows;

    // Cell height is usually negative but we need it to be positive
    SetTransform(fTop, fLeft, dCellWidth, dCellHeight);
}

void ExtentRectangle::Load(const char * psFilePath){
    // Basic file existence Check.
    CheckFile(psFilePath, true);

    // Check Valid Data Set
    GDALDataset * pDS = (GDALDataset*) GDALOpen(psFilePath, GA_ReadOnly);
    if (pDS == NULL)
        throw RasterManagerException(INPUT_FILE_NOT_VALID, CPLGetLastErrorMsg());

    GDALRasterBand * pBand = pDS->GetRasterBand(1);

    cols = pBand->GetXSize();
    rows = pBand->GetYSize();

    pDS->GetGeoTransform(m_GeoTransform);

    GDALClose(pDS);

}

void ExtentRectangle::Union(ExtentRectangle * aRectangle){

    // For readability let's set all the values first
    double right = std::max(GetRight(), aRectangle->GetRight());
    double bottom = std::min(GetBottom(), aRectangle->GetBottom());

    double left = std::min(GetLeft(), aRectangle->GetLeft());
    double top = std::max(GetTop(), aRectangle->GetTop());

    cols = (int)((right - left) / fabs(GetCellWidth()));
    rows = (int)((top - bottom) / fabs(GetCellHeight()));

    SetTransform(top, left, GetCellWidth(), GetCellHeight());
}

void ExtentRectangle::Intersect(ExtentRectangle * aRectangle){

    // For readability let's set all the values first
    double right = std::min(GetRight(), aRectangle->GetRight());
    double bottom = std::max(GetBottom(), aRectangle->GetBottom());

    double left = std::max(GetLeft(), aRectangle->GetLeft());
    double top = std::min(GetTop(), aRectangle->GetTop());

    cols = (int)((right - left) / fabs(GetCellWidth()));
    rows = (int)((top - bottom) / fabs(GetCellHeight()));

    SetTransform(top, left, GetCellWidth(), GetCellHeight());
}

void ExtentRectangle::SetTransform(double dTop, double dLeft, double dCellWidth, double dCellHeight){
    m_GeoTransform[0] = dLeft;
    m_GeoTransform[1] = dCellWidth;
    m_GeoTransform[2] = 0;
    m_GeoTransform[3] = dTop;
    m_GeoTransform[4] = 0;
    m_GeoTransform[5] = dCellHeight;
}

int ExtentRectangle::GetRowTranslation(ExtentRectangle * aRectangle){
    return (int) ((aRectangle->GetTop() - GetTop()) / fabs(GetCellHeight()) ) ;
}

int ExtentRectangle::GetColTranslation(ExtentRectangle * aRectangle){
    return (int) ((aRectangle->GetLeft() - GetLeft()) / fabs(GetCellWidth()) );
}

double ExtentRectangle::GetRight(){
    return GetLeft() + ( (double)cols * fabs(GetCellWidth()) );
}
double ExtentRectangle::GetBottom(){
    return GetTop() - ((double)rows * fabs(GetCellHeight()) );
}
double ExtentRectangle::GetWidth(){
    return cols * fabs(GetCellWidth());
}
double ExtentRectangle::GetHeight(){
    return rows * fabs(GetCellHeight());
}
double ExtentRectangle::GetTop(){
    return m_GeoTransform[3];
}
double ExtentRectangle::GetLeft(){
    return m_GeoTransform[0];
}
double ExtentRectangle::GetCellWidth(){
    return m_GeoTransform[1];
}
double ExtentRectangle::GetCellHeight(){
    return m_GeoTransform[5];
}
int ExtentRectangle::GetRows(){
    return rows;
}
int ExtentRectangle::GetCols(){
    return cols;
}
double * ExtentRectangle::GetGeoTransform(){

    // The affine transform consists of six coefficients returned by GDALDataset::GetGeoTransform()
    // which map pixel/line coordinates into georeferenced space using the following relationship:

    //    Xgeo = GT(0) + Xpixel*GT(1) + Yline*GT(2)
    //    Ygeo = GT(3) + Xpixel*GT(4) + Yline*GT(5)

    // In case of north up images, the GT(2) and GT(4) coefficients are zero, and the GT(1) is
    // pixel width, and GT(5) is pixel height. The (GT(0),GT(3)) position is the top left corner
    // of the top left pixel of the raster.

    return m_GeoTransform;
}


} // Namespace

