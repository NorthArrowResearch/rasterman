#include "rasterarray.h"
#include <QDebug>

namespace RasterManager {

RasterArray::RasterArray(const char * psFilePath) : Raster(psFilePath)
{
    //Resize vectors
    Terrain.resize(GetTotalCells()); //Values input from file, modified throughout program
    Neighbors.resize(8);


}

void RasterArray::WriteArraytoRaster(QString sOutputPath, std::vector<double> *vPointArray ){

    // Create the output dataset for writing
    const QByteArray csOutput = sOutputPath.toLocal8Bit();

    // Set the bounds and nodata to be the same as the input
    GDALDataset * pDSOutput = CreateOutputDS(csOutput.data(), this);
    double * pOutputLine = (double *) CPLMalloc(sizeof(double)*GetCols());

    // Write rows and columns
    for (int i = 0; i < GetRows(); i++)
    {
        for (int j = 0; j < GetCols(); j++){
            pOutputLine[j] = vPointArray->at(i*GetCols() + j);
        }
        pDSOutput->GetRasterBand(1)->RasterIO(GF_Write, 0,  i, GetCols(), 1, pOutputLine, GetCols(), 1, GDT_Float64, 0, 0);
    }
    CPLFree(pOutputLine);
    GDALClose(pDSOutput);

}
void RasterArray::WriteArraytoRaster(QString sOutputPath, std::vector<int> *vPointArray){
    std::vector<double> vDouble(vPointArray->begin(), vPointArray->end());
    WriteArraytoRaster(sOutputPath, &vDouble);
}

size_t RasterArray::GetNeighborID(int id, eDirection dir){

    if (IsDirectionValid(id, dir)){
        //Returns the ID value for the eight neighbors, with -1 for cells off the grid
        switch (dir) {
        case DIR_NW: return id - 1 - GetCols(); break;
        case DIR_N:  return id - GetCols(); break;
        case DIR_NE: return id + 1 - GetCols(); break;
        case DIR_E:  return id + 1; break;
        case DIR_SE: return id + 1 + GetCols(); break;
        case DIR_S:  return id + GetCols(); break;
        case DIR_SW: return id - 1 + GetCols(); break;
        case DIR_W:  return id - 1; break;
        default: return -1; break;
        }
    }
    else{
        // We're out of bounds
        return -1;
    }
}

void RasterArray::GetNeighbors(int ID)
{
    for (int d = DIR_NW; d <= DIR_W; d++)
    {
        Neighbors.at(d) = GetNeighborID( ID, (eDirection) d );
    }
}

int RasterArray::getCol(int i){
    return (int) floor(i % GetCols());
}
int RasterArray::getRow(int i){
    return (int) floor(i / GetCols());
}

bool RasterArray::HasValidNeighbor(int ID){
    // Opposite of Neighbournovalue. Returns true if there are any valid neighbors.
    GetNeighbors(ID);
    for (int d = DIR_NW; d <= DIR_W; d++)
    {
        if (Neighbors.at(d) != ENTRYPOINT && Terrain.at(Neighbors.at(d)) != dNoDataValue)
            return true;
    }
    return false;
}

bool RasterArray::IsBorder(int ID)
{
    //Tests if cell is on the outer edge of the grid, and thus is an outlet
    if ( IsTopEdge(ID) || IsBottomEdge(ID) || IsLeftEdge(ID) || IsRightEdge(ID) ){               // In the Left Col
        return true;
    }
    return false;
}

bool RasterArray::IsDirectionValid(int ID, eDirection dir){
    // We want to make sure the direction we're asking for is not out of bounds
    bool bValid = true;
    if (       ( IsTopEdge(ID)    && (dir == DIR_NW || dir == DIR_N || dir == DIR_NE) )
            || ( IsBottomEdge(ID) && (dir == DIR_SE || dir == DIR_S || dir == DIR_SW) )
            || ( IsRightEdge(ID)  && (dir == DIR_NE || dir == DIR_E || dir == DIR_SE) )
            || ( IsLeftEdge(ID)   && (dir == DIR_SW || dir == DIR_W || dir == DIR_NW) )
            ){
        bValid = false;
    }
    return bValid;
}

int RasterArray::GetIDFromCoords(int row, int col){
    return  row*( GetCols() )+ col;
}

void RasterArray::TestDir(size_t id){
    GetNeighbors(id);
    //
    //    |0|1|2|
    //    -------
    //    |7|X|3|
    //    -------
    //    |6|5|4|
    //    -------

    qDebug() << QString("Top: %1 Right: %2 Bottom: %3 Left %4").arg(IsTopEdge(id)).arg(IsRightEdge(id)).arg(IsBottomEdge(id)).arg(IsLeftEdge(id));

    qDebug() << "------------";
    qDebug() << QString("|%1|%2|%3|").arg(Neighbors.at(DIR_NW)).arg(Neighbors.at(DIR_N)).arg(Neighbors.at(DIR_NE));
    qDebug() << "------------";
    qDebug() << QString("|%1|%2|%3|").arg(Neighbors.at(DIR_W)).arg("X").arg(Neighbors.at(DIR_E));
    qDebug() << "------------";
    qDebug() << QString("|%1|%2|%3|").arg(Neighbors.at(DIR_SW)).arg(Neighbors.at(DIR_S)).arg(Neighbors.at(DIR_SE));
    qDebug() << "------------";


}

}
