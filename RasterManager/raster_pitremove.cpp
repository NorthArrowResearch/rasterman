#define MY_DLL_EXPORT
/*
 * Raster Fill -- Optimized Pit Removal
 *
 * MArch 2, 2015
 *
 *  This code is adapted from https://github.com/crwr/OptimizedPitRemoval
 *
 * ============== OPTIMIZED PIT REMOVAL V1.5.1 ================================
 * Stephen Jackson, Center for Research in Water Resources, University of Texas at Austin;
 * srj9@utexas.edu
 * Acknowledgements: Soille, P. (2004), Optimal removal of spurious pits in grid digital elevation models, Water Resour. Res., 40, W12509;
 * Dr. David Maidment, University of Texas at Austin;
 * Dr. David Tarboton, Utah State University;
 * TauDEM (Terrain Analysis Using Digital Elevation Models), Hydrology Research Group, Utah State University
 *
 *
*/

#include "raster_pitremove.h"
#include "rastermanager_interface.h"
#include "rastermanager_exception.h"
#include "raster.h"
#include "gdal.h"
#include "gdal_priv.h"
#include "rastermanager.h"

namespace RasterManager {

const double PI =  3.14159265359;
const double aref[10] = { -atan2((double)1,(double)1), 0., -aref[0],(double)(0.5*PI),PI-aref[2],(double)PI,
                          PI+aref[2],(double)(1.5*PI),2.*PI-aref[2],(double)(2.*PI) };   // DGT is concerned that this assumes square grids.  For different dx and dx needs adjustment

int nameadd( char*,char*,char*);
double prop( float a, int k);
int readoutlets(char *outletsfile, int *noutlets, double*& x, double*& y);
int readoutlets(char *outletsfile, int *noutlets, double*& x, double*& y, int*& id);

RasterPitRemoval::RasterPitRemoval(const char * sRasterInput,
                                   const char * sRasterOutput,
                                   FillMode eMethod){


    ProcessTimer initTimer("");
    QueueItem = new LoopTimer("Queue Item");

    // Basic File existence Checking
    CheckFile(sRasterInput, true);
    CheckFile(sRasterOutput, false);

    sInputPath = QString(sRasterInput);
    sOutputPath = QString(sRasterOutput);
    Mode = eMethod;

    // Import Raster
    // ------------------------------------------------------
    rInputRaster = new Raster(sRasterInput);

    // Set up a reasonable nodata value
    if (rInputRaster->GetNoDataValuePtr() == NULL)
        dNoDataValue = (double) -std::numeric_limits<float>::max();
    else
        dNoDataValue = rInputRaster->GetNoDataValue();

    int nNumCells = rInputRaster->GetCols()*rInputRaster->GetRows();

    //Resize vectors
    Terrain.resize(nNumCells); //Values input from file, modified throughout program
    Direction.resize(nNumCells);
    Flooded.resize(nNumCells);
    Checked.resize(nNumCells);
    BlankBool.resize(nNumCells);
    IsPit.resize(nNumCells);
    Neighbors.resize(8);

    TotalCells = Terrain.size();
    rasterCols = rInputRaster->GetCols();

    SavePits = true;
}

RasterPitRemoval::~RasterPitRemoval()
{
    delete rInputRaster;
    delete QueueItem;
}

int RasterPitRemoval::Run(){

    // Set up the GDal Dataset and rasterband info
    GDALDataset * pDSInput = (GDALDataset*) GDALOpen(rInputRaster->FilePath(), GA_ReadOnly);
    if (pDSInput == NULL)
        throw RasterManagerException(INPUT_FILE_ERROR, "Could not open input Raster");
    GDALRasterBand * pRBInput = pDSInput->GetRasterBand(1);

    // Setup our Read buffer and read the entire raster into an array
    double * pInputLine = (double *) CPLMalloc(sizeof(double)*rInputRaster->GetCols());
    for (int i = 0; i < pRBInput->GetYSize(); i++){
        pRBInput->RasterIO(GF_Read, 0,  i, pRBInput->GetXSize(), 1, pInputLine, pRBInput->GetXSize(), 1, GDT_Float64, 0, 0);
        for (int j = 0; j < pRBInput->GetXSize(); j++){
            int nIndex = i*rInputRaster->GetCols() + j; //Convert a 2d index to a 1D one
            Terrain.at(nIndex) = pInputLine[j];
            Flooded.at(nIndex) = UNFLOODED;
            Direction.at(nIndex) = INIT;
            IsPit.at(nIndex) = 0;
        }
    }
    CPLFree(pInputLine);

    //The entire DEM is scanne3d and all outlets are added to the Main Queue
    //An outlet is defined as a cell that is either on the border of the grid or has a neighbor with no_data
    //This allows internal points of no data to be used as outlets
    for (size_t i=0; i < (size_t) TotalCells; i++)
    {
        //Test if cell is on border or if cell has a neighbor with no data
        if(HasValidNeighbor(i) &&
                (IsBorder(i) || NeighborNoValue(i) ) ){
            AddToMainQueue(i, true); //All outlet cells by definition have a path to the outlet, thus ConfirmDescend = true.
            Direction.at(i) = ENTRYPOINT;
        }
    }


    // Iterate Main Queue, removing all pits
    // ------------------------------------------------------
    IterateMainQueue();
    WriteArraytoRaster(sOutputPath, &Terrain);

    //    DEBUG
//    debugFunc();

    return PROCESS_OK;
}


void RasterPitRemoval::debugFunc(){
    // THESE ARE ALL DEBUG
    std::vector<double> debugArr;        // Something to fill up and test the output
    debugArr.resize(TotalCells);

    for (size_t i=0; i < (size_t) TotalCells; i++){
        debugArr.at(i) = (int) Direction.at(i);
    }

    // DEBUG LOOP
    WriteArraytoRaster(appendToBaseFileName(sOutputPath, "_DEBUG-direction"), &debugArr);

}

void RasterPitRemoval::WriteArraytoRaster(QString sOutputPath, std::vector<int> *vPointArray){
    std::vector<double> vDouble(vPointArray->begin(), vPointArray->end());
    WriteArraytoRaster(sOutputPath, &vDouble);
}

void RasterPitRemoval::WriteArraytoRaster(QString sOutputPath, std::vector<double> *vPointArray ){

    // Create the output dataset for writing
    const QByteArray csOutput = sOutputPath.toLocal8Bit();

    // Set the bounds and nodata to be the same as the input
    GDALDataset * pDSOutput = CreateOutputDS(csOutput.data(), rInputRaster);
    double * pOutputLine = (double *) CPLMalloc(sizeof(double)*rInputRaster->GetCols());

    // Write rows and columns
    for (int i = 0; i < rInputRaster->GetRows(); i++)
    {
        for (int j = 0; j < rInputRaster->GetCols(); j++){
            pOutputLine[j] = vPointArray->at(i*rInputRaster->GetCols() + j);
        }
        pDSOutput->GetRasterBand(1)->RasterIO(GF_Write, 0,  i, rInputRaster->GetCols(), 1, pOutputLine, rInputRaster->GetCols(), 1, GDT_Float64, 0, 0);
    }
    CPLFree(pOutputLine);
    GDALClose(pDSOutput);

}

void RasterPitRemoval::IterateMainQueue()
{
    // MainQueue will gradually have all cells in grid added to it, starting from the borders and those next to no_data_value cells and
    // progressing upward with a rising flood.
    // Each cell is removed from the queue in order of lowest elevation and checked to see if it is a pit minimum.
    // If it is a minimum, the pit removal procedures are run.
    // After each cell is removed from the queue, each of its unflooded neighbors is added.
    // The vector Direction keeps track of which neighboring cell caused that cell to become flooded. This can be used to trace a natural path to an outlet.
    // This continues until all cells have been flooded and the Main Queue is empty, and thus all pits removed.
    // Note: Preprocessing to identify all pits does not seem a good strategy, as the number of pits which need to be handled is dependant on the step size.
    // (A coarser step size will cause more pits to be filled before they are removed from the queue.)

    point CurCell;
    point CurNeighbor;
    int CurPitNum=0;
    bool ConfirmDescend=false;

    while(!MainQueue.empty())
    {

        CurCell = MainQueue.top();
        MainQueue.pop();

        //Check Cell to determine if it is a Pit Minimum
        if(IsLocalMinimum(CurCell.id))
        {
            PitRemoveHybrid(CurCell.id);
            CurPitNum++;
        }
        else //Some cells within a depression may still be classified as Flooded=1 after pit has been removed. Need to correct this
        {
            if(Flooded.at(CurCell.id) == FLOODED )
            {
                GetNeighbors(CurCell.id);
                int i;
                for (i=DIR_NW;i<DIR_W;i++)
                {
                    if (Neighbors.at(i)>-1)
                    {
                        if ((Flooded.at(Neighbors.at(i)) == FLOODEDDESC ) && (Terrain.at(Neighbors.at(i))<=Terrain.at(CurCell.id)))
                        {
                            Flooded.at(CurCell.id) = FLOODEDDESC;
                            break;
                        }
                    }
                }
            }
        }

        // Add unflooded neighbors to Main Queue and identify the direction the flooding came from
        GetDryNeighbors(CurCell.id);
        while(!NeighborQueue.empty())
        {
            CurNeighbor = NeighborQueue.top();
            NeighborQueue.pop();

            ConfirmDescend = false;

            if(Terrain.at(CurNeighbor.id)>=Terrain.at(CurCell.id)
                    && Flooded.at(CurCell.id) == FLOODEDDESC )
                ConfirmDescend=true;

            if (HasValidNeighbor(CurNeighbor.id)){

                AddToMainQueue( CurNeighbor.id, ConfirmDescend );
                SetFlowDirection( CurNeighbor.id, CurCell.id );

            }
        }
    }
}

void RasterPitRemoval::AddToMainQueue(int ID, bool ConfirmDescend)
{
    //Adds cell to main queue. Input ConfirmDescend = true if there is a known continuously descending path to an outlet.
    //Cells are added to Main Queue at the moment they become flooded.
    //The vector Flooded is a permanent record of whether each cell has been flooded and whether there is a known path to an outlet

    point CurCell;
    if (Flooded.at(ID) == UNFLOODED)
    {
        CurCell.id = ID;
        CurCell.elev = Terrain.at(ID);

        if (Terrain.at(ID) != dNoDataValue && HasValidNeighbor(ID) ){
            MainQueue.push(CurCell);
        }


        if(ConfirmDescend)
        {
            Flooded.at(ID)= FLOODEDDESC;
        }
        else
        {
            Flooded.at(ID) = FLOODED;
        }

    }
}

double RasterPitRemoval::GetCrestElevation(int PitID)
{
    //Find the highest point along the path to an outlet
    bool ReachedOutlet = 0;
    double Crest;
    int CurID, NextID;

    //Initialize Crest as the Pit Minimum elevation
    CurID = PitID;
    Crest = PitElev;

    while(!ReachedOutlet)
    {
        NextID = TraceFlow(CurID, (eDirection) Direction.at(CurID));
        if(NextID < 0) //CurID is a border cell
            ReachedOutlet = 1;
        else if (Terrain.at(NextID) == dNoDataValue) //CurID is next to an internal outlet
            ReachedOutlet = 1;
        else if((Terrain.at(NextID) < Terrain.at(PitID))&&(Flooded.at(NextID)==FLOODEDDESC)) //NextID is lower than Pit and NextID on confirmed descending path to outlet
            ReachedOutlet = 1;
        else
        {
            if((Terrain.at(NextID) > Crest) && (Terrain.at(NextID) != dNoDataValue)) Crest = Terrain.at(NextID);
        }
        CurID = NextID;
    }
    return Crest;
}


bool RasterPitRemoval::CheckCell(int ID, eDirection dir, int& CurNeighborID, double CrestElev)
{
    bool bValid = IsDirectionValid(ID, dir)  // If the direction we're asking for isn't off the grid
            && !Checked.at(CurNeighborID)    // AND this value hasn't been checked already
            && Terrain.at(CurNeighborID) < CrestElev // AND we're below the fill line
            && Terrain.at(CurNeighborID)>=Terrain.at(ID); // AND we're below our source cell

   Checked.at(CurNeighborID) = true;

   return bValid;
}

void RasterPitRemoval::SetFlowDirection(int FromID, int ToID)
{
    //If two cells are not neighbors or if a neighbor is off the grid/ has no_data, direction set to 0.
    int numCols = rInputRaster->GetCols();

    if(ToID == FromID + 1)
        Direction.at(FromID) = DIR_E;   //Flow is to East
    else if(ToID == FromID + 1 + numCols)
        Direction.at(FromID) = DIR_SE;   //Flow is to Southeast
    else if(ToID == FromID + numCols)
        Direction.at(FromID) = DIR_S;   //Flow is to South
    else if(ToID == FromID - 1 + numCols)
        Direction.at(FromID) = DIR_SW;   //Flow is to Southwest
    else if(ToID == FromID - 1)
        Direction.at(FromID) = DIR_W;  //Flow is to West
    else if(ToID == FromID - 1 - numCols)
        Direction.at(FromID) = DIR_NW;  //Flow is to Northwest
    else if(ToID == FromID - numCols)
        Direction.at(FromID) = DIR_N;  //Flow is to North
    else if(ToID == FromID + 1 - numCols)
        Direction.at(FromID) = DIR_NE; //Flow is to Northeast
    else
        Direction.at(FromID) = ENTRYPOINT;
}

int RasterPitRemoval::TraceFlow(int FromID, eDirection eFlowDir)
{
    //Returns the cell pointed to by the direction grid at the given location.
    //If flow direction equals 0, This is a border cell. Return -1.
    int numCols = rInputRaster->GetCols();

    switch (eFlowDir) {
    case DIR_E:  return FromID + 1; break;
    case DIR_SE: return FromID + 1 + numCols; break;
    case DIR_S:  return FromID + numCols; break;
    case DIR_SW: return FromID - 1 + numCols; break;
    case DIR_W:  return FromID - 1; break;
    case DIR_NW: return FromID - 1 - numCols; break;
    case DIR_N:  return FromID - numCols; break;
    case DIR_NE: return FromID + 1 - numCols; break;
    default: return ENTRYPOINT; break;
    }
    return ENTRYPOINT;
}

void RasterPitRemoval::GetDryNeighbors(int ID)
{
    //Adds all neighbors which have not yet been flooded to NeighborQueue.

    point Neighbor;
    GetNeighbors(ID);

    for (int d=DIR_NW;d<DIR_W;d++)
    {
        if (Neighbors.at(d)>-1)
        {
            // If it's dry then push it onto the dry queue
            if (Flooded.at(Neighbors.at(d)) == UNFLOODED) {
                Neighbor.id = Neighbors.at(d);
                Neighbor.elev = Terrain.at(Neighbors.at(d));
                NeighborQueue.push(Neighbor);
            }
        }
    }
}

void RasterPitRemoval::FillToElevation(int PitID, double FillElev)
{
    //Fills all cells within a depression to the specified elevation
    int CurID;
    if ((Terrain.at(PitID) < FillElev) && (Terrain.at(PitID) != dNoDataValue)) {
        Terrain.at(PitID) = FillElev;
    }
    for (size_t i=0; i < Depression.size(); i++)
    {
        CurID = Depression.at(i);
        if ((Terrain.at(CurID) < FillElev) && (Terrain.at(CurID) != dNoDataValue))
        {
            Terrain.at(CurID) = FillElev;
        }
    }
}

bool RasterPitRemoval::IsBorder(int ID)
{
    //Tests if cell is on the outer edge of the grid, and thus is an outlet
    if ( IsTopEdge(ID) || IsBottomEdge(ID) || IsLeftEdge(ID) || IsRightEdge(ID) ){               // In the Left Col
        return true;
    }
    return false;
}

bool RasterPitRemoval::IsDirectionValid(int ID, eDirection dir){
    // We want to make sure the direction we're asking for is not
    if (IsTopEdge(ID)){
        if (dir >= DIR_NW && dir <= DIR_NE){
            return false;
        }
    }
    else if(IsBottomEdge(ID)){
        if (dir >= DIR_SE && dir <= DIR_SW){
            return false;
        }
    }
    else if (IsRightEdge(ID)){
        if (dir >= DIR_NE && dir <= DIR_SE){
            return false;
        }
    }
    else if (IsLeftEdge(ID)){
        if (dir >= DIR_SW && dir <= DIR_W  && dir != DIR_NW){
            return false;
        }
    }
    return true;
}

bool RasterPitRemoval::NeighborNoValue(int ID)
{
    //Tests if cell is next to a cell with no_data, and thus is an outlet
    bool novalue = false;

    GetNeighbors(ID);

    for (int d=DIR_NW;d<DIR_W;d++)
    {
        if ( Neighbors.at(d)== ENTRYPOINT )
        {
            novalue = true;
            break;
        }
        else if (Terrain.at(Neighbors.at(d)) == dNoDataValue )
        {
            novalue = true;
            SetFlowDirection(ID, Neighbors.at(d));
            break;
        }
    }
    return novalue;
}

bool RasterPitRemoval::HasValidNeighbor(int ID){
    // Opposite of Neighbournovalue. Returns true if there are any valid neighbors.
    GetNeighbors(ID);
    for (int d=DIR_NW;d<DIR_W;d++)
    {
        if (Neighbors.at(d) != ENTRYPOINT && Terrain.at(Neighbors.at(d)) != dNoDataValue)
            return true;
    }
    return false;
}

void RasterPitRemoval::PitRemoveHybrid(int PitID)
{

    double CrestElev, IdealFillLevel;
    PitElev = Terrain.at(PitID);

    //Get Crest Elevation
    CrestElev = GetCrestElevation(PitID);

    //Get Depression Extents and set PitElev
    GetDepressionExtent(PitID, CrestElev);

    if(PitElev < CrestElev)
    {
        //Modify Terrain to remove Pit
        FillToElevation(PitID, CrestElev);
    }

    Flooded.at(PitID) = FLOODEDDESC; //Confirm that cell has descending path to outlet (i.e. Pit has been removed)

    //Reset global variables
    CutFunction = BlankMap;
    FillFunction = BlankMap;
    Depression = BlankInt;
}

void RasterPitRemoval::GetDepressionExtent(int PitID, double CrestElev)
{
    //Makes an elevation ordered list of every cell in the depression
    //A compound depression (neighboring pit with separating ridge lower than crest elevation) is treated as a separate depression. That pit will be removed later.
    int CurNeighborID;
    int CurID;
    point CurPoint;
    point NeighborPoint;

    CurPoint.id = PitID;
    CurPoint.elev = Terrain.at(PitID);
    DepressionQueue.push(CurPoint);
    Depression.push_back(CurPoint.id);

    while(!DepressionQueue.empty())
    {
        CurPoint = DepressionQueue.top();
        DepressionQueue.pop();
        CurID = CurPoint.id;

        //Get each neighbor cell that is lower than the Crest elevation and with elevation greater than or equal to the present cell
        for (int d=DIR_NW; d < DIR_W; d++)
        {
            if ( CheckCell( CurID, (eDirection)d, CurNeighborID, CrestElev ) )
            {
                NeighborPoint.id=CurNeighborID;
                NeighborPoint.elev=Terrain.at(CurNeighborID);

                DepressionQueue.push(NeighborPoint);
                Depression.push_back(CurNeighborID);
            }
        }
    }
    Checked = BlankBool;
}


int RasterPitRemoval::getCol(int i){
    return (int) floor(i % rInputRaster->GetCols());
}
int RasterPitRemoval::getRow(int i){
    return (int) floor(i / rInputRaster->GetCols());
}


bool RasterPitRemoval::IsLocalMinimum(int CurID)
{
    // A local Minimum has been found if the cell does not have a confirmed path to an outlet, and
    // all neighbors are either higher or the same elevation but flooded.
    // This means for a wide pit with a flat bottom, the last flooded cell (roughly farthest
    // from the outlet) is considered the minimum.

    bool IsMinimum = true;
    if (Flooded.at(CurID)==FLOODEDDESC)  //Cell is on confirmed path to outlet
    {
        IsMinimum = false;
    }
    else //Flooded = 1
    {
        GetNeighbors(CurID);
        for (int d=DIR_NW;d<DIR_W;d++)
        {
            if( Terrain.at(Neighbors.at(d)) < Terrain.at(CurID))
                IsMinimum = false;
            if((Terrain.at(Neighbors.at(d)) == Terrain.at(CurID)) && (Flooded.at(Neighbors.at(d)) == UNFLOODED))
                IsMinimum = false;
        }
    }

    if (SavePits == true)
        if (IsMinimum==true)
            IsPit.at(CurID) = 1;
    return IsMinimum;
}

void RasterPitRemoval::GetNeighbors(int ID)
{
    //Returns the ID value for the eight neighbors, with -1 for cells off the grid
    //Northwest
    if(ID<rasterCols)
        Neighbors.at(DIR_NW)=-1;
    else if (!(ID % rasterCols))
        Neighbors.at(DIR_NW)=-1;
    else
        Neighbors.at(DIR_NW)=ID-1-rasterCols;

    //North
    if(ID<rasterCols)
        Neighbors.at(DIR_N)=-1;
    else
        Neighbors.at(DIR_N)=ID-rasterCols;

    //Northeast
    if(ID<rasterCols)
        Neighbors.at(DIR_NE)=-1;
    else if (!((ID+1) % rasterCols))
        Neighbors.at(DIR_NE)=-1;
    else
        Neighbors.at(DIR_NE)=ID+1-rasterCols;

    //East
    if (!((ID+1) % rasterCols))
        Neighbors.at(DIR_E)=-1;
    else
        Neighbors.at(DIR_E)=ID+1;

    //Southeast
    if (TotalCells-ID <rasterCols + 1)
        Neighbors.at(DIR_SE)=-1;
    else if (!((ID+1) % rasterCols))
        Neighbors.at(DIR_SE)=-1;
    else
        Neighbors.at(DIR_SE)=ID+1+rasterCols;

    //South
    if (TotalCells-ID < rasterCols + 1)
        Neighbors.at(DIR_S)=-1;
    else
        Neighbors.at(DIR_S)= ID + rasterCols;

    //Southwest
    if (TotalCells-ID <rasterCols + 1)
        Neighbors.at(DIR_SW)=-1;
    else if (!(ID % rasterCols))
        Neighbors.at(DIR_SW)=-1;
    else
        Neighbors.at(DIR_SW)=ID - 1 + rasterCols;

    //West
    if (!(ID % rasterCols))
        Neighbors.at(DIR_W)=-1;
    else
        Neighbors.at(DIR_W)=ID-1;
}

}
