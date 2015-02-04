#Raster to PNG
Raster to PNG is a set of C++ classes that leverage the Geospatial Data Abstraction Library (GDAL) to convert raster data sets to PNG images for display in reports. These classes can reproduce symbology from ESRI products and the Geomorphic Change Detection (GCD) software. Raster to PNG also supports legend printing (as a separate PNG image).

##1. Creating a PNG Image from a Raster
`Renderer` is the base class for class for all classes. Below are examples for how to use Raster to PNG classes for raster conversion with different symbology types. For the examples below we will use the following variables as input and output paths. The rendrers for stretched and classified symbology support centering the color ramp on zero and displaying zero values as NoData. In addition, multiple color ramp options are available. If you are symbolizing a raster with byte data (values from 0-255; e.g. a hillshade raster) be sure to use the methods described in the special cases section.

        const char *inputRaster = "C:/Test/test.tif";
        const char *outputPNG = "C:/Test/test.png";
        const char *outputPNG2 = "C:/Test/test2.png";
 
###Stretched Symbology
Raster to PNG supports two types of stretched symbology; Minimum - Maximum and Standard Deviation. The example below shows how to create a renderer for Min-Max symbology and Standard Deviation symbology with a Standard Deviation stretch of 2.5.

        //create the renderer
        //input raster path, color ramp style, transparency (0-255), center ramp on zero, symbolize zero as NoData
        Renderer_StretchMinMax Renderer1 = new Renderer_StretchMinMax(inputRaster, CR_BlackWhite, 255, FALSE, FALSE);

        //input raster path, std dev stretch, . . . 
        Renderer_StretchStdDev Renderer2 = new Renderer_StretchStdDev(inputRaster, 2.5, CR_BlackWhite, 255, FALSE, FALSE);

        //output PNG path, quality (0-100), size of output PNG (no. of pixels on longest side)
        Renderer1.rasterToPNG(outputPNG, 100, 1000);
        Renderer2.rasterToPNG(outputPNG2, 100, 1000);
   
###Classified Symbology
The renderer for classified symbology has the same parameters as the stretched symbology renderers, except it has an `integer` parameter for the number of classes instead of a `double` parameter for the stretch function. The code below creates a classified PNG with 20 classes. Each classification is done with an equal interval method.

        //create the renderer
        //input raster path, number of classes, color ramp style, transparency, center ramp on zero, symbolize zero as NoData
        Renderer_Classified Renderer1 = new Renderer_Classified(inputRaster, 20, CR_BlackWhite, 255, FALSE, FALSE);

        //print to PNG
        Renderer1.rasterToPNG(outputPNG1, 100, 1000);

###Special Cases (Byte data and GCD)
When converting a raster that contains byte data (e.g. hillshade raster) be sure to use the `Renderer_ByteData` class. This class will copy the values directly to the PNG instead of classifying or applying a stretch to the values during conversion to a PNG. Raster to PNG displays the values 0 and 255 as NoData, because ESRI uses 255 as NoData for byte rasters and ET-AL functions use 0 as NoData for byte rasters. IMPORTANT: the values in the raster must range from 0 - 255 for this to work properly.

There are special sub classes of `Renderer_Classified` for GCD symbology. These classes are named `Renderer_GCD` followed by the raster type (i.e. `PtDens`, `SlopeDeg`, `Error`, etc.). These classes generally take two parameters, the input raster path and transparency, the other parameters are set as defaults to ensure consistency with GCD layers produced from ESRI products.
##2. Creating a Legend
It is extremely easy to print a legend from a raster layer. After instantiating a `Renderer` object simply call the `printLegend()` or `printLegend(const char *path)` method. The `printLegend()` method prints a legend to with the name of the output PNG plus "legend". For example, if I called `printLegend()` with Renderer1 from the above example the legend would be printed to the path `C:/Test/test_legend.png`. You may also specify a specific legend path using the `printLegend(const char *path)` method. The `printLegend` method calls `setPrecision()` to determine the decimal precesion of the legend labels. The precision can be changed by calling `setPrecision(int prec)` before `printLegend` is called to set a custom decimal precision for a legend. The `setPrecision` method does not change class break or interval values, just the labels that are displayed.

##3. Creating a Layered PNG
PNG images can be stacked to create a single PNG image with multiple layers. The base class `Renderer` has a static member `stackImages(const char *inputList, const char *outputImage, int nQuality)` that is used to accomplish this. The input is a `;` delimited list of file paths. The image listed first in the list will appear at the bottom of the stacked image, the second on top of the first, and so on. If I wanted to stack the two PNGs created in the stretched symbology example I would use the following code.

        const char *inputPath = "C:/Test/test.png;test2.png";
        const char *outputPath = "C:/Test/stackout.png";
        Renderer::stackImages(inputPath, outputPath, 100);
