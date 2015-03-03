#ifndef RASTER_FILL_H
#define RASTER_FILL_H

#endif // RASTER_FILL_H

#include <raster.h>
#include <rastermanager_exception.h>
#include <rastermanager_global.h>
#include <queue>

namespace RasterManager {



// ====== CLASSES AND STRUCTURES ================================
struct point
{
  //A point contains a raster cell location in 1-D array format (0-based, starting from upper left corner) and the associated elevation
  int id; //0-based grid cell number in a 1-D array
  double elev; //Elevation associated with the point
};
class ComparePoint
{
  public:
    bool operator()(point& p1, point& p2) // Returns true if p1 is greater than p2. This makes the priority queue return low elevations first
    {
       if (p1.elev > p2.elev) return true;
       return false;
    }
};
class CompareElevation
{
  public:
    bool operator()(double e1, double e2) // Returns true if p1 is greater than p2. This makes the priority queue return low elevations first
    {
       if (e1 > e2) return true;
       return false;
    }
};



class RasterPitRemoval {

public:

    RasterPitRemoval(const char * sRasterInput,
                     const char * sRasterOutput,
                     FillMode eMethod);
    ~RasterPitRemoval();

    int Run();
private:

    enum FlowDirection { DIR_E, DIR_SE, DIR_S, DIR_SW, DIR_W, DIR_NW, DIR_N, DIR_NE };

    // Main functions
    void InitializeMainQueue();
    void IterateMainQueue();

    // Queue Functions
    void AddToMainQueue(int ID, bool ConfirmDescend);

    double GetCrestElevation(int PitID);
    void SetFlowDirection(int FromID, int ToID);
    int TraceFlow(int FromID, int FlowDirection);
    void FillToElevation(int PitID, double FillElev);
    void CutToElevation(int PitID);
    bool CheckCell(int ID, int Direction, int &CurNeighborID, double CrestElev);
    int TraceFlow(int FromID, FlowDirection eFlowDir);
    void CreateFillFunction(int PitID, double CrestElev);
    double GetIdealFillLevel(double CrestElev);

    bool IsLocalMinimum(int CurID);
    bool IsBorder(int ID);

    // Neighbour Functions
    void GetNeighbors(int ID);
    void GetDryNeighbors(int ID);
    bool NeighborNoValue(int ID);

    void PitRemoveHybrid(int PitID);

    bool SavePits; // Input (ignored for now)

    FillMode Mode; // Input. See enum in rastermanager_interface.h


    void CreateCutFunction(int PitID, double CrestElev);
    void GetDepressionExtent(int PitID, double CrestElev);

    Raster * rInputRaster;      // The object that contains all our raster properties
    QString sOutputPath;        // Path to output raster
    QString sInputPath;         // Path to Input raster

    double dNoDataValue;        // The nodata value we will use throughout
    double PitElev;
    int TotalCells;             // Number of elements in the array

    std::vector<double> Terrain; //This begins as the input DEM and is modified by the algorithm.
    std::vector<int> Direction; //8-direction indicator of which cell caused the current cell to become flooded
    std::vector<int> Flooded; //Value of 0=unflooded; Value of 1=flooded; Value of 2=flooded and has confirmed descending path to an outlet
    std::vector<bool> Checked; //Used to determine the extent of a depression. Reset after each depression is identified
    std::vector<bool> BlankBool;  //Used for clearing the contents of a vector-bool of Terrain size
    std::vector<int> Depression; //Stores the extent of
    std::vector<int> BlankInt; //Used for clearing the contents of a vector-int  of Zero size
    std::vector<int> Neighbors; //Stores the ID of the current cell's eight neighbors
    std::vector<double> IsPit; //Optional binary record of which cells were identified as local minima.

    std::map<double, double> CutFunction; //Stores paired elevation/cost data
    std::map<double, double> FillFunction; //Stores paired elevation/cost data
    std::map<double, double> BlankMap; //Used to reset Cut and Fill Functions

    std::priority_queue<point, std::vector<point>, ComparePoint> MainQueue;
    std::priority_queue<point, std::vector<point>, ComparePoint> NeighborQueue;
    std::priority_queue<point, std::vector<point>, ComparePoint> DepressionQueue;

};

}
