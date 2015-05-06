#ifndef RASTER_GUTPOLYGON_H
#define RASTER_GUTPOLYGON_H


namespace RasterManager {

class Raster2Polygon
{
public:
    Raster2Polygon();
    int Initialize(const char *psShpFile);
    int AddGut(const char *psShpFile, const char *psInput, const char *psName);
};


}
#endif // RASTER_GUTPOLYGON_H
