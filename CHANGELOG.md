--------------------------------

## 6.1.6

This release is in preparation for our GUT tool and contains a lot of new features for ways to process raster files.

### New Features

* RasterMan Now has an Icon! Added source in SVG format
* **New Operation**: `fill`: Pit Removal
* **New Operation**: `dist`: Euclidean distance to the nearest non-zero, non-noData value.  
* **New Operation**: `normalize`: Normalize the raster
* **New Operation**: `invert`: Turn all nodata points into a value.
* **New Operation**: `raster2csv`: complement to csv2raster
* **New Operation**: `extractpoints`: get a csv full of values extracted.
* **New Operation**: `filter`: perform operations like "smooth" over a moving window of arbitrary size (less than 16 cells).
* Added a C interface for csv2raster and raster2csv

### Fixes

* Fixed a bug to do with whitespace inside CSV cell quotes
* Completely refactored `csv2raster` to work faster.

--------------------------------

## 6.1.5

This release is a big refactor of Rasterman. There are several new features and loads of fixes.

### New Features

* "C" interface has changed extensively. This will affect all our C# tools that consume this.
* Linux building is now much easier.
* Vector to raster is added but is still temperamental.

### Fixes

* Better exception handling
* Better return codes.
* Lots of memory optimization fixes.
* Fixes to the way projections and set and retrieved.
* Fixed a problem with NoDataVals on CSV inputs.

--------------------------------

## 6.1.4

This was an intermediate version that we didn't ship. The main features of this version were a fix to an "off-by-one" error in csvtoraster

--------------------------------

## 6.1.3

### New Features

* png raster -> png support. 
* Operations which create rasters now set max, min before closing the file.
* New Functions: `IsConcurrent` and `MakeConcurrent`
* More correct POSIX error reporting on the command line.

### Fixes

* Moved basic math enumerations. This should be noted for any future GCD-addon release.
* More error codes added.
* A few more memory leaks caught and squashed. Dr. Memory now offers a clean bill of health.

### Notes

This release was mainly aimed at Ubuntu 12.04 64-bit and done mainly so that we could include a raster-to-png converter.

**Note:** In addition to the list of dependencies (in windows) we've added `Qt5Gui.dll`

The complete list of dependencies is as follows:

* `gdal110.dll`, 
* `icudt52.dll`, 
* `icuin52.dll`
* `icuuc52.dll`
* `Qt5Core.dll`
* `Qt5Gui.dll`
* `Qt5Xml.dll`

--------------------------------

## 6.1.2

### Fixes

* A number of memory errors were identified and crushed. These fixes offer a vast improvement to the stability of our tool set.
* A number of cleanups and standardizations (also perfectly cromulent verbifications) were added to make the command line a nicer place to play.

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