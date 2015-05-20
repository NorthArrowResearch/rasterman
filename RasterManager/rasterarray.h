#ifndef RASTERARRAY_H
#define RASTERARRAY_H

#include "raster.h"
#include <queue>

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

class RM_DLL_API RasterArray : public Raster
{

public:
    RasterArray(const char * raster);

    bool operator ==(RasterArray &src);
    bool operator !=(RasterArray &src);

    // Testing Functions
    bool IsBorder(size_t ID);

    // Normally we wouldn't put member variables here but
    // It's kind of the only point of this class.
    std::vector<double> Terrain;    // This begins as the input DEM and is modified by the algorithm.
    std::vector<size_t> Neighbors;     // Stores the ID of the curent Cell's eight neighbors


    /**
     * @brief TestChecked: Compare all cells in a raster
     * @param id
     */
    void TestChecked(size_t id);

    // Neighbour Functions
    size_t GetNeighborID(size_t id, eDirection dir);
    double GetNeighborVal(size_t ID, eDirection dir);

    void PopulateNeighbors(int ID);

    bool HasValidNeighbor(size_t ID);
    bool IsDirectionValid(size_t ID, eDirection dir);

    int GetIDFromCoords(int row, int col);

    inline double GetCell(size_t ID){ return Terrain.at(ID); }


    // We don't allow access to the checked array directly
    inline void SetChecked(size_t ID){ if (ID < Checked.size()) Checked.at(ID) = 1; }
    inline void UnSetChecked(size_t ID){ if (ID < Checked.size()) Checked.at(ID) = 0; }
    inline bool IsChecked(size_t ID){ return (ID < Checked.size() && Checked.at(ID) == 1); }
    inline void ResetChecked(){ std::fill(Checked.begin(), Checked.end(), 0); }

    // Helper functions for debugging what row/col you are on
    size_t getRow(size_t i);
    size_t getCol(size_t i);

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
    void WriteArraytoRaster(QString sOutputPath, std::vector<double> *vPointArray, GDALDataType * dataType);
    void WriteArraytoRaster(QString sOutputPath, std::vector<int> *vPointArray, GDALDataType * dataType);
    void WriteArraytoRaster(QString sOutputPath, std::vector<size_t> *vPointArray, GDALDataType * dataType);

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
     * @brief AreaRaster -- Write a raster with areas for each different value
     * @param psOutputRaster
     * @return
     */
    int AreaRaster(const char * psOutputRaster);

    /**
     * @brief AreaThreshold
     * @param psOutputRaster
     * @param dArea
     */
    int AreaThresholdRaster(const char * psOutputRaster, double dArea);

    /**
     * @brief SmoothEdge
     * @param psInputRaster
     * @param psOutput
     * @param nCells
     * @return
     */
    int SmoothEdge(const char *psOutputRaster, int nCells);

    /**
     * @brief reCurseDebug
     * @param ID
     * @param row
     * @param col
     */
    void reCurseDebug(size_t ID, size_t row, size_t col);


    /**
     * @brief CellCompare
     * @param raArray2
     * @return
     */
    bool CellCompare(RasterArray *raArray2);

    void GetFeatures();

    int CreateDrain(const char *psOutputRaster);

protected:
    // Instead of recursive algorithms which can cuase Stack Overflows we use a priority Queue
    // That we fill and empty
    std::priority_queue<RasterArray::point, std::vector<RasterArray::point>, RasterArray::ComparePoint> MainQueue;

private:

    size_t invalidID;
    std::vector<int> Checked;      // Convenience Array used to decide if a cell has been visited.

    /**
     * @brief FindAreas -- This is a wrapper to kick-start the AreaWalker (below)
     * @param AreaMap
     * @param AreaFeatures
     * @return
     */
    int FindAreas(std::vector<int> *AreaMap, QHash<int, double> *AreaFeatures, bool bValDelim);

    /**
     * @brief RasterArray::AreaWalker -- Queue-based algorithm to find the size of each area.
     *                                      In the process we find area IDs too.
     * @param ID
     * @param CurrentFeatureID
     * @param pdCellsInArea
     * @param pAreaMap
     * @param bMask -- False: Every different value is a new area, True: Areas are surrounded by nodatavalue
     * @return
     *
     *
     * https://en.wikipedia.org/wiki/Flood_fill
     *
     */
    bool AreaWalker(size_t ID,
                    int *CurrentFeatureID,
                    int *pdCellsInArea,
                    std::vector<int> *pAreaMap, bool bValDelim);
};

}
#endif // RASTERARRAY_H
