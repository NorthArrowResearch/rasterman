# RasterManager

Raster manager is a tool created by North Arrow Research. It is designed to provide rock-solid operations for manipulating rasters.

It does not depend on any of our other git repositories. GDAL 1.11 is required

## Commands

All commands have in-depth help. Just type `rastermanager <command>`

* `raster` Display basic properties (rows, cols etc) for a raster.
* `bilinear` Bilinear resample of a raster to produce a new raster.
* `copy` Copy a raster to produce a new raster with the specified extent.
* `mosaic` Stitch two or more overlappint rasters.
* `mask` Mask one raster using another.
* `add` Add two rasters or a raster and a constant.
* `subtract` Subtract two rasters or a constant from a raster.
* `divide` Divide a raster by a number or another raster.
* `multiply` Multiply a raster by a number or another raster.
* `power` Raise a raster to a power.
* `sqrt` Get the square root of a raster.

## Developer Notes:

### Compatibility

Currently Rastermanager works on Win32, Win64 and OSX 10.10.

### Folder structure:

In order to build this project alongside our [Habitat Model](https://bitbucket.org/northarrowresearch/habitat-model-console) and [GCD](https://bitbucket.org/northarrowresearch/gcd-console) tools you will need the following directory structure so that everything else can find rastermanager

```
<PROJECTROOT>/
<PROJECTROOT>/RasterManager/rastermanager/   <-- This repo's root
<PROJECTROOT>/GCD/gcd-console/               <-- The GCD console repo root
<PROJECTROOT>/HabitatModel/habitatmodel      <-- CHaMP Habitat model repo root
<PROJECTROOT>/Deploy/Debug(32/64)            <-- Debug executable for all projects
<PROJECTROOT>/Deploy/Release(32/64)          <-- Release executables for all projects
```