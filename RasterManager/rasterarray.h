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

    DIR_X = 999, // DIR_X is the center point
    INVALID = -1
};

class RasterArray : public Raster
{

public:
    RasterArray(const char * raster);

    // Testing Functions
    bool IsBorder(int ID);

    // Normally we wouldn't put member variables here but
    // It's kind of the only point of this class.
    std::vector<double> Terrain;    // This begins as the input DEM and is modified by the algorithm.
    std::vector<size_t> Neighbors;     // Stores the status of the curent Cell's eight neighbors
    std::vector<bool> Checked;      // Convenience Array used to decide if a cell has been visited.

    // Neighbour Functions
    size_t GetNeighborID(size_t id, eDirection dir);
    double GetNeighborVal(size_t id, eDirection dir);

    void PopulateNeighbors(int ID);

    bool HasValidNeighbor(int ID);
    bool IsDirectionValid(int ID, eDirection dir);

    int GetIDFromCoords(int row, int col);

    // Helper functions for debugging what row/col you are on
    int getRow(int i);
    int getCol(int i);

    inline bool IsTopEdge(size_t id)   { return id < (size_t)GetCols(); }
    inline bool IsBottomEdge(size_t id){ return (GetTotalCells() - id) < ((size_t)GetCols() + 1); }

    inline bool IsRightEdge(size_t id) { return ((id+1) % (size_t)GetCols()) == 0; }
    inline bool IsLeftEdge(size_t id)  { return (id % (size_t)GetCols()) == 0; }

    inline size_t GetTotalCells(){ return (size_t)(GetCols() * GetRows()); }

    /**
     * @brief WriteArraytoRaster
     * @param sOutputPath
     * @param vPointArray
     */
    void WriteArraytoRaster(QString sOutputPath, std::vector<double> *vPointArray);
    void WriteArraytoRaster(QString sOutputPath, std::vector<int> *vPointArray);
    void WriteArraytoRaster(QString sOutputPath, std::vector<size_t> *vPointArray);
    void WriteArraytoRaster(QString sOutputPath, std::vector<bool> *vPointArray);

    // Testing and DEbug Functions
    void TestNeighbourID(size_t id);
    void TestNeighbourVal(size_t id);

    // ====== CLASSES AND STRUCTURES ================================
    struct point
    {
      //A point contains a raster cell location in 1-D array format (0-based, starting from upper left corner) and the associated elevation
      size_t id; //0-based grid cell number in a 1-D array
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

    size_t invalidID;

    bool AreaThresholdWalker(int ID,
                             int *CurrentFeatureID,
                             int *pdCellsInArea,
                             std::vector<int> *pAreaMap);
};

}
#endif // RASTERARRAY_H
