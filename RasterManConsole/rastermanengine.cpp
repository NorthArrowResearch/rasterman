#include "rastermanengine.h"

#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <cstring>

#include "raster.h"
#include "rastermanager_interface.h"

namespace RasterManager {

RasterManEngine::RasterManEngine()
{ }

int RasterManEngine::Run(int argc, char * argv[])
{

    bool bRecognizedCommand = true;

    if (argc > 1)
    {
        int eResult = PROCESS_OK;

        RasterManager::RegisterGDAL();
        QString sCommand(argv[1]);

        if (QString::compare(sCommand, "Raster", Qt::CaseInsensitive) == 0)
            eResult = RasterProperties(argc, argv);

        else if (QString::compare(sCommand, "BiLinear", Qt::CaseInsensitive) == 0)
            eResult = BiLinearResample(argc, argv);

        else if (QString::compare(sCommand, "Copy", Qt::CaseInsensitive) == 0)
            eResult = RasterCopy(argc, argv);

        else if (QString::compare(sCommand, "Add", Qt::CaseInsensitive) == 0)
            eResult = RasterAdd(argc, argv);

        else if (QString::compare(sCommand, "Subtract", Qt::CaseInsensitive) == 0)
            eResult = RasterSubtract(argc, argv);

        else if (QString::compare(sCommand, "Divide", Qt::CaseInsensitive) == 0)
            eResult = RasterDivide(argc, argv);

        else if (QString::compare(sCommand, "Multiply", Qt::CaseInsensitive) == 0)
            eResult = RasterMultiply(argc, argv);

        else if (QString::compare(sCommand, "Power", Qt::CaseInsensitive) == 0)
            eResult = RasterPower(argc, argv);

        else if (QString::compare(sCommand, "Sqrt", Qt::CaseInsensitive) == 0)
            eResult = RasterSqrt(argc, argv);

        else if (QString::compare(sCommand, "csv2raster", Qt::CaseInsensitive) == 0)
            eResult = CSVToRaster(argc, argv);

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

        else if (QString::compare(sCommand, "PNG", Qt::CaseInsensitive) == 0)
            eResult = PNG(argc, argv);

        else
            bRecognizedCommand = false;

        RasterManager::DestroyGDAL();

        return eResult;
    }
    else
        bRecognizedCommand = false;

    if (!bRecognizedCommand)
    {
        std::cout << "\n Usage: rasterman <command> [paramters...]\n";

        std::cout << "\n Commands (type rasterman followed by the command to retrieve parameter information):\n";
        std::cout << "\n    raster          Display basic properties (rows, cols etc) for a raster.";
        std::cout << "\n    bilinear        Bilinear resample of a raster to produce a new raster.";
        std::cout << "\n    copy            Copy a raster to produce a new raster with the specified extent.";
        std::cout << "\n    mosaic          Stitch two or more overlappint rasters.";
        std::cout << "\n    makeconcurrent  Make all input rasters concurrent.";
        std::cout << "\n    mask            Mask one raster using another raster or a vector.";
        std::cout << "\n    setnull         Set a NoDataValue in a raster based on thesholding.";
        std::cout << "\n ";
        std::cout << "\n    add          Add two rasters or a raster and a constant.";
        std::cout << "\n    subtract     Subtract two rasters or a constant from a raster.";
        std::cout << "\n    divide       Divide a raster by a number or another raster.";
        std::cout << "\n    multiply     Multiply a raster by a number or another raster.";
        std::cout << "\n    power        Raise a raster to a power.";
        std::cout << "\n    sqrt         Get the square root of a raster.";
        std::cout << "\n";
        std::cout << "\n    hillshade       Create a hillshade raster.";
        std::cout << "\n    slope           Create a slope raster.";
        std::cout << "\n    png             Create a PNG image copy of a raster.";
        std::cout << "\n ";
        std::cout << "\n    csv2raster      Create a raster from a .csv file";
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
    CheckFile(argc, argv, 2, true);


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

    CheckFile(argc, argv, 2, true);
    CheckFile(argc, argv, 3, false);

    std::cout << "\n\n --  Bilinear Resampling --";

    double fLeft, fTop, fCellSize;
    int nRows, nCols;
    int eResult = PROCESS_OK;

    GetOutputRasterProperties(fLeft, fTop, nRows, nCols, fCellSize, argc, argv, 4);

    RasterManager::Raster rOriginal(argv[2]);
    eResult = rOriginal.ReSample(argv[3], fCellSize, fLeft, fTop, nRows, nCols);
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

        // Check that the files exist (or not)
        CheckFile(argc, argv, 2, true);
        CheckFile(argc, argv, 3, false);

        double fLeft, fTop, fCellSize;
        int nRows, nCols;
        GetOutputRasterProperties(fLeft, fTop, nRows, nCols, fCellSize, argc, argv, 4);

        int eResult = PROCESS_OK;
        RasterManager::Raster rOriginal(argv[2]);
        eResult = rOriginal.Copy(argv[3], fCellSize, fLeft, fTop, nRows, nCols);
        return eResult;
}

int RasterManEngine::RasterAdd(int argc, char * argv[])
{
    if (argc != 5)
    {
        std::cout << "\n Raster Addition:";
        std::cout << "\n    Usage: rasterman add <raster1_file_path> <raster2_file_path> <output_file_path>";
        std::cout << "\n";
        std::cout << "\n Arguments:";
        std::cout << "\n    raster1_file_path: Absolute full path to existing first raster.";
        std::cout << "\n    raster2_file_path: Absolute full path to existing second raster.";
        std::cout << "\n     output_file_path: Absolute full path to desired output raster file.";
        std::cout << "\n";
        return PROCESS_OK;
    }

    CheckFile(argc, argv, 2, true);
    QString sArg2 = argv[3];
    CheckFile(argc, argv, 4, false);

    int eResult = PROCESS_OK;

    bool FileisNumeric;
    double dOperator = sArg2.toDouble(&FileisNumeric);

    if (!FileisNumeric){
        CheckFile(argc, argv, 3, true);
        eResult =  RasterManager::BasicMath(argv[2],
                argv[3], NULL, RasterManager::RM_BASIC_MATH_ADD,
                argv[4]);
    }
    else {
        eResult =  RasterManager::BasicMath(argv[2],
                NULL, dOperator, RasterManager::RM_BASIC_MATH_ADD,
                argv[4]);
    }
    return eResult;
}

int RasterManEngine::RasterSubtract(int argc, char * argv[])
{
    if (argc != 5)
    {
        std::cout << "\n Raster Subtraction: Subtract the second raster from the first.";
        std::cout << "\n    Usage: rasterman subtract <raster1_file_path> <raster2_file_path> <output_file_path>";
        std::cout << "\n";
        std::cout << "\n Arguments:";
        std::cout << "\n    raster1_file_path: Absolute full path to existing first raster.";
        std::cout << "\n    raster2_file_path: Absolute full path to existing second raster.";
        std::cout << "\n     output_file_path: Absolute full path to desired output raster file.";
        std::cout << "\n";
        return PROCESS_OK;
    }


    CheckFile(argc, argv, 2, true);
    QString sArg2 = argv[3];
    CheckFile(argc, argv, 4, false);

    int eResult = PROCESS_OK;

    bool FileisNumeric;
    double dOperator = sArg2.toDouble(&FileisNumeric);

    if (!FileisNumeric){
        CheckFile(argc, argv, 3, true);
        eResult =  RasterManager::BasicMath(argv[2],
                argv[3], NULL, RasterManager::RM_BASIC_MATH_SUBTRACT,
                argv[4]);
    }
    else {
        eResult =  RasterManager::BasicMath(argv[2],
                NULL, dOperator, RasterManager::RM_BASIC_MATH_ADD,
                argv[4]);
    }
    return eResult;

}
int RasterManEngine::RasterDivide(int argc, char * argv[])
{
    if (argc != 5)
    {
        std::cout << "\n Raster Division: Divide the first raster by the second.";
        std::cout << "\n    Usage: rasterman divide <raster1_file_path> <raster2_file_path> <output_file_path>";
        std::cout << "\n ";
        std::cout << "\n Arguments:";
        std::cout << "\n    raster1_file_path:  Absolute full path to existing first raster.";
        std::cout << "\n    raster2_file_path:  Absolute full path to existing second raster.";
        std::cout << "\n     output_file_path:  Absolute full path to desired output raster file.";
        std::cout << "\n ";
        return PROCESS_OK;
    }

    CheckFile(argc, argv, 2, true);
    QString sArg2 = argv[3];
    CheckFile(argc, argv, 4, false);

    int eResult = PROCESS_OK;

    bool FileisNumeric;
    double dOperator = sArg2.toDouble(&FileisNumeric);

    if (!FileisNumeric){
        CheckFile(argc, argv, 3, true);
        eResult =  RasterManager::BasicMath(argv[2],
                argv[3], NULL, RasterManager::RM_BASIC_MATH_DIVIDE,
                argv[4]);
    }
    else {
        eResult =  RasterManager::BasicMath(argv[2],
                NULL, dOperator, RasterManager::RM_BASIC_MATH_DIVIDE,
                argv[4]);
    }
    return eResult;

}

int RasterManEngine::RasterMultiply(int argc, char * argv[])
{
    if (argc != 5)
    {
        std::cout << "\n Raster Multiplication: Multiply two rasters.";
        std::cout << "\n    Usage: rasterman multiply <raster1_file_path> <raster2_file_path> <output_file_path>";
        std::cout << "\n ";
        std::cout << "\n Arguments:";
        std::cout << "\n    raster1_file_path:  Absolute full path to existing first raster.";
        std::cout << "\n    raster2_file_path:  Absolute full path to existing second raster.";
        std::cout << "\n     output_file_path:   Absolute full path to desired output raster file.";
        std::cout << "\n ";
        return PROCESS_OK;
    }

    CheckFile(argc, argv, 2, true);
    QString sArg2 = argv[3];
    CheckFile(argc, argv, 4, false);

    int eResult = PROCESS_OK;

    bool FileisNumeric;
    double dOperator = sArg2.toDouble(&FileisNumeric);

    if (!FileisNumeric){
        CheckFile(argc, argv, 3, true);
        eResult =  RasterManager::BasicMath(argv[2],
                argv[3], NULL, RasterManager::RM_BASIC_MATH_MULTIPLY,
                argv[4]);
    }
    else {
        eResult =  RasterManager::BasicMath(argv[2],
                NULL, dOperator, RasterManager::RM_BASIC_MATH_MULTIPLY,
                argv[4]);
    }
    return eResult;

}
int RasterManEngine::RasterPower(int argc, char * argv[])
{
    if (argc != 5)
    {
        std::cout << "\n Raise every value in a raster to the power of a value.";
        std::cout << "\n    Usage: rasterman power <raster_file_path> <power_value> <output_file_path>";
        std::cout << "\n ";
        std::cout << "\n Arguments:";
        std::cout << "\n    raster_file_path: Absolute full path to existing first raster.";
        std::cout << "\n         power_value: Value to be used as the power.";
        std::cout << "\n    output_file_path: Absolute full path to desired output raster file.";
        std::cout << "\n ";
        return PROCESS_OK;
    }

    CheckFile(argc, argv, 2, true);
    double dPower = GetDouble(argc, argv, 3);
    CheckFile(argc, argv, 4, false);
    int eResult = PROCESS_OK;

    eResult = RasterManager::BasicMath(argv[2], NULL, dPower, RasterManager::RM_BASIC_MATH_POWER, argv[4]);
    return eResult;
}
int RasterManEngine::RasterSqrt(int argc, char * argv[])
{
    if (argc != 4)
    {
        std::cout << "\n Get the square root of a raster.";
        std::cout << "\n    Usage: rasterman sqrt <raster_file_path> <output_file_path>";
        std::cout << "\n ";
        std::cout << "\n Arguments:";
        std::cout << "\n    raster_file_path: Absolute full path to existing first raster.";
        std::cout << "\n    output_file_path: Absolute full path to desired output raster file.";
        std::cout << "\n ";
        return PROCESS_OK;
    }

    CheckFile(argc, argv, 2, true);
    CheckFile(argc, argv, 3, false);
    int eResult = PROCESS_OK;

    eResult = RasterManager::BasicMath(argv[2], NULL, NULL, RasterManager::RM_BASIC_MATH_SQRT,
            argv[3]);
    return eResult;

}

int RasterManEngine::Mosaic(int argc, char * argv[])
{
    if (argc != 4)
    {
        std::cout << "\n Stitch together two or more overlapping rasters.";
        std::cout << "\n    Usage: rasterman mosaic <raster_file_paths> ... <output_file_path>";
        std::cout << "\n ";
        std::cout << "\n Arguments:";
        std::cout << "\n    raster_file_paths: two or more raster file paths; semicolon delimited.";
        std::cout << "\n    output_file_path: Absolute full path to desired output raster file.";
        std::cout << "\n ";
        return PROCESS_OK;
    }

    QString rasterInputs(argv[2]);
    QStringList inputFileList = rasterInputs.split(";");

    foreach (QString sFilename, inputFileList) {
        if (sFilename.compare("") != 0){
            CheckFile(sFilename, true);
        }

    }

    CheckFile(argc, argv, 3, false);
    int eResult = PROCESS_OK;

    eResult = RasterManager::Mosaic(argv[2], argv[3]);
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


    QString rasterInputs(argv[2]);
    QString rasterOutputs(argv[3]);

    QString delimiterPattern(";");

    QStringList inputFileList = rasterInputs.split(delimiterPattern);
    QStringList outputFileList = rasterOutputs.split(delimiterPattern);

    foreach (QString sFilename, inputFileList) {
        if (sFilename.compare("") != 0) {
            CheckFile(sFilename, true);
        }
    }
    foreach (QString sFilename, outputFileList) {
        if (sFilename.compare("") != 0){
            CheckFile(sFilename, false);
        }
    }

    int eResult = RasterManager::MakeConcurrent(argv[2], argv[3]);

    return eResult;
}


int RasterManEngine::Mask(int argc, char * argv[])
{
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

    CheckFile(argc, argv, 2, true);
    CheckFile(argc, argv, 3, true);
    CheckFile(argc, argv, 4, false);

    int eResult =  RasterManager::Mask(argv[2],
            argv[3],
            argv[4]);

    return eResult;

}

int RasterManEngine::Slope(int argc, char * argv[])
{
    if (argc != 5)
    {
        std::cout << "\n Bilinear Resample Usage:";
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

    CheckFile(argc, argv, 3, true);
    CheckFile(argc, argv, 4, false);

    int nSlopeType = RasterManager::RasterManagerInputCodes::SLOPE_DEGREES;
    QString sType(argv[2]);
    if (sType.compare(sType, "percent", Qt::CaseInsensitive) == 0)
        nSlopeType = RasterManager::RasterManagerInputCodes::SLOPE_PERCENT;

    RasterManager::Raster rOriginal(argv[3]);
    int eResult = rOriginal.Slope(argv[4], nSlopeType);

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

    CheckFile(argc, argv, 2, true);
    CheckFile(argc, argv, 3, false);

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
    case 8: eStyle = RasterManager::GetSymbologyStyleFromString(argv[7]); break;
    default:
        std::cout << "\n Create a PNG Image File:";
        std::cout << "\n    Syntax: rasterman png <raster_file_path> <output_file_path> <quality> <long_axis> <transparency> [<raster_type>]";
        std::cout << "\n   Command: PNG";
        std::cout << "\n";
        std::cout << "\n Arguments:";
        std::cout << "\n    raster_file_path: Absolute full path to existing raster file.";
        std::cout << "\n    output_file_path: Absolute full path to output, hillshade raster file.";
        std::cout << "\n             quality: Image quality integer from 1 to 100. (100 is highest quality.)";
        std::cout << "\n           long_axis: Number of pixels on the longer of the width or height.";
        std::cout << "\n             opacity: png opacity from 0 to 100. 100 is solid and 0 is transparent.";
        std::cout << "\n         raster_type: Known raster type. Leave blank for gray scale PNG image.";
        std::cout << "\n";
        return eResult;
        break;
    }

    CheckFile(argc, argv, 2, true);
    CheckFile(argc, argv, 3, false);

    int nQuality = GetInteger(argc, argv,4);
    int nLongLength = GetInteger(argc,argv, 5);
    int nTransparency = GetInteger(argc, argv, 6);

    RasterManager::Raster rOriginal(argv[2]);
    eResult = rOriginal.PNG(argv[3], nQuality, nLongLength, nTransparency, eStyle);

    return eResult;
}

int RasterManEngine::CSVToRaster(int argc, char * argv[])
{
    if (argc < 8)
    {
        std::cout << "\n Convert a CSV file into a raster.";
        std::cout << "\n    Usage: gcd csv2raster <csv_file_path> <output_file_path> <XField> <YField> <DataField> [<top> <left> <rows> <cols> <cell_size> <no_data_val>] | <raster_template>";
        std::cout << "\n ";
        std::cout << "\n Arguments:";
        std::cout << "\n       csv_file_path: Absolute full path to existing .csv file.";
        std::cout << "\n    output_file_path: Absolute full path to desired output raster file.";
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

    CheckFile(argc, argv, 2, true);
    CheckFile(argc, argv, 3, false);

    // Either all
    if (argc == 13){

        double dLeft, dTop, dCellSize, dNoDataVal;
        int nRows, nCols;

        GetOutputRasterProperties(dTop, dLeft, nRows, nCols, dCellSize, argc, argv, 7);

        QString sNoDataVal = argv[12];
        if (sNoDataVal.compare("min", Qt::CaseInsensitive) == 0)
            dNoDataVal = (double) std::numeric_limits<float>::lowest();
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
        CheckFile(argc, argv, 7, true);
        eResult = RasterManager::Raster::CSVtoRaster(argv[2],
                argv[3],
                argv[7],
                argv[4],
                argv[5],
                argv[6] );
    }

    return eResult;

}


void RasterManEngine::CheckFile(QString sFile, bool bMustExist)
{
    // Enough arguments
    if (sFile.isNull() || sFile.isEmpty())
        throw std::runtime_error("Command line missing a file path.");
    else
    {
        // Check if the directory the file exists in is actually there
        QDir sFilePath = QFileInfo(sFile).absoluteDir();
        if (!sFilePath.exists()){
            QString sErr = "The directory of the file you specified does not exist: " + sFilePath.absolutePath();
            throw  std::runtime_error(sErr.toStdString());
        }

        sFile = sFile.trimmed();
        sFile = sFile.replace("\"","");
        if (bMustExist)
        {
            if (!QFile::exists(sFile)){
                QString sErr = "The specified input file does not exist: " + sFile;
                throw  std::runtime_error(sErr.toStdString());
            }
        }
        else {
            if (QFile::exists(sFile)){
                QString sErr = "The specified output file already exists." + sFile;
                throw  std::runtime_error(sErr.toStdString());
            }
        }
    }


}


void RasterManEngine::CheckFile(int argc, char * argv[], int nIndex, bool bMustExist)
{

    if (nIndex < argc)
    {
        QString filename = argv[nIndex];
        CheckFile(filename, bMustExist);
    }
    else
        throw std::runtime_error("Insufficient command line arguments for operation.");

}

int RasterManEngine::GetInteger(int argc, char * argv[], int nIndex)
{
    int nResult = 0;
    if (nIndex < argc)
    {
        // Enough arguments
        QString sInput = argv[nIndex];
        if (sInput.isNull() || sInput.isEmpty())
            throw std::runtime_error("Command line missing integer argument.");
        else
        {
            sInput = sInput.trimmed();
            bool bOK = false;
            nResult = sInput.toInt(&bOK);
            if (!bOK)
                throw std::runtime_error("Unable to convert input to integer numeric value.");
        }
    }
    else
        throw std::runtime_error("Insufficient command line arguments for operation.");

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
            throw  std::runtime_error("Command line missing integer argument.");
        else
        {
            sInput = sInput.trimmed();
            bool bOK = false;
            fResult = sInput.toDouble(&bOK);
            if (!bOK)
                throw  std::runtime_error("Unable to convert input to double numeric value.");
        }
    }
    else
        throw std::runtime_error("Insufficient command line arguments for operation.");

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
