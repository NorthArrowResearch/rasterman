#ifndef RASTER_PITREMOVE_H
#define RASTER_PITREMOVE_H

#endif // RASTER_FILL_H

#include "raster.h"
#include "rastermanager_exception.h"
#include "rastermanager_global.h"
#include <queue>
#include "benchmark.h"


namespace RasterManager {


class RM_DLL_API RasterPitRemoval {

public:

    RasterPitRemoval(const char * sRasterInput,
                     const char * sRasterOutput,
                     FillMode eMethod);
    ~RasterPitRemoval();

    int Run();

private:

    // All directions follow the same 0-7 vector,
    // defined clockwise from Northwest.
    //    -------
    //    |0|1|2|
    //    -------
    //    |7|X|3|
    //    -------
    //    |6|5|4|
    //    -------

    // DEBUG Variables
    LoopTimer * QueueItem;

    // Value of UNFLOODED; Value of FLOODED; Value of FLOODEDDESC and has confirmed descending path to an outlet
    enum eDirection {
        DIR_NW = 0,
        DIR_N = 1,
        DIR_NE = 2,
        DIR_E = 3,
        DIR_SE = 4,
        DIR_S = 5,
        DIR_SW = 6,
        DIR_W = 7,

        INIT = 999,
        NONEIGHBOUR = -1
    };

    enum FloodedState { UNFLOODED, FLOODED, FLOODEDDESC };

    /**
     * @brief WriteArraytoRaster
     * @param sOutputPath
     * @param vPointArray
     */
    void WriteArraytoRaster(QString sOutputPath, std::vector<double> *vPointArray);
    void WriteArraytoRaster(QString sOutputPath, std::vector<int> *vPointArray);

    // Main work function
    void InitializeMainQueue();

    // Queue Functions
    void IterateMainQueue();
    void AddToMainQueue(int ID, bool ConfirmDescend);

    // Utility Functions
    void SetFlowDirection(int FromID, int ToID);
    int TraceFlow(int FromID, int Direction);
    void FillToElevation(int PitID, double FillElev);
    void CutToElevation(int PitID);
    int TraceFlow(int FromID, eDirection eFlowDir);
    void CreateFillFunction(int PitID, double CrestElev);

    double GetIdealFillLevel(double CrestElev);
    double GetCrestElevation(int PitID);

    // Testing Functions
    bool CheckCell(int ID, int Direction, int &CurNeighborID, double CrestElev);
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

    std::vector<double> Terrain;            // This begins as the input DEM and is modified by the algorithm.
    std::vector<double> debugVector;        // Something to fill up and test the output
    std::vector<eDirection> Direction;   // 8-direction indicator of which cell caused the current cell to become flooded
    std::vector<FloodedState> Flooded;      // Value of 0=unflooded; Value of 1=flooded; Value of 2=flooded and has confirmed descending path to an outlet
    std::vector<bool> Checked;      // Used to determine the extent of a depression. Reset after each depression is identified
    std::vector<bool> BlankBool;    // Used for clearing the contents of a vector-bool of Terrain size
    std::vector<int> Depression;    // Stores the extent of
    std::vector<int> BlankInt;      // Used for clearing the contents of a vector-int  of Zero size
    std::vector<int> Neighbors;     // Stores the ID of the current cell's eight neighbors
    std::vector<double> IsPit;      // Optional binary record of which cells were identified as local minima.

    std::map<double, double> CutFunction;   // Stores paired elevation/cost data
    std::map<double, double> FillFunction;  // Stores paired elevation/cost data
    std::map<double, double> BlankMap;      // Used to reset Cut and Fill Functions

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
        inline bool operator()(point& p1, point& p2) // Returns true if p1 is greater than p2. This makes the priority queue return low elevations first
        {
           if (p1.elev > p2.elev) return true;
           return false;
        }
    };

    std::priority_queue<point, std::vector<point>, ComparePoint> MainQueue;
    std::priority_queue<point, std::vector<point>, ComparePoint> NeighborQueue;
    std::priority_queue<point, std::vector<point>, ComparePoint> DepressionQueue;

    // Helper functions for debugging what row/col you are on
    int getRow(int i);
    int getCol(int i);

};


}
