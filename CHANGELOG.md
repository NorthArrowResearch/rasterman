--------------------------------

## 6.1.1

### New Features

* Hillshade
* Slope

### Fixes

* Fixed a bug with how the GDAL driver is found for raster inputs.

--------------------------------

## 6.1.0

### New Features

* RasterManager is now its own repo, separate from GCD.
* Basic Math functions: add, subtract, divide, multiply, power and sqrt both with other rasters and decimal number constants
* Mosaic: stitches together multiple rasters
* Mask: Use one raster to mask out another.
* Project now compiles on OSX Yosemite which opens up the future for *nix compatibility as well as other compilers like clang, gcc etc.
* Compiled successfully on 64-bit windows.


### Fixes

* Addressed a bug with bilinear resample where edge values would be the average of the neighbouring value and the smallest possible `<float>`.
* Versioned DLLs now part of the `RasterManager.pro` file.
* Lots of fixes to how this all compiles please read [README.md](./README.md) for instructions on setting things up.
* Cleaning up the command line outputs and adding a summary after every command, just to be helpful.
* Large refactoring of code:
    * Removing type checking and duplicated functions since GDAL does that for us. 
    * Raster Objects now descend from `RasterMeta` and `ExtentRectangles` objects.

--------------------------------