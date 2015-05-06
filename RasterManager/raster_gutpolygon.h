#ifndef RASTER_GUTPOLYGON_H
#define RASTER_GUTPOLYGON_H


namespace RasterManager {

class Raster2Polygon
{
public:
    Raster2Polygon();
    static int AddGut(const char *psShpFile, const char *psInput, const char *tier1, const char *tier2);

private:
    static int Initialize(const char *psShpFile);

};


}
#endif // RASTER_GUTPOLYGON_H
