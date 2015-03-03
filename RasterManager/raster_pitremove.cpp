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
    Direction.resize(nNumCells);  //Starts empty
    Flooded.resize(nNumCells); //Starts empty
    Checked.resize(nNumCells);
    BlankBool.resize(nNumCells);
    IsPit.resize(nNumCells);

    TotalCells = Terrain.size();

}

RasterPitRemoval::~RasterPitRemoval()
{
    delete rInputRaster;
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
            Terrain.at(i) = pInputLine[i*rInputRaster->GetCols() + j];
        }
    }

    //The entire DEM is scanned and all outlets are added to the Main Queue
    //An outlet is defined as a cell that is either on the border of the grid or has a neighbor with no_data
    //This allows internal points of no data to be used as outlets
    for (size_t i=0; i < (size_t) TotalCells; i++)
    {
        //Test if cell is on border or if cell has a neighbor with no data
        if(IsBorder(i) || NeighborNoValue(i)){
            AddToMainQueue(i, true); //All outlet cells by definition have a path to the outlet, thus ConfirmDescend = true.
            Direction.at(i)=0;
        }
    }

    // Iterate Main Queue, removing all pits
    // ------------------------------------------------------
    IterateMainQueue();

    // Create the output dataset for writing
    const QByteArray csOutput = sOutputPath.toLocal8Bit();

    GDALDataset * pDSOutput = CreateOutputDS(csOutput.data(), rInputRaster);
    double * pOutputLine = (double *) CPLMalloc(sizeof(double)*rInputRaster->GetCols());

    // Write rows and columns
    for (int i = 0; i < rInputRaster->GetRows(); i++)
    {
        for (int j = 0; j < rInputRaster->GetCols(); j++)
            pOutputLine[j] = Terrain.at(i*rInputRaster->GetCols() + j);

        pDSOutput->GetRasterBand(1)->RasterIO(GF_Write, 0,  i, rInputRaster->GetCols(), 1, pOutputLine, rInputRaster->GetCols(), 1, GDT_Float64, 0, 0);
    }
    return PROCESS_OK;
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
            //User-selected method for removing pit
            if(Mode == FILL_CUT)
            {
                CutToElevation(CurCell.id);
            }
            else
            {
                PitRemoveHybrid(CurCell.id);
            }
            CurPitNum++;
        }
        else //Some cells within a depression may still be classified as Flooded=1 after pit has been removed. Need to correct this
        {
            if(Flooded.at(CurCell.id) == FLOODED)
            {
                GetNeighbors(CurCell.id);
                int i;
                for (i=0;i<8;i++)
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

        //Add unflooded neighbors to Main Queue and identify the direction the flooding came from
        GetDryNeighbors(CurCell.id);
        while(!NeighborQueue.empty())
        {
            CurNeighbor = NeighborQueue.top();
            NeighborQueue.pop();

            ConfirmDescend=false;
            if(Terrain.at(CurNeighbor.id)>=Terrain.at(CurCell.id)&&(Flooded.at(CurCell.id)== FLOODEDDESC))
                ConfirmDescend=true;
            AddToMainQueue(CurNeighbor.id, ConfirmDescend);
            SetFlowDirection(CurNeighbor.id, CurCell.id);
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
        MainQueue.push(CurCell);

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
        NextID = TraceFlow(CurID, (FlowDirection) Direction.at(CurID));
        if(NextID<0) //CurID is a border cell
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


bool RasterPitRemoval::CheckCell(int ID, int Direction, int& CurNeighborID, double CrestElev)
{
    bool Check = false;
    int Size = Terrain.size();
    int numCols = rInputRaster->GetCols();
    switch (Direction)
    {
    case 0: //Northwest
    {
        if(!(ID<numCols) && (ID % numCols))
        {
            CurNeighborID = ID-1-numCols;
            Check = true;
        }
    }
        break;
    case 1: //North
    {
        if(!(ID<numCols))
        {
            CurNeighborID = ID - numCols;
            Check = true;
        }
    }
        break;
    case 2: //Northeast
    {
        if(!(ID<numCols) && ((ID+1) % numCols))
        {
            CurNeighborID = ID+1-numCols;
            Check = true;
        }
    }
        break;
    case 3: //East

    {
        if (((ID+1) % numCols))
        {
            CurNeighborID = ID+1;
            Check = true;
        }
    }
        break;
    case 4: //Southeast
    {
        if (!(Size-ID <numCols + 1) && ((ID+1) % numCols))
        {
            CurNeighborID = ID+1+numCols;
            Check = true;
        }
    }
        break;
    case 5: //South
    {
        if (!(Size-ID <numCols + 1))
        {
            CurNeighborID = ID+numCols;
            Check = true;
        }
    }
        break;
    case 6: //Southwest
    {
        if (!(Size-ID <numCols + 1) && (ID % numCols))
        {
            CurNeighborID = ID-1+numCols;
            Check = true;
        }
    }
        break;
    case 7: //West
    {
        if ((ID % numCols))
        {
            CurNeighborID = ID-1;
            Check = true;
        }
    }
        break;
    }

    if (Check==true)
    {
        if (!( (Checked.at(CurNeighborID)==0) && (Terrain.at(CurNeighborID) < CrestElev) && (Terrain.at(CurNeighborID)>=Terrain.at(ID))))
        {
            Check = false;
        }
        Checked.at(CurNeighborID) = 1;
    }

    return Check;
}

void RasterPitRemoval::SetFlowDirection(int FromID, int ToID)
{
    //Flow direction is FROM current cell TO cell which caused it to flood, clockwise from East (1,2,4,8,16,32,64,128)
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
        Direction.at(FromID) = 0;
}

int RasterPitRemoval::TraceFlow(int FromID, FlowDirection eFlowDir)
{
    //Returns the cell pointed to by the direction grid at the given location.
    //If flow direction equals 0, This is a border cell. Return -1.
    int numCols = rInputRaster->GetCols();

    switch (eFlowDir) {
    case DIR_E:  return FromID + 1; break;  //1
    case DIR_SE: return FromID + 1 + numCols; break; //2
    case DIR_S:  return FromID + numCols; break; //4
    case DIR_SW: return FromID - 1 + numCols; break; //8
    case DIR_W:  return FromID - 1; break; //16
    case DIR_NW: return FromID - 1 - numCols; break; //32
    case DIR_N:  return FromID - numCols; break; //64
    case DIR_NE: return FromID + 1 - numCols; break; //128
    default: return -1; break;
    }
    return -1;
}

void RasterPitRemoval::GetDryNeighbors(int ID)
{
    //Adds all neighbors which have not yet been flooded to NeighborQueue.

    point DryNeighbor;
    GetNeighbors(ID);

    int i;
    for (i=0;i<8;i++)
    {
        if (Neighbors.at(i)>-1)
        {
            if (Flooded.at(Neighbors.at(i)) == UNFLOODED)
            {
                DryNeighbor.id = Neighbors.at(i);
                DryNeighbor.elev = Terrain.at(Neighbors.at(i));
                NeighborQueue.push(DryNeighbor);
            }
        }
    }
}

void RasterPitRemoval::FillToElevation(int PitID, double FillElev)
{
    //Fills all cells within a depression to the specified elevation
    int CurID;

    if ((Terrain.at(PitID) < FillElev) && (Terrain.at(PitID) != dNoDataValue)) Terrain.at(PitID) = FillElev;

    for (size_t i=0; i < Depression.size();i++)
    {
        CurID = Depression.at(i);
        if ((Terrain.at(CurID) < FillElev) && (Terrain.at(CurID) != dNoDataValue))
        {
            Terrain.at(CurID) = FillElev;
        }
    }
}

void RasterPitRemoval::CutToElevation(int PitID)
{
    //This is used both when Mode = CUT_ONLY and by the Hybrid process after Filling has occured
    //Starting at the Pit, Backtrack along flow direction and set each cell equal to pit elevation
    //Stop when either an equal or lower elevation is found, or when an outlet is reached
    PitElev = Terrain.at(PitID);
    bool ReachedOutlet = 0;
    int CurID, NextID;

    CurID = PitID;

    while(!ReachedOutlet)
    {
        NextID = TraceFlow(CurID, (FlowDirection) Direction.at(CurID));
        if(NextID<0) //CurID is a border cell
            ReachedOutlet = 1;
        else if (Terrain.at(NextID) == dNoDataValue) //CurID is next to an internal outlet
            ReachedOutlet = 1;
        else if((Terrain.at(NextID) < PitElev)&&(Flooded.at(NextID)==FLOODEDDESC)) //NextID is lower than Pit and NextID on confirmed descending path to outlet
            ReachedOutlet = 1;
        else
        {
            if ((Terrain.at(NextID) > PitElev) && (Terrain.at(NextID) != dNoDataValue))
                Terrain.at(NextID) = PitElev;
            Flooded.at(NextID) = FLOODEDDESC; //Confirm that cell has descending path to outlet
        }
        CurID = NextID;
    }
}



void RasterPitRemoval::CreateFillFunction(int PitID, double CrestElev)
{
    //Create a list of elevations from the Pit to the Crest, incremented by step size
    //For each elevation in list, sum all positive difference between List elev and Terrain Elev for all cells within the depression
    //FillFunction represents the (Fill Volume / Cell Area)
    int CurID;
    double CurGroundElev;
    double OldFill;
    double CurStep;

    //Initialize
    CurStep = PitElev;
    while (CurStep < CrestElev)
    {
        FillFunction[CurStep] = 0;
        CurStep = CurStep + rInputRaster->GetCellHeight();
    }
    FillFunction[CrestElev] = 0; //Make sure an entry for the Crest is added

    //Calculate Cost
    for (size_t i=0; i < Depression.size();i++)
    {
        CurID = Depression.at(i);
        CurGroundElev = Terrain.at(CurID);

        CurStep = PitElev;
        while (CurStep < CrestElev)
        {
            if (CurStep > CurGroundElev)
            {
                OldFill = FillFunction.find(CurStep)->second;
                FillFunction[CurStep] = OldFill + CurStep - CurGroundElev;
            }

            CurStep = CurStep + rInputRaster->GetCellHeight();

            //Exit Conditions - make sure the last step gets accounted for
            if (CurStep > CrestElev)
            {
                OldFill = FillFunction.find(CrestElev)->second;
                FillFunction[CrestElev] = OldFill + CrestElev - CurGroundElev;
            }
        }
    }
}

void RasterPitRemoval::CreateCutFunction(int PitID, double CrestElev)
{
    //Create a list of elevations from the Pit to the Crest, incremented by step size
    //For each elevation in list, sum all positive difference between Terrain elev and List Elev for all cells along path from pit to outlet
    //CutFunction represents the (Cut Volume / Cell Area)
    bool ReachedOutlet = 0;
    int CurID, NextID;
    double CellElev = Terrain.at(PitID);
    double CurStep;
    double OldCut;
    CurID = PitID;

    //Initialize all values
    CurStep = PitElev;
    while (CurStep < CrestElev)
    {
        CutFunction[CurStep] = 0;
        CurStep = CurStep + rInputRaster->GetCellHeight();
    }
    CutFunction[CrestElev] = 0; //Make sure an entry for the Crest is added, but that cut will always be zero

    //Calculate Cost
    while(!ReachedOutlet)
    {
        NextID = TraceFlow(CurID, (FlowDirection) Direction.at(CurID));
        if(NextID<0) //CurID is a border cell
            ReachedOutlet = 1;
        else if (Terrain.at(NextID) == dNoDataValue) //CurID is next to an internal outlet
            ReachedOutlet = 1;
        else if((Terrain.at(NextID) < PitElev)&&(Flooded.at(NextID)==FLOODEDDESC)) //NextID is lower than Pit and NextID on confirmed descending path to outlet
            ReachedOutlet = 1;
        else
        {
            CellElev = Terrain.at(NextID);
            CurStep = PitElev;
            while (CurStep < CellElev)
            {
                if (CutFunction.count(CurStep)) //check if entry exists
                    OldCut = CutFunction.find(CurStep)->second;
                else
                    OldCut = 0;
                CutFunction[CurStep] = OldCut + CellElev - CurStep;
                CurStep = CurStep + rInputRaster->GetCellHeight();
            }
        }
        CurID = NextID;
    }
}

double RasterPitRemoval::GetIdealFillLevel(double CrestElev)
{
    //If balancing, find point of minimum difference between cut and fill
    //If not balancing, find point of minimum total cost
    //NOTE: CutFunction and FillFunction are currently both defined with only positive values (rather than cut being negative, so the calculations below account for this.

    double FillLevel;
    double CurTotalCost, CurCutCost, CurFillCost, MinCost, CurDifference, MinDifference;
    double CurStep;

    switch (Mode)
    {
    case FILL_CUT:
        FillLevel = dNoDataValue;
        break;
    case FILL_BAL:
    {
        //Initialize
        CurStep = PitElev;
        CurCutCost = CutFunction.find(CurStep)->second;
        CurFillCost = FillFunction.find(CurStep)->second;
        MinDifference = fabs(CurFillCost - CurCutCost);
        FillLevel = CurStep;

        //Optimize
        while (CurStep < CrestElev)
        {
            CurCutCost = CutFunction.find(CurStep)->second;
            CurFillCost = FillFunction.find(CurStep)->second;
            CurDifference = fabs(CurFillCost - CurCutCost);

            if (CurDifference < MinDifference)
            {
                MinDifference = CurDifference;
                FillLevel = CurStep;
            }
            CurStep = CurStep + rInputRaster->GetCellHeight();
        }
    }
        break;
    case FILL_MINCOST:
    {
        //Initialize
        CurStep = PitElev;
        MinCost = CutFunction.find(CurStep)->second;
        //        MinCost = MinCost;
        FillLevel = CurStep;

        if (MinCost > 0)
        {
            //Optimize
            while (CurStep < CrestElev)
            {
                CurCutCost = CutFunction.find(CurStep)->second;
                CurFillCost = FillFunction.find(CurStep)->second;
                CurTotalCost = CurCutCost + CurFillCost;
                if (CurTotalCost < MinCost)
                {
                    MinCost = CurTotalCost;
                    FillLevel = CurStep;
                }
                CurStep = CurStep + rInputRaster->GetCellHeight();
            }
        }
    }
        break;
    }
    return FillLevel;
}

bool RasterPitRemoval::IsBorder(int ID)
{
    //Tests if cell is on the outer edge of the grid, and thus is an outlet
    int numCols = rInputRaster->GetCols();
    if (ID < numCols ||                 // In the top row
            TotalCells - ID < numCols + 1 ||  // In the Last Row
            ! (ID % numCols) ||         // In the Right Col
            ! ((ID+1) % numCols) ){     // In the Left Col
        return true;
    }
    else {
        return false;
    }
}

bool RasterPitRemoval::NeighborNoValue(int ID)
{
    //Tests if cell is next to a cell with no_data, and thus is an outlet
    bool novalue = false;

    GetNeighbors(ID);

    for (int i=0;i<8;i++)
    {
        if (Neighbors.at(i)==-1)
        {
            novalue = true;
            break;
        }
        else if (Terrain.at(Neighbors.at(i)) == dNoDataValue )
        {
            novalue = true;
            SetFlowDirection(ID, Neighbors.at(i));
            break;
        }
    }
    return novalue;
}

void RasterPitRemoval::PitRemoveHybrid(int PitID)
{
    //This handles the cases where Mode = BAL or MIN_COST
    //Cost is defined as the difference between the original terrain and the modified terrain for each point, summed across all modified points.
    //BAL minimizes the sum of cost across all cells modified for each depression (i.e., tries to get Cut Volume = Fill Volume)
    //MIN_COST minimizes the sum of the absolute value of cost across all cells modified for each depression (i.e. tries to have the least disturbance to the original terrain)
    //BAL will *always* have a mix of cut and fill (for suitably small step size)
    //MIN_COST will *sometimes* have a mix of cut and fill, but some pits may be best removed using only cut or only fill
    //Filling modifies a 2-D region (depression). Cutting modifies a 1-D path to an outlet. Thus MIN_COST will often result in more cut than fill.

    double CrestElev, IdealFillLevel;
    PitElev = Terrain.at(PitID);

    //Get Crest Elevation
    CrestElev = GetCrestElevation(PitID);

    //Get Depression Extents and set PitElev
    GetDepressionExtent(PitID, CrestElev);

    //Create Cut Function
    CreateCutFunction(PitID, CrestElev);

    //Create Fill Function
    CreateFillFunction(PitID, CrestElev);

    if(PitElev < CrestElev)
    {
        //Find desired Fill Elevation
        IdealFillLevel = GetIdealFillLevel(CrestElev);

        //Modify Terrain to remove Pit
        FillToElevation(PitID, IdealFillLevel);
        CutToElevation(PitID);
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
        int i;
        for (i=0;i<8;i++)
        {
            if (CheckCell(CurID, i, CurNeighborID, CrestElev))
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



bool RasterPitRemoval::IsLocalMinimum(int CurID)
{
    //A local Minimum has been found if the cell does not have a confirmed path to an outlet, and all neighbors are either higher or the same elevation but flooded.
    //This means for a wide pit with a flat bottom, the last flooded cell (roughly farthest from the outlet) is considered the minimum.

    bool IsMinimum = true;
    if (Flooded.at(CurID)==FLOODEDDESC)  //Cell is on confirmed path to outlet
    {
        IsMinimum = false;
    }
    else //Flooded = 1
    {
        GetNeighbors(CurID);
        int i = 0;
        while ((i<8)&&(IsMinimum==true))
        {
            if (Neighbors.at(i)>-1)
            {
                if( Terrain.at(Neighbors.at(i)) < Terrain.at(CurID))
                    IsMinimum = false;
                if((Terrain.at(Neighbors.at(i)) == Terrain.at(CurID)) && (Flooded.at(Neighbors.at(i)) == UNFLOODED))
                    IsMinimum = false;
            }
            i++;
        }
    }

    if (SavePits == true)
        if (IsMinimum==true) IsPit.at(CurID) = 1;
    return IsMinimum;
}

void RasterPitRemoval::GetNeighbors(int ID)
{
    //Neighbors is a 0-7 vector, defined clockwise from Northwest
    //Returns the ID value for the eight neighbors, with -1 for cells off the grid
    int numCols = rInputRaster->GetCols();
    //Northwest
    if(ID<numCols)
        Neighbors.at(0)=-1;
    else if (!(ID % numCols))
        Neighbors.at(0)=-1;
    else
        Neighbors.at(0)=ID-1-numCols;

    //North
    if(ID<numCols)
        Neighbors.at(1)=-1;
    else
        Neighbors.at(1)=ID-numCols;

    //Northeast
    if(ID<numCols)
        Neighbors.at(2)=-1;
    else if (!((ID+1) % numCols))
        Neighbors.at(2)=-1;
    else
        Neighbors.at(2)=ID+1-numCols;

    //East
    if (!((ID+1) % numCols))
        Neighbors.at(3)=-1;
    else
        Neighbors.at(3)=ID+1;

    //Southeast
    if (TotalCells-ID <numCols + 1)
        Neighbors.at(4)=-1;
    else if (!((ID+1) % numCols))
        Neighbors.at(4)=-1;
    else
        Neighbors.at(4)=ID+1+numCols;

    //South
    if (TotalCells-ID <numCols + 1)
        Neighbors.at(5)=-1;
    else
        Neighbors.at(5)=ID+numCols;

    //Southwest
    if (TotalCells-ID <numCols + 1)
        Neighbors.at(6)=-1;
    else if (!(ID % numCols))
        Neighbors.at(6)=-1;
    else
        Neighbors.at(6)=ID-1+numCols;

    //West
    if (!(ID % numCols))
        Neighbors.at(7)=-1;
    else
        Neighbors.at(7)=ID-1;
}

}
