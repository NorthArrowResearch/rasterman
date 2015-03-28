#ifndef RASTERARRAY_H
#define RASTERARRAY_H

#include "raster.h"
namespace RasterManager{

// All directions follow the same 0-7 vector,
// defined clockwise from Northwest.
//    -------
//    |0|1|2|
//    -------
//    |7|X|3|
//    -------
//    |6|5|4|
//    -------
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
    ENTRYPOINT = -1 // An entrypoint is a pixel beside a nodata value or at the edge of the raster
    // Think of an entry point as UP as well
};

class RasterArray : public Raster
{

public:
    RasterArray(const char * raster);

    // Testing Functions
    bool IsBorder(int ID);

    // Normally we wouldn't put member variables here but
    // It's kind of the only point of this class.
    std::vector<double> Terrain;            // This begins as the input DEM and is modified by the algorithm.
    std::vector<int> Neighbors;     // Stores the ID of the current cell's eight neighbors

    // Neighbour Functions
    size_t GetNeighborID(int id, eDirection dir);

    void GetNeighbors(int ID);

    bool HasValidNeighbor(int ID);
    bool IsDirectionValid(int ID, eDirection dir);

    int GetIDFromCoords(int row, int col);

    // Helper functions for debugging what row/col you are on
    int getRow(int i);
    int getCol(int i);

    inline bool IsTopEdge(int id)   { return id < GetCols(); }
    inline bool IsBottomEdge(int id){ return (GetTotalCells() - id) < (GetCols() + 1); }

    inline bool IsRightEdge(int id) { return ((id+1) % GetCols()) == 0; }
    inline bool IsLeftEdge(int id)  { return (id % GetCols()) == 0; }

    inline size_t GetTotalCells(){ return GetCols() * GetRows(); }

    /**
     * @brief WriteArraytoRaster
     * @param sOutputPath
     * @param vPointArray
     */
    void WriteArraytoRaster(QString sOutputPath, std::vector<double> *vPointArray);
    void WriteArraytoRaster(QString sOutputPath, std::vector<int> *vPointArray);

    // Testing and DEbug Functions
    void TestDir(size_t id);

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

    /**
     * @brief AreaThreshold
     * @param psOutputRaster
     * @param dArea
     */
    int AreaThreshold(const char * psOutputRaster, double dArea);

private:

    bool AreaThresholdWalker(size_t ID,
                             int CurrentFeature,
                             std::vector<bool> * pChecked,
                             QHash<int, double> * pAreaFeatures,
                             std::vector<int> * pAreaMap);
};

}
#endif // RASTERARRAY_H
