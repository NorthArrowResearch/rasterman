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

RasterPitRemoval::RasterPitRemoval(const char * sRasterInput, const char * sRasterOutput,
                                   FillMode eMethod) : RasterArray(sRasterInput){

    // Basic File existence Checking
    CheckFile(sRasterOutput, false);

    sOutputPath = QString(sRasterOutput);
    Mode = eMethod;

    //Resize vectors
    FloodDirection.resize(GetTotalCells());
    Flooded.resize(GetTotalCells());
    BlankBool.resize(GetTotalCells());
    IsPit.resize(GetTotalCells());

    SavePits = true;
}

RasterPitRemoval::~RasterPitRemoval()
{
}

int RasterPitRemoval::Run(){

    // Setup our Read buffer and read the entire raster into an array
    for (size_t nIndex=0; nIndex < (size_t) GetTotalCells(); nIndex++)
    {
        Flooded.at(nIndex) = UNFLOODED;
        FloodDirection.at(nIndex) = INIT;
        IsPit.at(nIndex) = 0;
    }
    //    WriteArraytoRaster(appendToBaseFileName(sOutputPath, "_DEBUG-1-original"), &Terrain); // DEBUG ONLY

    //The entire DEM is scanne3d and all outlets are added to the Main Queue
    //An outlet is defined as a cell that is either on the border of the grid or has a neighbor with no_data
    //This allows internal points of no data to be used as outlets
    for (size_t i=0; i < (size_t) GetTotalCells(); i++)
    {
        //Test if cell is on border or if cell has a neighbor with no data
        if(HasValidNeighbor(i) &&
                (IsBorder(i) || NeighborNoValue(i) ) ){
            AddToMainQueue(i, true); //All outlet cells by definition have a path to the outlet, thus ConfirmDescend = true.
            FloodDirection.at(i) = FLOODSOURCE;
        }
    }

    // Iterate Main Queue, removing all pits
    // ------------------------------------------------------
    IterateMainQueue();
    WriteArraytoRaster(sOutputPath, &Terrain, NULL);

    //    DEBUG
    //    debugFunc();

    return PROCESS_OK;
}

void RasterPitRemoval::debugFunc(){
    // THESE ARE ALL DEBUG .. Test the various functions that make the decisions

    std::vector<double> borders;        // Something to fill up and test the output
    std::vector<double> nnv;        // Something to fill up and test the output
    std::vector<double> islocalmin;        // Something to fill up and test the output
    std::vector<double> hasvalidneigh;        // Something to fill up and test the output

    borders.resize(GetTotalCells());
    nnv.resize(GetTotalCells());
    islocalmin.resize(GetTotalCells());
    hasvalidneigh.resize(GetTotalCells());

    for (size_t i=0; i < (size_t) GetTotalCells(); i++){
        borders.at(i) = (double) IsBorder(i);
        nnv.at(i) = (double) NeighborNoValue(i);
        islocalmin.at(i) = (double) IsLocalMinimum(i);
        hasvalidneigh.at(i) = (double) HasValidNeighbor(i);
    }

    // DEBUG LOOP
    //    WriteArraytoRaster(appendToBaseFileName(sOutputPath, "_DEBUG-IsBorder"), &borders);
    //    WriteArraytoRaster(appendToBaseFileName(sOutputPath, "_DEBUG-NeighborNoValue"), &nnv);
//    WriteArraytoRaster(appendToBaseFileName(sOutputPath, "_DEBUG-2-IsLocalMinimum"), &islocalmin, GDT_Float64);
    //    WriteArraytoRaster(appendToBaseFileName(sOutputPath, "_DEBUG-HasValidNeighbor"), &hasvalidneigh);
    WriteArraytoRaster(appendToBaseFileName(sOutputPath, "_DEBUG-3-Depressions"), &IsPit, NULL);
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
                PopulateNeighbors(CurCell.id);
                for (int d = DIR_NW; d <= DIR_W; d++)
                {
                    if (IsDirectionValid(CurCell.id, (eDirection) d))
                    {
                        if ((Flooded.at(Neighbors.at(d)) == FLOODEDDESC )
                                && (Terrain.at(Neighbors.at(d))<=Terrain.at(CurCell.id)))
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

        if (Terrain.at(ID) != GetNoDataValue()
 && HasValidNeighbor(ID) ){
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
        NextID = TraceFlow(CurID, FloodDirection.at(CurID));
        if(NextID < 0) //CurID is a border cell
            ReachedOutlet = 1;
        else if (Terrain.at(NextID) == GetNoDataValue()
) //CurID is next to an internal outlet
            ReachedOutlet = 1;
        else if((Terrain.at(NextID) < Terrain.at(PitID))&&(Flooded.at(NextID)==FLOODEDDESC)) //NextID is lower than Pit and NextID on confirmed descending path to outlet
            ReachedOutlet = 1;
        else
        {
            if((Terrain.at(NextID) > Crest) && (Terrain.at(NextID) != GetNoDataValue()
)) Crest = Terrain.at(NextID);
        }
        CurID = NextID;
    }
    return Crest;
}


bool RasterPitRemoval::CheckCell(int ID, int CurNeighborID, double CrestElev)
{
    bool bCheckCell = true;

    if (CurNeighborID > -1){
        // Here are the cases for which we do not check the neighbor cell:
        if (       IsChecked(CurNeighborID)                    // neighbor has been checked already
                || Terrain.at(CurNeighborID) > CrestElev       // OR we're above the fill line
                || Terrain.at(CurNeighborID) < Terrain.at(ID)  // OR neighbour is above our current cell
                ){
            bCheckCell = false;
        }
        // Mark neighbour as checked
        SetChecked(CurNeighborID);
    }
    else{
        // We're out of bounds
        bCheckCell = false;
    }
    return bCheckCell;
}

void RasterPitRemoval::SetFlowDirection(int FromID, int ToID)
{
    //If two cells are not neighbors or if a neighbor is off the grid/ has no_data, direction set to 0.
    int numCols = GetCols();

    if(ToID == FromID + 1)
        FloodDirection.at(FromID) = DIR_E;   //Flow is from East
    else if(ToID == FromID + 1 + numCols)
        FloodDirection.at(FromID) = DIR_SE;   //Flow is from Southeast
    else if(ToID == FromID + numCols)
        FloodDirection.at(FromID) = DIR_S;   //Flow is from South
    else if(ToID == FromID - 1 + numCols)
        FloodDirection.at(FromID) = DIR_SW;   //Flow is from Southwest
    else if(ToID == FromID - 1)
        FloodDirection.at(FromID) = DIR_W;  //Flow is from West
    else if(ToID == FromID - 1 - numCols)
        FloodDirection.at(FromID) = DIR_NW;  //Flow is from Northwest
    else if(ToID == FromID - numCols)
        FloodDirection.at(FromID) = DIR_N;  //Flow is from North
    else if(ToID == FromID + 1 - numCols)
        FloodDirection.at(FromID) = DIR_NE; //Flow is from Northeast
    else
        FloodDirection.at(FromID) = FLOODSOURCE;
}

int RasterPitRemoval::TraceFlow(int FromID, int eFlowDir)
{
    //Returns the cell pointed to by the direction grid at the given location.
    //If flow direction equals 0, This is a border cell. Return -1.
    switch (eFlowDir) {
    case DIR_E:  return FromID + 1;              break;
    case DIR_SE: return FromID + 1 + GetCols();  break;
    case DIR_S:  return FromID + GetCols();      break;
    case DIR_SW: return FromID - 1 + GetCols();  break;
    case DIR_W:  return FromID - 1;              break;
    case DIR_NW: return FromID - 1 - GetCols();  break;
    case DIR_N:  return FromID - GetCols();      break;
    case DIR_NE: return FromID + 1 - GetCols();  break;
    default: return FLOODSOURCE;                 break;
    }
    return FLOODSOURCE;
}

bool RasterPitRemoval::NeighborNoValue(int ID)
{
    //Tests if cell is next to a cell with no_data, and thus is an outlet
    bool novalue = false;

    PopulateNeighbors(ID);

    for (int d = DIR_NW; d <= DIR_W; d++)
    {
        if ( !IsDirectionValid(ID, (eDirection)d) )
        {
            novalue = true;
            break;
        }
        else if (Terrain.at(Neighbors.at(d)) == GetNoDataValue() )
        {
            novalue = true;
            SetFlowDirection(ID, Neighbors.at(d));
            break;
        }
    }
    return novalue;
}


void RasterPitRemoval::GetDryNeighbors(int ID)
{
    //Adds all neighbors which have not yet been flooded to NeighborQueue.

    point Neighbor;
    PopulateNeighbors(ID);

    for (int d = DIR_NW; d <= DIR_W; d++)
    {
        if (IsDirectionValid(ID, (eDirection)d))
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
    if ((Terrain.at(PitID) < FillElev) && (Terrain.at(PitID) != GetNoDataValue()
)) {
        Terrain.at(PitID) = FillElev;
    }
    for (size_t i=0; i < Depression.size(); i++)
    {
        CurID = Depression.at(i);
        if ((Terrain.at(CurID) < FillElev) && (Terrain.at(CurID) != GetNoDataValue()
))
        {
            Terrain.at(CurID) = FillElev;
        }
    }
}

void RasterPitRemoval::PitRemoveHybrid(int PitID)
{

    double CrestElev;
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
        for (int d = DIR_NW; d <= DIR_W; d++)
        {
            int CurNeighborID =  GetNeighborID(CurID, (eDirection)d);

            if ( CurNeighborID > -1 && CheckCell( CurID, CurNeighborID, CrestElev ) )
            {
                NeighborPoint.id=CurNeighborID;
                NeighborPoint.elev=Terrain.at(CurNeighborID);

                DepressionQueue.push(NeighborPoint);
                Depression.push_back(CurNeighborID);
            }
        }
    }
    ResetChecked();

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
    else
    {
        PopulateNeighbors(CurID);
        for (int d = DIR_NW; d <= DIR_W; d++)
        {
            if (IsDirectionValid(CurID, (eDirection) d)){
                double currVal = Terrain.at(CurID);
                double neighborVal = Terrain.at(Neighbors.at(d));
                double neighborFlood = Flooded.at(Neighbors.at(d));

                if( neighborVal < currVal || ( neighborVal == currVal && ( neighborFlood  == UNFLOODED ) ) )
                    IsMinimum = false;
            }
        }
    }

    // Save the pits for later if we want to output them
    if (SavePits && IsMinimum)
        IsPit.at(CurID) = 1;

    return IsMinimum;
}

}
