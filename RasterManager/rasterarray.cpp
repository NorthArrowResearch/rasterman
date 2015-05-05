#include "rasterarray.h"
#include "rastermanager_exception.h"
#include <QDebug>

namespace RasterManager {

RasterArray::RasterArray(const char * psFilePath) : Raster(psFilePath)
{
    //Resize vectors
    Terrain.resize(GetTotalCells()); //Values input from file, modified throughout program
    Checked.resize(GetTotalCells());
    Neighbors.resize(8);

    invalidID = std::numeric_limits<size_t>::max();

    // Set up the GDal Dataset and rasterband info
    GDALDataset * pDSInput = (GDALDataset*) GDALOpen(FilePath(), GA_ReadOnly);
    if (pDSInput == NULL)
        throw RasterManagerException(INPUT_FILE_ERROR, "Could not open input Raster");
    GDALRasterBand * pRBInput = pDSInput->GetRasterBand(1);

    // Setup our Read buffer and read the entire raster into an array
    double * pInputLine = (double *) CPLMalloc(sizeof(double)*GetCols());
    for (int i = 0; i < pRBInput->GetYSize(); i++){
        pRBInput->RasterIO(GF_Read, 0,  i, pRBInput->GetXSize(), 1, pInputLine, pRBInput->GetXSize(), 1, GDT_Float64, 0, 0);
        for (int j = 0; j < pRBInput->GetXSize(); j++){
            int nIndex = GetIDFromCoords(i,j); //Convert a 2d index to a 1D one
            Terrain.at(nIndex) = pInputLine[j];
        }
    }
    CPLFree(pInputLine);
}

bool RasterArray::operator ==( RasterArray &src)
{
    if (IsConcurrent(&src) && CellCompare(&src) )
        return true;
    else
        return false;
}
bool RasterArray::operator !=( RasterArray &src){
    return !(*this==src);
}

bool RasterArray::CellCompare(RasterArray * raArray2){

    if (!IsConcurrent(raArray2))
        return false;

    for (size_t ID = 0; ID < GetTotalCells(); ID++){
        if (!qFuzzyCompare(Terrain.at(ID), raArray2->GetCell(ID))){
            return false;
        }
    }
    return true;
}


void RasterArray::WriteArraytoRaster(QString sOutputPath, std::vector<double> *vPointArray, GDALDataType * dataType){

    // Create the output dataset for writing
    const QByteArray csOutput = sOutputPath.toLocal8Bit();

    RasterMeta Output = *this;

    if (dataType == NULL){
        dataType = this->GetGDALDataType();
    }

    Output.SetGDALDataType(dataType);

    // Set the bounds and nodata to be the same as the input
    GDALDataset * pDSOutput = CreateOutputDS(csOutput.data(), &Output);

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
void RasterArray::WriteArraytoRaster(QString sOutputPath, std::vector<int> *vPointArray, GDALDataType * dataType){
    std::vector<double> vDouble(vPointArray->begin(), vPointArray->end());
    WriteArraytoRaster(sOutputPath, &vDouble, dataType);
}
void RasterArray::WriteArraytoRaster(QString sOutputPath, std::vector<size_t> *vPointArray, GDALDataType * dataType){
    std::vector<double> vDouble(vPointArray->begin(), vPointArray->end());
    WriteArraytoRaster(sOutputPath, &vDouble, dataType);
}

bool RasterArray::IsBorder(size_t ID)
{
    //Tests if cell is on the outer edge of the grid, and thus is an outlet
    if ( IsTopEdge(ID) || IsBottomEdge(ID) || IsLeftEdge(ID) || IsRightEdge(ID) ){               // In the Left Col
        return true;
    }
    return false;
}

bool RasterArray::IsDirectionValid(size_t ID, eDirection dir){
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

size_t RasterArray::GetNeighborID(size_t id, eDirection dir){
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
        default: return invalidID; break;
        }
    }
    else{
        // We're out of bounds
        return invalidID;
    }
}

double RasterArray::GetNeighborVal(size_t ID, eDirection dir){
    if (IsDirectionValid(ID, dir))
        return Terrain.at(GetNeighborID(ID, dir));
    else
        return invalidID;
}

void RasterArray::PopulateNeighbors(int ID)
{
    for (int d = DIR_NW; d <= DIR_W; d++)
    {
        Neighbors.at(d) = GetNeighborID(ID, (eDirection) d);
    }
}

size_t RasterArray::getCol(size_t i){
    return (int) floor((double)(i % GetCols()));
}
size_t RasterArray::getRow(size_t i){
    return (int) floor((double)((i / GetCols())));
}


bool RasterArray::HasValidNeighbor(size_t ID){
    // Opposite of Neighbournovalue. Returns true if there are any valid neighbors.
    // In this case valid means not out of bounds and not NoData
    PopulateNeighbors(ID);
    for (int d = DIR_NW; d <= DIR_W; d++)
    {
        if (IsDirectionValid(ID, (eDirection)d ) && Terrain.at(Neighbors.at(d)) != GetNoDataValue() )
            return true;
    }
    return false;
}



int RasterArray::GetIDFromCoords(int row, int col){
    return  row*( GetCols() )+ col;
}

void RasterArray::TestNeighbourVal(size_t id){
    PopulateNeighbors(id);
    //
    //    |0|1|2|
    //    -------
    //    |7|X|3|
    //    -------
    //    |6|5|4|
    //    -------

    qDebug() << QString("Top: %1 Right: %2 Bottom: %3 Left %4").arg(IsTopEdge(id)).arg(IsRightEdge(id)).arg(IsBottomEdge(id)).arg(IsLeftEdge(id));
    qDebug() << "------------";
    qDebug() << QString("|%1|%2|%3|").arg(GetNeighborVal(id, DIR_NW)).arg(GetNeighborVal(id, DIR_N)).arg(GetNeighborVal(id, DIR_NE));
    qDebug() << "------------";
    qDebug() << QString("|%1|%2|%3|").arg(GetNeighborVal(id, DIR_W)).arg("X").arg(GetNeighborVal(id, DIR_E));
    qDebug() << "------------";
    qDebug() << QString("|%1|%2|%3|").arg(GetNeighborVal(id, DIR_SW)).arg(GetNeighborVal(id, DIR_S)).arg(GetNeighborVal(id, DIR_SE));
    qDebug() << "------------";


}


void RasterArray::TestChecked(size_t id){
    PopulateNeighbors(id);
    qDebug() << "------------";
    qDebug() << QString("|%1|%2|%3|").arg(IsChecked(Neighbors.at(DIR_NW))).arg(IsChecked(Neighbors.at(DIR_N))).arg(IsChecked(Neighbors.at(DIR_NE)));
    qDebug() << "------------";
    qDebug() << QString("|%1|%2|%3|").arg(IsChecked(Neighbors.at(DIR_W))).arg("X").arg(IsChecked(Neighbors.at(DIR_E)));
    qDebug() << "------------";
    qDebug() << QString("|%1|%2|%3|").arg(IsChecked(Neighbors.at(DIR_SW))).arg(IsChecked(Neighbors.at(DIR_S))).arg(IsChecked(Neighbors.at(DIR_SE)));
    qDebug() << "------------";

}


void RasterArray::TestNeighbourID(size_t id){
    PopulateNeighbors(id);
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
