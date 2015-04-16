#ifndef RASTER_PITREMOVE_H
#define RASTER_PITREMOVE_H

#include "raster.h"
#include "rastermanager_exception.h"
#include "rastermanager_global.h"
#include "rasterarray.h"
#include <queue>
#include "benchmark.h"

namespace RasterManager {

class RM_DLL_API RasterPitRemoval : public RasterArray {

public:

    RasterPitRemoval(const char *sRasterInput,
                     const char * sRasterOutput,
                     FillMode eMethod);
    ~RasterPitRemoval();

    int Run();

private:

    // DEBUG Variables
    LoopTimer * QueueItem;

    // Value of UNFLOODED; Value of FLOODED; Value of FLOODEDDESC and has confirmed descending path to an outlet
    // A Flood Source is a pixel beside a nodata value or at the edge of the raster
    enum FloodedState {
        INIT = -999,
        FLOODSOURCE = -10,
        UNFLOODED,
        FLOODED,
        FLOODEDDESC
    };

    void debugFunc();

    // Main work function
    void InitializeMainQueue();

    // Queue Functions
    void IterateMainQueue();
    void AddToMainQueue(int ID, bool ConfirmDescend);

    // Utility Functions
    void SetFlowDirection(int FromID, int ToID);
    void FillToElevation(int PitID, double FillElev);
    void CutToElevation(int PitID);
    int TraceFlow(int FromID, int eFlowDir);
    void CreateFillFunction(int PitID, double CrestElev);

    bool CheckCell(int ID, int CurNeighborID, double CrestElev);

    bool NeighborNoValue(int ID);

    void GetDryNeighbors(int ID);

    double GetIdealFillLevel(double CrestElev);
    double GetCrestElevation(int PitID);

    void PitRemoveHybrid(int PitID);

    bool IsLocalMinimum(int CurID);

    void GetDepressionExtent(int PitID, double CrestElev);

    bool SavePits; // Input (ignored for now)

    FillMode Mode; // Input. See enum in rastermanager_interface.h

    QString sOutputPath;        // Path to output raster

    double dNoDataValue;        // The nodata value we will use throughout
    double PitElev;

    std::vector<FloodedState> Flooded;      // Value of 0=unflooded; Value of 1=flooded; Value of 2=flooded and has confirmed descending path to an outlet
    std::vector<bool> BlankBool;    // Used for clearing the contents of a vector-bool of Terrain size
    std::vector<int> Depression;    // Stores the extent of
    std::vector<int> BlankInt;      // Used for clearing the contents of a vector-int  of Zero size
    std::vector<double> IsPit;      // Optional binary record of which cells were identified as local minima.
    std::vector<int> FloodDirection;   // 8-direction indicator of which cell caused the current cell to become flooded

    std::map<double, double> CutFunction;   // Stores paired elevation/cost data
    std::map<double, double> FillFunction;  // Stores paired elevation/cost data
    std::map<double, double> BlankMap;      // Used to reset Cut and Fill Functions

    std::priority_queue<RasterArray::point, std::vector<RasterArray::point>, RasterArray::ComparePoint> NeighborQueue;
    std::priority_queue<RasterArray::point, std::vector<RasterArray::point>, RasterArray::ComparePoint> DepressionQueue;

};


}

#endif // RASTER_FILL_H
