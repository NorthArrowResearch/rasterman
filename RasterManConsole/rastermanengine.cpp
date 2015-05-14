#include "rastermanengine.h"

#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <cstring>
#include <QDebug>
#include "raster.h"
#include "rastermanager.h"
#include "rastermanager_interface.h"
#include "rastermanager_exception.h"
#include "raster_pitremove.h"
#include "raster_gutpolygon.h"

namespace RasterManager {

RasterManEngine::RasterManEngine()
{
    CheckRasterManVersion();
}

void RasterManEngine::CheckRasterManVersion(){

    QString sVersion = QString(EXEVERSION);
    QString sLibVersion = QString(RasterManager::GetLibVersion());

    if (sVersion.compare(sLibVersion, Qt::CaseInsensitive) != 0){
        QString sErr = QString("Exe is at: %1, Library is at %2").arg(sVersion).arg(sLibVersion);
        throw RasterManager::RasterManagerException(RASTERMAN_VERSION, sErr);
    }
}

int RasterManEngine::Run(int argc, char * argv[])
{

    bool bRecognizedCommand = true;

    if (argc > 1)
    {
        int eResult = PROCESS_OK;

        // For Debug only! ----------------------------------
        QString sProcname = "Unknown";
        if (argc >= 2)
            sProcname = QString(argv[1]);
        RasterManager::ProcessTimer benchProcess(sProcname);
        // ---------------------------------------------------

        RasterManager::RegisterGDAL();
        QString sCommand(argv[1]);

        if (QString::compare(sCommand, "Raster", Qt::CaseInsensitive) == 0)
            eResult = RasterProperties(argc, argv);

        else if (QString::compare(sCommand, "BiLinear", Qt::CaseInsensitive) == 0)
            eResult = BiLinearResample(argc, argv);

        else if (QString::compare(sCommand, "Copy", Qt::CaseInsensitive) == 0)
            eResult = RasterCopy(argc, argv);

        else if (QString::compare(sCommand, "math", Qt::CaseInsensitive) == 0)
            eResult = RasterMath(argc, argv);

        else if (QString::compare(sCommand, "csv2raster", Qt::CaseInsensitive) == 0)
            eResult = CSVToRaster(argc, argv);

        else if (QString::compare(sCommand, "raster2csv", Qt::CaseInsensitive) == 0)
            eResult = RasterToCSV(argc, argv);

        else if (QString::compare(sCommand, "vector2raster", Qt::CaseInsensitive) == 0)
            eResult = VectorToRaster(argc, argv);

        else if (QString::compare(sCommand, "Slope", Qt::CaseInsensitive) == 0)
            eResult = Slope(argc, argv);

        else if (QString::compare(sCommand, "Hillshade", Qt::CaseInsensitive) == 0)
            eResult = Hillshade(argc, argv);

        else if (QString::compare(sCommand, "Mosaic", Qt::CaseInsensitive) == 0)
            eResult = Mosaic(argc, argv);

        else if (QString::compare(sCommand, "MakeConcurrent", Qt::CaseInsensitive) == 0)
            eResult = MakeConcurrent(argc, argv);

        else if (QString::compare(sCommand, "Mask", Qt::CaseInsensitive) == 0)
            eResult = Mask(argc, argv);

        else if (QString::compare(sCommand, "MaskVal", Qt::CaseInsensitive) == 0)
            eResult = MaskVal(argc, argv);

        else if (QString::compare(sCommand, "PNG", Qt::CaseInsensitive) == 0)
            eResult = PNG(argc, argv);

        else if (QString::compare(sCommand, "invert", Qt::CaseInsensitive) == 0)
            eResult = invert(argc, argv);
        else if (QString::compare(sCommand, "extractpoints", Qt::CaseInsensitive) == 0)
            eResult = extractpoints(argc, argv);
        else if (QString::compare(sCommand, "filter", Qt::CaseInsensitive) == 0)
            eResult = filter(argc, argv);
        else if (QString::compare(sCommand, "normalize", Qt::CaseInsensitive) == 0)
            eResult = normalize(argc, argv);
        else if (QString::compare(sCommand, "fill", Qt::CaseInsensitive) == 0)
            eResult = fill(argc, argv);
        else if (QString::compare(sCommand, "dist", Qt::CaseInsensitive) == 0)
            eResult = dist(argc, argv);
        else if (QString::compare(sCommand, "linthresh", Qt::CaseInsensitive) == 0)
            eResult = LinThresh(argc, argv);

        else if (QString::compare(sCommand, "setnull", Qt::CaseInsensitive) == 0)
            eResult = SetNull(argc, argv);

        else if (QString::compare(sCommand, "areathresh", Qt::CaseInsensitive) == 0)
            eResult = AreaThresh(argc, argv);

        else if (QString::compare(sCommand, "smoothedges", Qt::CaseInsensitive) == 0)
            eResult = SmoothEdges(argc, argv);

        else if (QString::compare(sCommand, "combine", Qt::CaseInsensitive) == 0)
            eResult = Combine(argc, argv);

        else if (QString::compare(sCommand, "stats", Qt::CaseInsensitive) == 0)
            eResult = Stats(argc, argv);

        else if (QString::compare(sCommand, "stackstats", Qt::CaseInsensitive) == 0)
            eResult = StackStats(argc, argv);

        else if (QString::compare(sCommand, "compare", Qt::CaseInsensitive) == 0)
            eResult = Compare(argc, argv);

        else if (QString::compare(sCommand, "uniform", Qt::CaseInsensitive) == 0)
            eResult = Uniform(argc, argv);

        else if (QString::compare(sCommand, "compare", Qt::CaseInsensitive) == 0)
            eResult = Compare(argc, argv);

        else if (QString::compare(sCommand, "gutpoly", Qt::CaseInsensitive) == 0)
            eResult = GutPoly(argc, argv);

        else
            bRecognizedCommand = false;

        RasterManager::DestroyGDAL();

        // For Debug only! ----------------------------------
        benchProcess.Output();
        // -------------------------------------------------

        return eResult;
    }
    else
        bRecognizedCommand = false;

    if (!bRecognizedCommand)
    {
        std::cout << "\n Rasterman  v" << EXEVERSION;
        std::cout << "\n Usage: rasterman <command> [paramters...]\n";

        std::cout << "\n Commands (type rasterman followed by the command to retrieve parameter information):\n";
        std::cout << "\n    raster          Display basic properties (rows, cols etc) for a raster.";
        std::cout << "\n    stats           Display specific statistics (mean, max, min, etc) for a raster.";
        std::cout << "\n    compare         Compare two rasters: check divisible, orthogonal, concurrent and by cell";
        std::cout << "\n    stackstats      Create a raster from a stack of rasters using a statistic (mean, max, min, etc).";

        std::cout << "\n";
        std::cout << "\n    bilinear        Bilinear resample of a raster to produce a new raster.";
        std::cout << "\n    copy            Copy a raster to produce a new raster with the specified extent.";
        std::cout << "\n    mosaic          Stitch two or more overlappint rasters.";
        std::cout << "\n    combine         Combine multiple rasters using one or more methods.";
        std::cout << "\n    makeconcurrent  Make all input rasters concurrent.";
        std::cout << "\n    mask            Mask one raster using another raster or a vector.";
        std::cout << "\n    maskval         Mask one raster using one of its values.";
        std::cout << "\n ";
        std::cout << "\n    setnull         Remove values a number of different ways (high-pass, low-pass etc.)";
        std::cout << "\n ";
        std::cout << "\n    math         Perform basic math on two rasters or a raster and a number.";
        std::cout << "\n    invert       Create a raster from nodata values of another.";
        std::cout << "\n    filter       Perform operations like \"mean\" and \"range\" over a moving window.";
        std::cout << "\n    normalize    Normalize a raster.";
        std::cout << "\n    uniform      Make a uniform raster.";
        std::cout << "\n    fill         Optimized Pit Removal.";
        std::cout << "\n    dist         Euclidean distance calculation.";
        std::cout << "\n    linthesh     Linear thresholding of a raster.";
        std::cout << "\n    areathresh   Thresholding of features below a certain area.";
        std::cout << "\n    smoothedges  Smooth the edges of features delineated by NoData.";

        std::cout << "\n";
        std::cout << "\n    hillshade    Create a hillshade raster.";
        std::cout << "\n    slope        Create a slope raster.";
        std::cout << "\n    png          Create a PNG image copy of a raster.";
        std::cout << "\n ";
        std::cout << "\n    csv2raster      Create a raster from a .csv file";
        std::cout << "\n    raster2csv      Create a raster from a .csv file";
//        std::cout << "\n    vector2raster   Create a raster from a vector file.";
        std::cout << "\n ";
        std::cout << "\n    extractpoints   Extract point values from a raster using a csv.";
        std::cout << "\n ";
    }
    return PROCESS_OK;
}

int RasterManEngine::RasterProperties(int argc, char * argv[])
{
    if (argc != 3)
    {
        std::cout << "\nRaster Properties Usage:";
        std::cout << "\n    Usage: rasterman raster <raster_file_path>";
        std::cout << "\n    Command: raster";
        std::cout << "\n Arguments:";
        std::cout << "\n    raster_file_path: Absolute full path to existing raster file.";
        std::cout << "\n";
        return PROCESS_OK;
    }

    RasterManager::PrintRasterProperties(argv[2]);
    return PROCESS_OK;
}

int RasterManEngine::BiLinearResample(int argc, char * argv[])
{
    if (argc != 9)
    {
        std::cout << "\n Bilinear Resample Usage:";
        std::cout << "\n    Syntax: rasterman bilinear <raster_file_path> <output_file_path> <left> <top> <rows> <cols> <cell_size>";
        std::cout << "\n   Command: bilinear";
        std::cout << "\n";
        std::cout << "\n Arguments:";
        std::cout << "\n    raster_file_path: Absolute full path to existing raster file.";
        std::cout << "\n    output_file_path: Absolute full path to output, resampled raster file.";
        std::cout << "\n    left: Left coordinate of the output raster extent.";
        std::cout << "\n    top: Top coordinate of the output raster extent.";
        std::cout << "\n    rows: Number of rows in the output raster.";
        std::cout << "\n    cols: Number of columns in the output raster.";
        std::cout << "\n    cell_size: Cell size for the output raster.";
        std::cout << "\n";
        return PROCESS_OK;
    }

    std::cout << "\n\n --  Bilinear Resampling --";

    double fLeft, fTop, fCellSize;
    int nRows, nCols;
    int eResult = PROCESS_OK;

    GetOutputRasterProperties(fLeft, fTop, nRows, nCols, fCellSize, argc, argv, 4);

    RasterManager::Raster rOriginal(argv[2]);
    eResult = rOriginal.ReSample(argv[3], fCellSize, fLeft, fTop, nRows, nCols);

    std::cout << "\n\n Input Raster: --------------------\n";
    PrintRasterProperties(argv[2]);
    std::cout << "\n\n Output Raster: --------------------\n";
    PrintRasterProperties(argv[3]);

    return eResult;


}

int RasterManEngine::RasterCopy(int argc, char * argv[])
{
    if (argc != 9)
    {
        std::cout << "\nRaster Copy: Copy an existing raster cell by cell, specifying the dimensions out the output.";
        std::cout << "\n   Usage: rasterman copy <raster_file_path> <output_file_path> <left> <top> <rows> <cols> <cell_size>";
        std::cout << "\n";
        std::cout << "\n Arguments:";
        std::cout << "\n   raster_file_path: Absolute full path to existing raster file.";
        std::cout << "\n   output_file_path: Absolute full path to output raster file.";
        std::cout << "\n               left: Left coordinate of the output raster extent.";
        std::cout << "\n                top: Top coordinate of the output raster extent.";
        std::cout << "\n               rows: Number of rows in the output raster.";
        std::cout << "\n               cols: Number of columns in the output raster.";
        std::cout << "\n          cell_size: Cell size for the output raster.";
        std::cout << "\n";
        return PROCESS_OK;
    }

        double fLeft, fTop, fCellSize;
        int nRows, nCols;
        GetOutputRasterProperties(fLeft, fTop, nRows, nCols, fCellSize, argc, argv, 4);

        int eResult = PROCESS_OK;
        RasterManager::Raster rOriginal(argv[2]);
        eResult = rOriginal.Copy(argv[3], &fCellSize, fLeft, fTop, nRows, nCols);


        PrintRasterProperties(argv[3]);
        return eResult;
}


int RasterManEngine::RasterMath(int argc, char * argv[])
{
    int eResult = PROCESS_OK;
    if (argc != 6 && argc != 5 && argc !=4)
    {
        std::cout << "\n Raster Multiplication: Perform basi math on a raster and a number or on two rasters";
        std::cout << "\n    Usage: rasterman math <operation> <raster1_file_path> [<raster2_file_path>|<numeric_arg>] <output_file_path>";
        std::cout << "\n    Usage: rasterman math power <raster1_file_path> <numeric_arg> <output_file_path>";
        std::cout << "\n ";
        std::cout << "\n Arguments:";
        std::cout << "\n           operation:  Mathematical operation. Currently supported";
        std::cout << "\n ";
        std::cout << "\n                                 add: Add two rasters or a raster and a number.";
        std::cout << "\n                            subtract: Add two rasters or a raster and a number.";
        std::cout << "\n                            multiply: Add two rasters or a raster and a number.";
        std::cout << "\n                              divide: Divide one raster by another or a raster by a number.";
        std::cout << "\n                            multiply: Multiply two rasters or a raster by a number.";
        std::cout << "\n                               power: Take a raster to the power of a number.";
        std::cout << "\n                                sqrt: Take the square root of a raster.";
        std::cout << "\n ";
        std::cout << "\n    raster1_file_path:  Absolute full path to existing first raster.";
        std::cout << "\n    raster2_file_path:  Absolute full path to existing second raster. (optional)";
        std::cout << "\n          numeric_arg:  A numeric operator. Use instead of raster2    (optional)";
        std::cout << "\n     output_file_path:  Absolute full path to desired output raster file.";
        std::cout << "\n                                 ";
        std::cout << "\n      Notes: power cannot use two rasters.";
        std::cout << "\n             sqrt only takes raster1 as an argument";
        std::cout << "\n ";
        return eResult;
    }

    QString sOperator = argv[2];

    // This is the SQRT Case
    if (argc == 5 && sOperator.compare("sqrt", Qt::CaseInsensitive) == 0){
        eResult =  Raster::RasterMath(argv[3], NULL, NULL, argv[2], argv[4]);
    }
    // This is the Power case

    // This is every other case: add, subtract, multiply, divide
    else if (argc == 6){

        bool FileisNumeric;
        QString sArg2(argv[4]);
        double dOperator = sArg2.toDouble(&FileisNumeric);

        if (FileisNumeric && sOperator.compare("power", Qt::CaseInsensitive) == 0){
            eResult =  Raster::RasterMath(argv[3], NULL, &dOperator, argv[2], argv[5]);
        }

        if (!FileisNumeric){
            eResult =  Raster::RasterMath(argv[3], argv[4], NULL, argv[2], argv[5]);
        }
        else {
            eResult =  Raster::RasterMath(argv[3], NULL, &dOperator, argv[2], argv[5]);
        }
    }
    else{
        throw RasterManagerException(ARGUMENT_VALIDATION, "Could not validate arguments.");
    }

    return eResult;

}

int RasterManEngine::Mosaic(int argc, char * argv[])
{
    if (argc != 4)
    {
        std::cout << "\n Stitch together two or more overlapping rasters.";
        std::cout << "\n    Usage: rasterman mosaic <raster_file_paths> <output_file_path>";
        std::cout << "\n ";
        std::cout << "\n Arguments:";
        std::cout << "\n    raster_file_paths: two or more raster file paths; semicolon delimited.";
        std::cout << "\n     output_file_path: Absolute full path to desired output raster file.";
        std::cout << "\n ";
        return PROCESS_OK;
    }

    int eResult = PROCESS_OK;

    eResult = Raster::RasterMosaic(argv[2], argv[3]);

    PrintRasterProperties(argv[3]);
    return eResult;

}

int RasterManEngine::Combine(int argc, char * argv[])
{
    if (argc != 5)
    {
        std::cout << "\n Combine multiple rasters using one or more methods.";
        std::cout << "\n    Usage: rasterman combine <raster_file_paths> <output_file_path> <method>";
        std::cout << "\n ";
        std::cout << "\n Arguments:";
        std::cout << "\n    raster_file_paths: two or more raster file paths; semicolon delimited.";
        std::cout << "\n     output_file_path: Absolute full path to desired output raster file.";
        std::cout << "\n               method: Method to use for combining rasters.";
        std::cout << "\n                       Valid Methods: multiply (more coming as needed)";

        std::cout << "\n ";
        return PROCESS_OK;
    }

    int eResult = PROCESS_OK;

    eResult = Raster::CombineRaster(argv[2], argv[3], argv[4]);

    PrintRasterProperties(argv[3]);
    return eResult;
}


int RasterManEngine::MakeConcurrent(int argc, char * argv[])
{
    if (argc != 4)
    {
        std::cout << "\n Make two or more rasters concurrent with each other.";
        std::cout << "\n    Usage: rasterman makeconcurrent <raster_input_paths> <raster_output_paths>";
        std::cout << "\n ";
        std::cout << "\n Arguments:";
        std::cout << "\n    raster_input_paths: two or more raster file paths; semicolon delimited.";
        std::cout << "\n    raster_output_paths: two or more raster file paths; semicolon delimited.";
        std::cout << "\n                         Must match raster_input_paths.";
        std::cout << "\n ";
        return PROCESS_OK;
    }

    int eResult = Raster::MakeRasterConcurrent(argv[2], argv[3]);

    PrintRasterProperties(argv[3]);
    return eResult;
}

int RasterManEngine::Mask(int argc, char * argv[])
{
    int eResult = PROCESS_OK;

    if (argc != 5)
    {
        std::cout << "\n Mask one raster using another.";
        std::cout << "\n    Usage: rasterman mask <raster_file_path> <raster_mask_path> <output_file_path>";
        std::cout << "\n ";
        std::cout << "\n Arguments:";
        std::cout << "\n    raster_file_path: two or more raster file paths, space delimited.";
        std::cout << "\n    raster_mask_path: A raster to be used as a mask. Mask will be created from NoDataValues.";
        std::cout << "\n    output_file_path: Absolute full path to desired output raster file.";
        std::cout << "\n ";
        return PROCESS_OK;
    }

    eResult =  Raster::RasterMask(
                argv[2],
            argv[3],
            argv[4]);

    return eResult;

}


int RasterManEngine::SetNull(int argc, char * argv[])
{
    int eResult = PROCESS_OK;

    if (argc != 6 && argc !=7)
    {
        std::cout << "\n Set values of a raster to NoData using simple rules.";
        std::cout << "\n    Usage: rasterman setnull <rule> <raster_file_path> <output_file_path> [<arg>| <lowval> <highval>]";
        std::cout << "\n ";
        std::cout << "\n Arguments:";
        std::cout << "\n                rule: Can be one of \"above\", \"below\", \"between\" and \"value\".";
        std::cout << "\n    raster_file_path: two or more raster file paths, space delimited.";
        std::cout << "\n    output_file_path: Absolute full path to desired output raster file.";
        std::cout << "\n                 arg: Value to threshold below or above. Use for \"above\", \"below\" and \"value\"";
        std::cout << "\n    lowval & highval: Low / high value. Use for \"above\" and \"below\"";
        std::cout << "\n ";
        return PROCESS_OK;
    }

    Raster theRaster(argv[3]);

    double dThresh1 = GetDouble(argc, argv, 5);
    double dThresh2 = 0;

    if (argc == 7)
        dThresh2 = GetDouble(argc, argv, 6);

    eResult = theRaster.SetNull(argv[4], argv[2], dThresh1, dThresh2);

    return eResult;

}

int RasterManEngine::MaskVal(int argc, char * argv[])
{
    int eResult = PROCESS_OK;

    if (argc != 5)
    {
        std::cout << "\n Mask a raster by one of its values.";
        std::cout << "\n    Usage: rasterman maskval <raster_file_path> <output_file_path> <mask_Value>";
        std::cout << "\n ";
        std::cout << "\n Arguments:";
        std::cout << "\n    raster_file_path: two or more raster file paths, space delimited.";
        std::cout << "\n    raster_mask_path: A raster to be used as a mask. Mask will be created from NoDataValues.";
        std::cout << "\n    output_file_path: Absolute full path to desired output raster file.";
        std::cout << "\n          mask_Value: (optional) Select mask value. Anything not equal to this value will be set to NoDataVal.";
        std::cout << "\n                      If not used, cells with a mask equal to 'Nodataval' will be masked out.";
        std::cout << "\n ";
        return PROCESS_OK;
    }

    double dMaskVal = GetDouble(argc, argv, 4);
    eResult =  Raster::RasterMaskValue( argv[2], argv[3], dMaskVal );

    return eResult;

}

int RasterManEngine::Uniform(int argc, char * argv[])
{
    if (argc != 5)
    {
        std::cout << "\n Uniform: Make a Raster that has a uniform value everywhere there is data";
        std::cout << "\n    Usage: rasterman uniform <raster_file_path> <output_file_path> <value>";
        std::cout << "\n ";
        std::cout << "\n Arguments:";
        std::cout << "\n    raster_file_path: Absolute full path to existing raster file.";
        std::cout << "\n    output_file_path: Absolute full path to output, slope raster.";
        std::cout << "\n               value: Value to use.";
        std::cout << "\n";
        return PROCESS_OK;
    }

    RasterManager::Raster rOriginal(argv[2]);
    double dVal = GetDouble(argc, argv, 4);
    int eResult = rOriginal.Uniform(argv[3], dVal);

    return eResult;

}

int RasterManEngine::Slope(int argc, char * argv[])
{
    if (argc != 5)
    {
        std::cout << "\n Slope Usage:";
        std::cout << "\n    Usage: rasterman slope <degrees | percent> <dem_file_path> <slope_file_path>";
        std::cout << "\n   Command: slope";
        std::cout << "\n ";
        std::cout << "\n Arguments:";
        std::cout << "\n    type: degrees or percent";
        std::cout << "\n    raster_file_path: Absolute full path to existing raster file.";
        std::cout << "\n    output_file_path: Absolute full path to output, slope raster.";
        std::cout << "\n";
        return PROCESS_OK;
    }

    RasterManager::Raster rOriginal(argv[3]);
    int eResult = rOriginal.Slope(argv[4], argv[2]);

    return eResult;
}

int RasterManEngine::Hillshade(int argc, char * argv[])
{
    if (argc != 4)
    {
        std::cout << "\n Create Hillshade:";
        std::cout << "\n    Syntax: rasterman hillshade <raster_file_path> <output_file_path>";
        std::cout << "\n   Command: hillshade";
        std::cout << "\n";
        std::cout << "\n Arguments:";
        std::cout << "\n    raster_file_path: Absolute full path to existing raster file.";
        std::cout << "\n    output_file_path: Absolute full path to output, hillshade raster file.";
        std::cout << "\n";
        return PROCESS_OK;
    }

    RasterManager::Raster rOriginal(argv[2]);
    int eResult = rOriginal.Hillshade(argv[3]);

    return eResult;
}

int RasterManEngine::PNG(int argc, char * argv[])
{
    int eResult = PROCESS_OK;
    Raster_SymbologyStyle eStyle = GSS_Unknown;

    switch (argc) {
    case 7: break;
    case 8: eStyle = (Raster_SymbologyStyle) RasterManager::GetSymbologyStyleFromString(argv[7]); break;
    default:
        std::cout << "\n Create a PNG Image File:";
        std::cout << "\n    Syntax: rasterman png <raster_file_path> <output_file_path> <quality> <long_axis> <opacity> [<raster_type>]";
        std::cout << "\n   Command: PNG";
        std::cout << "\n";
        std::cout << "\n Arguments:";
        std::cout << "\n    raster_file_path: Absolute full path to existing raster file.";
        std::cout << "\n    output_file_path: Absolute full path to output, hillshade raster file.";
        std::cout << "\n             quality: Image quality integer from 1 to 100. (100 is highest quality.)";
        std::cout << "\n           long_axis: Number of pixels on the longer of the width or height.";
        std::cout << "\n             opacity: png opacity from 0 to 100. 100 is solid and 0 is transparent.";
        std::cout << "\n         raster_type: (optional) Known raster type. Specify to apply a color gradient.";
        std::cout << "\n                      Leave blank for grayscale PNG image.";
//        std::cout << "\n                      Valid Options: DEM, DoD, Error, HillShade, PointDensity, SlopeDeg, SlopePC";
        std::cout << "\n                      Valid Options: DEM, HillShade";
        std::cout << "\n";
        return eResult;
        break;
    }

    int nQuality = GetInteger(argc, argv,4);
    int nLongLength = GetInteger(argc,argv, 5);
    int nTransparency = GetInteger(argc, argv, 6);

    RasterManager::Raster rOriginal(argv[2]);
    eResult = rOriginal.RastertoPng(argv[3], nQuality, nLongLength, nTransparency, eStyle);

    return eResult;
}

int RasterManEngine::CSVToRaster(int argc, char * argv[])
{
    if (argc != 8 && argc != 13)
    {
        std::cout << "\n Convert a CSV file into a raster.";
        std::cout << "\n    Usage: rasterman csv2raster <csv_file_path> <output_raster_path> <XField> <YField> <DataField> [<top> <left> <rows> <cols> <cell_size> <no_data_val>] | <raster_template>";
        std::cout << "\n ";
        std::cout << "\n Arguments:";
        std::cout << "\n       csv_file_path: Absolute full path to existing .csv file.";
        std::cout << "\n  output_raster_path: Absolute full path to desired output raster file.";
        std::cout << "\n";
        std::cout << "\n         XField: Name of the field to use for the x values";
        std::cout << "\n         YField: Name of the field to use for the y values";
        std::cout << "\n      DataField: Name of the field to use for the raster data.";
        std::cout << "\n";
        std::cout << "\n      Either specify the command line values of the output raster:";
        std::cout << "\n";
        std::cout << "\n            top: Top coordinate of the output raster extent.";
        std::cout << "\n           left: Left coordinate of the output raster extent.";
        std::cout << "\n           rows: Number of rows in the output raster.";
        std::cout << "\n           cols: Number of columns in the output raster.";
        std::cout << "\n      cell_size: Cell size for the output raster.";
        std::cout << "\n    no_data_val: number to use for no data val (use \"min\" for minimum float)";
        std::cout << "\n     Projection: Spatial reference from various text formats. eg: \"NAD83\", \"WGS84\" etc.";
        std::cout << "\n";
        std::cout << "\n      Or pass in a raster file with extent, cellsize and projection set correctly.";
        std::cout << "\n";
        std::cout << "\n    raster_template: Path to template raster file.";
        std::cout << "\n\n";
        return PROCESS_OK;
    }
    int eResult = PROCESS_OK;

    // Either all
    if (argc == 13){

        double dLeft, dTop, dCellSize, dNoDataVal;
        int nRows, nCols;

        GetOutputRasterProperties(dTop, dLeft, nRows, nCols, dCellSize, argc, argv, 7);

        QString sNoDataVal = argv[12];
        if (sNoDataVal.compare("min", Qt::CaseInsensitive) == 0)
            dNoDataVal = (double) -std::numeric_limits<float>::max();
        else
            dNoDataVal = sNoDataVal.toDouble();

        eResult = RasterManager::Raster::CSVtoRaster(argv[2],
                argv[3],
                dTop, dLeft, nRows, nCols,
                dCellSize, dNoDataVal,
                argv[4],
                argv[5],
                argv[6] );
    }
    // Otherwise a csv file is used
    else if (argc == 8){
        eResult = RasterManager::Raster::CSVtoRaster(argv[2],
                argv[3],
                argv[7],
                argv[4],
                argv[5],
                argv[6] );

    }
    PrintRasterProperties(argv[3]);
    return eResult;

}



int RasterManEngine::RasterToCSV(int argc, char * argv[])
{
    if (argc != 4)
    {
        std::cout << "\n Convert a CSV file into a raster.";
        std::cout << "\n    Usage: rasterman raster2csv <input_raster_path> <csv_file_path>";
        std::cout << "\n ";
        std::cout << "\n Arguments:";
        std::cout << "\n   input_raster_path: Absolute full path to existing raster file.";
        std::cout << "\n       csv_file_path: Absolute full path to desired output .csv file.";
        std::cout << "\n\n";
        return PROCESS_OK;
    }
    int eResult = PROCESS_OK;

    eResult = RasterManager::Raster::RasterToCSV( argv[2], argv[3] );

    return eResult;

}


int RasterManEngine::invert(int argc, char * argv[])
{
    if (argc != 4 && argc != 5)
    {
        std::cout << "\n Reassigns all NoData values to a constant value.";
        std::cout << "\n    Usage: rasterman invert <input_raster_path> <output_raster_path> [<value>]";
        std::cout << "\n ";
        std::cout << "\n Arguments:";
        std::cout << "\n   input_raster_path: Absolute full path to existing input raster file.";
        std::cout << "\n  output_raster_path: Absolute full path to desired output raster file.";
        std::cout << "\n               value: (optional) The value to use. 1 is the default";
        std::cout << "\n\n";
        return PROCESS_OK;
    }
    int eResult = PROCESS_OK;
    double dValue = 1;

    // The user specifies a value
    if (argc == 5){
        dValue = GetDouble(argc, argv, 4);
    }

    eResult = RasterManager::Raster::InvertRaster(
            argv[2],
            argv[3],
            dValue );

    PrintRasterProperties(argv[3]);
    return eResult;

}

int RasterManEngine::fill(int argc, char * argv[])
{
    if (argc != 4 && argc != 5)
    {
        std::cout << "\n Fill - Optimized Pit Removal.";
        std::cout << "\n    Usage: rasterman fill <input_raster_path> <output_raster_path> [<method>]";
        std::cout << "\n ";
        std::cout << "\n Arguments:";
        std::cout << "\n   input_raster_path: Absolute full path to existing input raster file.";
        std::cout << "\n  output_raster_path: Absolute full path to desired output raster file.";
        std::cout << "\n           ";
        std::cout << "\n              method: (optional)";
        std::cout << "\n                        mincost: (default) for Minimize Absolute Elevation Change";
        std::cout << "\n                        bal: for Minimize Net Elevation Change";
        std::cout << "\n                        cut: for Cut Only";
        std::cout << "\n\n";

        return PROCESS_OK;
    }
    int eResult = PROCESS_OK;

    //    Mincost is the default
    FillMode nMethod = FILL_MINCOST;
    // FILL_MINCOST, FILL_BAL , FILL_CUT

    // The user specifies values for window shape
    if (argc == 6){
        nMethod = (FillMode) GetFillMethodFromString(argv[4]);
    }

    RasterPitRemoval rasterPitRemove( argv[2], argv[3], nMethod );

    eResult = rasterPitRemove.Run();

    return eResult;

}



int RasterManEngine::filter(int argc, char * argv[])
{
    if (argc != 7 && argc != 5)
    {
        std::cout << "\n Filter computes an operation on a moving window.";
        std::cout << "\n    Usage: rasterman filter <operation> <input_raster_path> <output_raster_path> [<window_width> <window_height>]";
        std::cout << "\n ";
        std::cout << "\n Arguments:";
        std::cout << "\n           operation: What to do over a moving window. For now \"mean\", and \"range\" are currently supported.";
        std::cout << "\n                            Mean: Take a mean or Average over all valid cells in the window.";
        std::cout << "\n                           range: Max - Min of valid values in the window.";
        std::cout << "\n           ";
        std::cout << "\n   input_raster_path: Absolute full path to existing input raster file.";
        std::cout << "\n  output_raster_path: Absolute full path to desired output raster file.";
        std::cout << "\n           ";
        std::cout << "\n        window_width: (optional) Width (in cells) of moving window. Default is 3. Must be odd.";
        std::cout << "\n       window_height: (optional) Height (in cells) of moving window. Default is 3. Must be odd";
        std::cout << "\n                NOTE: The maximum widths allowed are 15 cells";
        std::cout << "\n\n";

        return PROCESS_OK;
    }
    int eResult = PROCESS_OK;

    // Default window shape
    int nWindowWidth = 3;
    int nWindowHeight = 3;


    // The user specifies values for window shape
    if (argc == 7){
        nWindowWidth = GetInteger(argc,argv, 5);
        nWindowHeight = GetInteger(argc,argv, 6);
    }

    eResult = RasterManager::Raster::FilterRaster(
                argv[2],
            argv[3],
            argv[4],
            nWindowWidth,
            nWindowHeight);

    PrintRasterProperties(argv[4]);
    return eResult;

}

int RasterManEngine::extractpoints(int argc, char * argv[]){
    if (argc < 5 || argc > 8)
    {
        std::cout << "\n Extract cell values from a raster.";
        std::cout << "\n    Usage: rasterman extractpoints <input_csv_path> <input_raster_path> <output_csv_path> [<x_field> <y_field>] [<nodata>]";
        std::cout << "\n               ";
        std::cout << "\n Arguments:    ";
        std::cout << "\n      input_csv_path: Absolute full path to existing csv file with points we want to extract.";
        std::cout << "\n   input_raster_path: Absolute full path to existing input raster file.";
        std::cout << "\n     output_csv_path: Absolute full path to desired output csv file containing the extracted values.";
        std::cout << "\n               ";
        std::cout << "\n        x_field: If your CSV has a header row you can specify an X field name.";
        std::cout << "\n        y_field: If your CSV has a header row you can specify an Y field name.";
        std::cout << "\n         nodata: (Optional) The string to use for nodata. Default is \"-9999\" ";
        std::cout << "\n\n";

        return PROCESS_OK;
    }
    int eResult = PROCESS_OK;

    QString XField = "";
    QString YField = "";
    QString Nodata = "";

    // IF no XY specified but there is a nodata value
    if (argc == 6){
        Nodata = QString(argv[5]);
    }
    else if (argc == 7){
        XField = QString(argv[5]);
        YField = QString(argv[6]);
    }
    // If XY and nodata valu specified
    else if (argc == 8){
        XField = QString(argv[5]);
        YField = QString(argv[6]);
        Nodata = QString(argv[7]);
    }

    eResult = RasterManager::Raster::ExtractPoints(
                argv[2],
            argv[3],
            argv[4],
            XField,
            YField,
            Nodata );

    return eResult;
}

int RasterManEngine::normalize(int argc, char * argv[])
{
    if (argc != 4)
    {
        std::cout << "\n Divide a raster by its maximum value.";
        std::cout << "\n    Usage: rasterman normalize <input_raster_path> <output_raster_path>";
        std::cout << "\n ";
        std::cout << "\n Arguments:";
        std::cout << "\n   input_raster_path: Absolute full path to existing input raster file.";
        std::cout << "\n  output_raster_path: Absolute full path to desired output raster file.";
        std::cout << "\n\n";

        return PROCESS_OK;
    }
    int eResult = PROCESS_OK;
    eResult = RasterManager::Raster::NormalizeRaster(
            argv[2],
            argv[3] );

    PrintRasterProperties(argv[3]);
    return eResult;

}


int RasterManEngine::dist(int argc, char * argv[])
{
    if (argc != 4)
    {
        std::cout << "\n Euclidean distance calculation.";
        std::cout << "\n    Usage: rasterman dist <input_raster_path> <output_raster_path>";
        std::cout << "\n ";
        std::cout << "\n Arguments:";
        std::cout << "\n   input_raster_path: Absolute full path to existing input raster file.";
        std::cout << "\n  output_raster_path: Absolute full path to desired output raster file.";
        std::cout << "\n\n";

        return PROCESS_OK;
    }
    int eResult = PROCESS_OK;
    eResult = RasterManager::Raster::EuclideanDistance(
            argv[2],
            argv[3] );

    PrintRasterProperties(argv[3]);
    return eResult;

}

int RasterManEngine::VectorToRaster(int argc, char * argv[])
{
    return PROCESS_OK;
    if (argc < 6)
    {
        std::cout << "\n Convert a Vector file into a raster.";
        std::cout << "\n    Usage: rasterman vector2raster <vector_file_path> <output_raster_path> <vector_layer> <vector_field> [<cell_size> | <raster_template_path>]";
        std::cout << "\n ";
        std::cout << "\n Arguments:";
        std::cout << "\n       vector_file_path: Absolute full path to existing .shp file.";
        std::cout << "\n     output_raster_path: Absolute full path to desired output raster file.";
        std::cout << "\n           vector_field: Name of the field. use \"quotes\" for names with spaces.";
        std::cout << "\n";
        std::cout << "\n      cell_size: Cell size for the output raster.";
        std::cout << "\n          OR";
        std::cout << "\n    raster_template_path: Path to template raster file. File will be used for extents, cell size etc.";
        std::cout << "\n\n";
        return PROCESS_OK;
    }
    int eResult = PROCESS_OK;

    // Either the last parameter is a double which indicates we are being given cell size.
    QString sArg2 = argv[5];

    bool FileisNumeric;
    double dCellSize = sArg2.toDouble(&FileisNumeric);

    if (FileisNumeric){
        eResult = RasterManager::Raster::VectortoRaster( argv[2],    // sVectorSourcePath
                argv[3],    // sRasterOutputPath
                dCellSize,  // dCellWidth
                argv[4]     // FieldNAme
                );
    }
    // Or we are using a template raster for the bounds
    else {
        eResult = RasterManager::Raster::VectortoRaster( argv[2],  // sVectorSourcePath
                argv[3],  // sRasterOutputPath
                argv[5],  // sRasterTemplate
                argv[4]   // FieldNAme
                );
    }

    return eResult;

}


int RasterManEngine::Stats(int argc, char * argv[])
{
    if (argc != 4)
    {
        std::cout << "\n Raster Statistics:";
        std::cout << "\n    Usage: rasterman stats <operation> <raster>";
        std::cout << "\n";
        std::cout << "\n Arguments:";
        std::cout << "\n    operation:";
        std::cout << "\n              mean: Mean (average) of the inputs";
//        std::cout << "\n          majority: Value that occurs most often";
        std::cout << "\n           maximum: determines the largest value";
//        std::cout << "\n            median: Median of inputs";
        std::cout << "\n           minimum: Smallest non-nodata vlaue of hte inputs";
//        std::cout << "\n          minority: Value that occurs the most often";
        std::cout << "\n             range: distance between max and min";
        std::cout << "\n               std: Calculates the standard deviation of the inpluts";
//        std::cout << "\n               sum: Total of all values";
//        std::cout << "\n             count: number of cells with values.";
//        std::cout << "\n           variety: Number of unique Values";
        std::cout << "\n                                                     ";
        std::cout << "\n    raster: Absolute full path to an existing raster.";
        std::cout << "\n";
        return PROCESS_OK;
    }

    RasterManager::Raster rRaster(argv[3]);
    double dVal;
    int eResult = rRaster.RasterStat( (Raster_Stats_Operation) GetStatFromString(argv[2]), &dVal);

    QString msg = QString("Raster Property \"%1\" is: %2").arg(argv[2]).arg(dVal);

    std::cout << "\n" << msg.toStdString();

    return PROCESS_OK;
}

int RasterManEngine::Compare(int argc, char * argv[])
{
    if (argc != 4)
    {
        std::cout << "\n Raster Compare: check rasters for orthogonality, divisibility, concurrency.";
        std::cout << "\n    Usage: rasterman compare <raster1> <raster2>";
        std::cout << "\n";
        std::cout << "\n Arguments:";
        std::cout << "\n    operation:";
        std::cout << "\n              raster1: Path to first raster";
        std::cout << "\n              raster2: Path to second raster";
        std::cout << "\n";
        return PROCESS_OK;
    }

    RasterManager::RasterArray raRaster1(argv[2]);
    RasterManager::RasterArray raRaster2(argv[3]);

    const QString sFalse = "False";
    const QString sTrue = "True";

    QString sDivisible1 = sFalse;
    QString sDivisible2 = sFalse;
    QString sOrthogonal = sFalse;
    QString sConcurrent = sFalse;
    QString sCellCompare = "Cells are not the same";

    if (raRaster1.IsDivisible())
        sDivisible1 = sTrue;
    if (raRaster2.IsDivisible())
        sDivisible2 = sTrue;
    if (raRaster1.IsOrthogonal(&raRaster2))
        sOrthogonal = sTrue;
    if (raRaster1.IsConcurrent(&raRaster2))
        sConcurrent = sTrue;
    if (raRaster1.CellCompare(&raRaster2))
        sCellCompare = "Cells are the same";

    RasterManager::printLine( QString(" Raster: %1").arg(argv[2]));
    RasterManager::printLine( QString("     Divisible: %1").arg(sDivisible1));
    RasterManager::printLine("");
    RasterManager::printLine( QString(" Raster: %1").arg(argv[3]));
    RasterManager::printLine( QString("     Divisible: %1").arg(sDivisible2));
    RasterManager::printLine("");
    RasterManager::printLine( QString(" Comparison: "));
    RasterManager::printLine( QString("             Orthogonal: %1").arg(sOrthogonal));
    RasterManager::printLine( QString("             Concurrent: %1").arg(sConcurrent));
    RasterManager::printLine( QString("            CellCompare: %1").arg(sCellCompare));

    return PROCESS_OK;
}

int RasterManEngine::StackStats(int argc, char * argv[])
{
    if (argc != 4)
    {
        std::cout << "\n Raster Stack Statistics:";
        std::cout << "\n    Usage: rasterman stackstats <operation> <raster_file_paths> <output_file_path>";
        std::cout << "\n";
        std::cout << "\n Arguments:";
        std::cout << "\n    operation:";
        std::cout << "\n              mean: Mean (average) of the inputs";
//        std::cout << "\n          majority: Value that occurs most often";
        std::cout << "\n           maximum: determines the largest value";
//        std::cout << "\n            median: Median of inputs";
        std::cout << "\n           minimum: Smallest non-nodata vlaue of hte inputs";
//        std::cout << "\n          minority: Value that occurs the most often";
        std::cout << "\n             range: distance between max and min";
        std::cout << "\n               std: Calculates the standard deviation of the inpluts";
//        std::cout << "\n               sum: Total of all values";
//        std::cout << "\n             count: number of cells with values.";
//        std::cout << "\n           variety: Number of unique Values";
        std::cout << "\n                                                     ";
        std::cout << "\n    raster_file_paths: two or more raster file paths; semicolon delimited.";
        std::cout << "\n     output_file_path: Absolute full path to desired output raster file.";
        std::cout << "\n";
        return PROCESS_OK;
    }

    double eResult = Raster::StackStats( (Raster_Stats_Operation) GetStatFromString(argv[2]), argv[3], argv[4] );

    return eResult;

}

int RasterManEngine::LinThresh(int argc, char * argv[])
{
    if (argc != 8)
    {
        std::cout << "\n LinThresh: Threshold the values of a raster using linear interpolation.";
        std::cout << "\n    Usage: rasterman linthesh <raster_input_path> <raster_output_path> <min_thresh> <min_thesh_val> <max_thresh> <max_thresh_val>";
        std::cout << "\n ";
        std::cout << "\n Arguments:";
        std::cout << "\n     raster_input_path: Path to an existing raster file.";
        std::cout << "\n    raster_output_path: Path to the desired output raster file.";
        std::cout << "\n            min_thresh: Raster value, below which the min_thresh_val will be used.";
        std::cout << "\n         min_thesh_val: The value to use for the low threshold point.";
        std::cout << "\n            max_thresh: Raster value, above which the max_thresh_val will be used.";
        std::cout << "\n        max_thresh_val: The value to use for the high threshold point.";
        std::cout << "\n ";
        std::cout << "\n     Notes: - Anything between min_thresh and max_thresh will be linearly interpolated";
        std::cout << "\n            - Set min_thresh = max_thresh to get a straight dropoff";
        std::cout << "\n            - min_thresh must be less than or equal to max_thresh";
        return PROCESS_OK;
    }

    double dMinThresh = GetDouble(argc, argv, 4);
    double dMinThreshVal = GetDouble(argc, argv, 5);
    double dMaxThresh = GetDouble(argc, argv, 6);
    double dMaxThreshVal = GetDouble(argc, argv, 7);

    int eResult = Raster::LinearThreshold(argv[2], argv[3], dMinThresh, dMinThreshVal, dMaxThresh, dMaxThreshVal);

    PrintRasterProperties(argv[3]);
    return eResult;
}

int RasterManEngine::GutPoly(int argc, char * argv[]){
    if (argc != 6)
    {
        std::cout << "\n GutPoly: Vectorize a GUT evidence raster and cram it into a SHP file.";
        std::cout << "\n    Usage: rasterman gutpoly <shp_input_path> <raster_input_path> <tier1> <tier2>";
        std::cout << "\n ";
        std::cout << "\n Arguments:";
        std::cout << "\n       shp_input_path: Path to an existing raster file.";
        std::cout << "\n    raster_input_path: Path to the desired output raster file.";
        std::cout << "\n                tier1: Name of tier1 unit.";
        std::cout << "\n                tier2: Name of tier2 unit.";
        std::cout << "\n ";
        std::cout << "\n        Note: This interface is EXPERIMENTAL and used for testing only.";
        std::cout << "\n              what could possibleye go wrong?";
        std::cout << "\n ";
        return PROCESS_OK;
    }

    int eResult = Raster2Polygon::AddGut(argv[2], argv[3], argv[4], argv[5]);

    return eResult;

}

int RasterManEngine::AreaThresh(int argc, char * argv[])
{

    if (argc != 5)
    {
        std::cout << "\n Area Threshold: Threshold features separated by NoData value below a certain area.";
        std::cout << "\n    Usage: rasterman areathresh <raster_input_path> <raster_output_path> <area_thresh>";
        std::cout << "\n ";
        std::cout << "\n Arguments:";
        std::cout << "\n     raster_input_path: Path to an existing raster file.";
        std::cout << "\n    raster_output_path: Path to the desired output raster file.";
        std::cout << "\n           area_thresh: Area below which a feature will be excluded.";
        std::cout << "\n ";
        return PROCESS_OK;
    }

    double dAreaThresh = GetDouble(argc, argv, 4);

    RasterArray raRaster(argv[2]);
    int eResult = raRaster.AreaThresholdRaster(argv[3], dAreaThresh);

    PrintRasterProperties(argv[3]);
    return eResult;

}


int RasterManEngine::SmoothEdges(int argc, char * argv[])
{

    if (argc != 5)
    {
        std::cout << "\n Smooth Edge: Subtract pixels from the edges and then add them back to smooth.";
        std::cout << "\n    Usage: rasterman smoothedges <raster_input_path> <raster_output_path> <cells>";
        std::cout << "\n ";
        std::cout << "\n Arguments:";
        std::cout << "\n     raster_input_path: Path to an existing raster file.";
        std::cout << "\n    raster_output_path: Path to the desired output raster file.";
        std::cout << "\n                 cells: Number of cells (pixels) to remove and then add back.";
        std::cout << "\n ";
        return PROCESS_OK;
    }

    int nCells = GetInteger(argc, argv, 4);

    RasterArray raRaster(argv[2]);
    int eResult = raRaster.SmoothEdge(argv[3], nCells);

    PrintRasterProperties(argv[3]);
    return eResult;

}

int RasterManEngine::GetInteger(int argc, char * argv[], int nIndex)
{
    int nResult = 0;
    if (nIndex < argc)
    {
        // Enough arguments
        QString sInput = argv[nIndex];
        if (sInput.isNull() || sInput.isEmpty())
            throw RasterManagerException(MISSING_ARGUMENT, "Command line missing integer argument.");
        else
        {
            sInput = sInput.trimmed();
            bool bOK = false;
            nResult = sInput.toInt(&bOK);
            if (!bOK)
                throw RasterManagerException(ARGUMENT_VALIDATION, "Unable to convert input to integer numeric value.");
        }
    }
    else
        throw RasterManagerException(MISSING_ARGUMENT, "Insufficient command line arguments for operation.");

    return nResult;
}

double RasterManEngine::GetDouble(int argc, char * argv[], int nIndex)
{
    double fResult = 0;
    if (nIndex < argc)
    {
        // Enough arguments
        QString sInput = argv[nIndex];
        if (sInput.isNull() || sInput.isEmpty())

            throw  RasterManagerException(MISSING_ARGUMENT, "Command line missing integer argument.");
        else
        {
            sInput = sInput.trimmed();
            bool bOK = false;
            fResult = sInput.toDouble(&bOK);
            if (!bOK)
                throw  RasterManagerException(ARGUMENT_VALIDATION, "Unable to convert input to double numeric value.");
        }
    }
    else
        throw RasterManagerException(MISSING_ARGUMENT, "Insufficient command line arguments for operation.");

    return fResult;
}


void RasterManEngine::GetOutputRasterProperties(double & fLeft, double & fTop, int & nRows, int & nCols, double & fCellSize, int argc, char * argv[], int nStartArg)
{
    fLeft = GetDouble(argc, argv, nStartArg);
    fTop = GetDouble(argc, argv, (nStartArg+1));
    nRows = GetInteger(argc, argv, (nStartArg+2));
    nCols = GetInteger(argc, argv, (nStartArg+3));
    fCellSize = GetDouble(argc, argv, (nStartArg+4));
}

} //RasterManager
