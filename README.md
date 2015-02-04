# RasterManager

Raster manager is a C++ tool created by [North Arrow Research](http://northarrowresearch.com). It is designed to provide rock-solid operations for manipulating rasters.

It does not depend on any of our other git repositories. GDAL 1.10 is required

## Usage

```
 Rasterman
 Usage: rasterman <command> [parameters...]

 Commands (type rasterman followed by the command to retrieve parameter information):

    raster          Display basic properties (rows, cols etc) for a raster.
    bilinear        Bilinear resample of a raster to produce a new raster.
    copy            Copy a raster to produce a new raster with the specified extent.
    mosaic          Stitch two or more overlappint rasters.
    makeconcurrent  Make all input rasters concurrent.
    mask            Mask one raster using another raster or a vector.
    setnull         Set a NoDataValue in a raster based on thesholding.

    add          Add two rasters or a raster and a constant.
    subtract     Subtract two rasters or a constant from a raster.
    divide       Divide a raster by a number or another raster.
    multiply     Multiply a raster by a number or another raster.
    power        Raise a raster to a power.
    sqrt         Get the square root of a raster.

    hillshade       Create a hillshade raster.
    slope           Create a slope raster.
    png             Create a PNG image copy of a raster.

    csv2raster      Create a raster from a .csv file
    vector2raster   Create a raster from a vector file.

```

## Commands

All commands have in-depth help. Just type `rastermanager <command>`

* `raster` Display basic properties (rows, cols etc) for a raster.
* `bilinear` Bilinear resample of a raster to produce a new raster.
* `copy` Copy a raster to produce a new raster with the specified extent.
* `mosaic` Stitch two or more overlappint rasters.
* `mask` Mask one raster using another.

* `hillshade` Create a hillshade raster
* `slope` Create a slope raster

* `add` Add two rasters or a raster and a constant.
* `subtract` Subtract two rasters or a constant from a raster.
* `divide` Divide a raster by a number or another raster.
* `multiply` Multiply a raster by a number or another raster.
* `power` Raise a raster to a power.
* `sqrt` Get the square root of a raster.
* `csv2raster` convert a .csv file into a .tiff

## Developer Notes:

### Compatibility

Currently Rastermanager builds on Win32, Win64, Ubuntu12.04 and OSX 10.10.

### Suggested Folder structure:

In order to build this project alongside our [Habitat Model](https://bitbucket.org/northarrowresearch/habitat-model-console) tools you will need the following directory structure so that everything else can find rastermanager:

```
<PROJECTROOT>/
<PROJECTROOT>/RasterManager/rastermanager/   <-- This repo's root
<PROJECTROOT>/HabitatModel/habitatmodel      <-- CHaMP Habitat model repo root
<PROJECTROOT>/Deploy/Debug(32/64)            <-- Debug executable for all projects
<PROJECTROOT>/Deploy/Release(32/64)          <-- Release executables for all projects
```

## Compiling:

Once Qt5 and GDaL are installed it should just be a matter of running the following

```
qmake -r RasterManager.pro
make
sudo make install
```

Now you should have `rasterman` in your `/usr/bin` an the `libRasterManager.*` files in `/usr/lib`

## Contributing

1. Fork it!
2. Create your feature branch: `git checkout -b my-new-feature`
3. Commit your changes: `git commit -am 'Add some feature'`
4. Push to the branch: `git push origin my-new-feature`
5. Submit a pull request :D

## Credits

### Developers

* [Philip Bailey](https://github.com/philipbaileynar)
* [Matt Reimer](https://github.com/MattReimer)

### Contributors

* [Konrad Hafen](https://sites.google.com/site/konradhafengis/)

## License

[GPL V3](LICENSE.txt)