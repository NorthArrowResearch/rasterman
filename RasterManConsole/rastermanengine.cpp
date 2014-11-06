#include "rastermanengine.h"

#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <cstring>

#include "raster.h"
#include "rastermanager_interface.h"

namespace RasterManager {

RasterManEngine::RasterManEngine(int argc, char * argv[])
{
    bool bRecognizedCommand = true;

    if (argc > 1)
    {
        RasterManager::RegisterGDAL();
        QString sCommand(argv[1]);

        if (QString::compare(sCommand, "Raster", Qt::CaseInsensitive) == 0)
            RasterProperties(argc, argv);

        else if (QString::compare(sCommand, "BiLinear", Qt::CaseInsensitive) == 0)
            BiLinearResample(argc, argv);

        else if (QString::compare(sCommand, "Copy", Qt::CaseInsensitive) == 0)
            RasterCopy(argc, argv);

        else if (QString::compare(sCommand, "Add", Qt::CaseInsensitive) == 0)
            RasterAdd(argc, argv);

        else if (QString::compare(sCommand, "Subtract", Qt::CaseInsensitive) == 0)
            RasterSubtract(argc, argv);

        else if (QString::compare(sCommand, "Divide", Qt::CaseInsensitive) == 0)
            RasterDivide(argc, argv);

        else if (QString::compare(sCommand, "Multiply", Qt::CaseInsensitive) == 0)
            RasterMultiply(argc, argv);

        else if (QString::compare(sCommand, "Power", Qt::CaseInsensitive) == 0)
            RasterPower(argc, argv);

        else if (QString::compare(sCommand, "Sqrt", Qt::CaseInsensitive) == 0)
            RasterSqrt(argc, argv);

        else if (QString::compare(sCommand, "csv2raster", Qt::CaseInsensitive) == 0)
            CSVToRaster(argc, argv);

        else if (QString::compare(sCommand, "Slope", Qt::CaseInsensitive) == 0)
            Slope(argc, argv);

        else if (QString::compare(sCommand, "Hillshade", Qt::CaseInsensitive) == 0)
            Hillshade(argc, argv);

        else if (QString::compare(sCommand, "Mosaic", Qt::CaseInsensitive) == 0)
            Mosaic(argc, argv);

        else if (QString::compare(sCommand, "MakeConcurrent", Qt::CaseInsensitive) == 0)
            MakeConcurrent(argc, argv);

        else if (QString::compare(sCommand, "Mask", Qt::CaseInsensitive) == 0)
            Mask(argc, argv);

        else
            bRecognizedCommand = false;
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
        std::cout << "\n    mask            Mask one raster using another.";
        std::cout << "\n ";
        std::cout << "\n    add          Add two rasters or a raster and a constant.";
        std::cout << "\n    subtract     Subtract two rasters or a constant from a raster.";
        std::cout << "\n    divide       Divide a raster by a number or another raster.";
        std::cout << "\n    multiply     Multiply a raster by a number or another raster.";
        std::cout << "\n    power        Raise a raster to a power.";
        std::cout << "\n    sqrt         Get the square root of a raster.";
        std::cout << "\n";
        std::cout << "\n    hillshade       Create a hillshade raster.";
        std::cout << "\n    mask            Create a slope raster.";
        std::cout << "\n ";
        return;
    }
}

void RasterManEngine::RasterProperties(int argc, char * argv[])
{
    if (argc != 3)
    {
        std::cout << "\nRaster Properties Usage:";
        std::cout << "\n    Usage: rasterman raster <raster_file_path>";
        std::cout << "\n    Command: raster";
        std::cout << "\n Arguments:";
        std::cout << "\n    raster_file_path: Absolute full path to existing raster file.";
        std::cout << "\n";
        return;
    }
    QString sRaster = GetFile(argc, argv, 2, true);

    try
    {
        RasterManager::PrintRasterProperties(sRaster.toStdString().c_str());

        std::cout << "\n\n completed successfully.\n";
    }
    catch (std::exception & ex)
    {
        std::cout <<"Error: " << ex.what() << std::endl;
    }
}

void RasterManEngine::BiLinearResample(int argc, char * argv[])
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
        return;
    }

    try
    {
        QString sOriginal = GetFile(argc, argv, 2, true);
        QString sOutput = GetFile(argc, argv, 3, false);

        std::cout << "\n\n** Bilinear Resampling **";

        double fLeft, fTop, fCellSize;
        int nRows, nCols;
        int eResult;

        GetOutputRasterProperties(fLeft, fTop, nRows, nCols, fCellSize, argc, argv, 4);

        RasterManager::Raster rOriginal(sOriginal.toStdString().c_str());
        eResult = rOriginal.ReSample(sOutput.toStdString().c_str(), fCellSize, fLeft, fTop, nRows, nCols);

        std::cout << "\n" << RasterManager::GetReturnCodeAsString(eResult);
    }
    catch (std::exception & ex)
    {
        std::cout <<"Error: " << ex.what() << std::endl;
    }
}

void RasterManEngine::RasterCopy(int argc, char * argv[])
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
        return;
    }

    try
    {
        QString sOriginal = GetFile(argc, argv, 2, true);
        QString sOutput = GetFile(argc, argv, 3, false);

        double fLeft, fTop, fCellSize;
        int nRows, nCols;
        GetOutputRasterProperties(fLeft, fTop, nRows, nCols, fCellSize, argc, argv, 4);

        int eResult;
        RasterManager::Raster rOriginal(sOriginal.toStdString().c_str());
        eResult = rOriginal.Copy(sOutput.toStdString().c_str(), fCellSize, fLeft, fTop, nRows, nCols);

        std::cout << "\n\n" << GetReturnCodeAsString(eResult) << "\n";
    }
    catch (std::exception & ex)
    {
        std::cout <<"Error: " << ex.what() << std::endl;
    }
}

void RasterManEngine::RasterAdd(int argc, char * argv[])
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
        return;
    }

    try
    {
        QString sRaster1 = GetFile(argc, argv, 2, true);
        QString sArg2 = argv[3];
        QString sOutputRaster = GetFile(argc, argv, 4, false);

        int eResult;

        bool FileisNumeric;
        double dOperator = sArg2.toDouble(&FileisNumeric);

        if (!FileisNumeric){
            QString sRaster2 = GetFile(argc, argv, 3, true);
            eResult =  RasterManager::BasicMath(sRaster1.toStdString().c_str(),
                                     sRaster2.toStdString().c_str(), NULL, RasterManager::RM_BASIC_MATH_ADD,
                                     sOutputRaster.toStdString().c_str());
        }
        else {
            eResult =  RasterManager::BasicMath(sRaster1.toStdString().c_str(),
                                     NULL, dOperator, RasterManager::RM_BASIC_MATH_ADD,
                                     sOutputRaster.toStdString().c_str());
        }

        std::cout << "\n\n" <<  RasterManager::GetReturnCodeAsString(eResult) << "\n";

    }
    catch (std::exception & ex)
    {
        std::cout <<"Error: " << ex.what() << std::endl;
    }
}

void RasterManEngine::RasterSubtract(int argc, char * argv[])
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
        return;
    }

    try
    {
        QString sRaster1 = GetFile(argc, argv, 2, true);
        QString sArg2 = argv[3];
        QString sOutputRaster = GetFile(argc, argv, 4, false);
        int eResult;

        bool FileisNumeric;
        double dOperator = sArg2.toDouble(&FileisNumeric);

        if (!FileisNumeric){
            QString sRaster2 = GetFile(argc, argv, 3, true);
            eResult =  RasterManager::BasicMath(sRaster1.toStdString().c_str(),
                                     sArg2.toStdString().c_str(), NULL, RasterManager::RM_BASIC_MATH_SUBTRACT,
                                     sOutputRaster.toStdString().c_str());
        }
        else {
            eResult =  RasterManager::BasicMath(sRaster1.toStdString().c_str(),
                                     NULL, dOperator, RasterManager::RM_BASIC_MATH_SUBTRACT,
                                     sOutputRaster.toStdString().c_str());
        }

        std::cout << "\n\n" <<  RasterManager::GetReturnCodeAsString(eResult) << "\n";
    }
    catch (std::exception & ex)
    {
        std::cout <<"Error: " << ex.what() << std::endl;
    }
}
void RasterManEngine::RasterDivide(int argc, char * argv[])
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
        return;
    }

    try
    {
        QString sRaster1 = GetFile(argc, argv, 2, true);
        QString sArg2 = argv[3];
        QString sOutputRaster = GetFile(argc, argv, 4, false);

        int eResult;

        bool FileisNumeric;
        double dOperator = sArg2.toDouble(&FileisNumeric);

        if (!FileisNumeric){
            QString sRaster2 = GetFile(argc, argv, 3, true);
            eResult =  RasterManager::BasicMath(sRaster1.toStdString().c_str(),
                                     sArg2.toStdString().c_str(), NULL, RasterManager::RM_BASIC_MATH_DIVIDE,
                                     sOutputRaster.toStdString().c_str());
        }
        else {
            eResult =  RasterManager::BasicMath(sRaster1.toStdString().c_str(),
                                     NULL, dOperator, RasterManager::RM_BASIC_MATH_DIVIDE,
                                     sOutputRaster.toStdString().c_str());
        }
        std::cout << "\n\n" << RasterManager::GetReturnCodeAsString(eResult) << "\n";
    }
    catch (std::exception & ex)
    {
        std::cout <<"Error: " << ex.what() << std::endl;
    }
}
void RasterManEngine::RasterMultiply(int argc, char * argv[])
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
        return;
    }

    try
    {
        QString sRaster1 = GetFile(argc, argv, 2, true);
        QString sArg2 = argv[3];
        QString sOutputRaster = GetFile(argc, argv, 4, false);

        int eResult;

        bool FileisNumeric;
        double dOperator = sArg2.toDouble(&FileisNumeric);

        if (!FileisNumeric){
            QString sRaster2 = GetFile(argc, argv, 3, true);
            eResult =  RasterManager::BasicMath(sRaster1.toStdString().c_str(),
                                     sArg2.toStdString().c_str(), NULL, RasterManager::RM_BASIC_MATH_MULTIPLY,
                                     sOutputRaster.toStdString().c_str());
        }
        else {
            eResult =  RasterManager::BasicMath(sRaster1.toStdString().c_str(),
                                     NULL, dOperator, RasterManager::RM_BASIC_MATH_MULTIPLY,
                                     sOutputRaster.toStdString().c_str());
        }
        std::cout << "\n\n" << RasterManager::GetReturnCodeAsString(eResult) << "\n";
    }
    catch (std::exception & ex)
    {
        std::cout <<"Error: " << ex.what() << std::endl;
    }
}
void RasterManEngine::RasterPower(int argc, char * argv[])
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
        return;
    }

    try
    {
        QString sRaster = GetFile(argc, argv, 2, true);
        double dPower = GetDouble(argc, argv, 3);
        QString sOutputRaster = GetFile(argc, argv, 4, false);
        int eResult;

        eResult = RasterManager::BasicMath(sRaster.toStdString().c_str(), NULL, dPower, RasterManager::RM_BASIC_MATH_POWER,
                                           sOutputRaster.toStdString().c_str());

        std::cout << "\n\n" << RasterManager::GetReturnCodeAsString(eResult) << "\n";
    }
    catch (std::exception & ex)
    {
        std::cout <<"Error: " << ex.what() << std::endl;
    }
}
void RasterManEngine::RasterSqrt(int argc, char * argv[])
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
        return;
    }

    try
    {
        QString sRaster = GetFile(argc, argv, 2, true);
        QString sOutputRaster = GetFile(argc, argv, 3, false);
        int eResult;

        eResult = RasterManager::BasicMath(sRaster.toStdString().c_str(), NULL, NULL, RasterManager::RM_BASIC_MATH_SQRT,
                                           sOutputRaster.toStdString().c_str());

        std::cout << "\n\n" << RasterManager::GetReturnCodeAsString(eResult) << "\n";
    }
    catch (std::exception & ex)
    {
        std::cout <<"Error: " << ex.what() << std::endl;
    }
}

void RasterManEngine::Mosaic(int argc, char * argv[])
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
        return;
    }
    try
    {
        QString sInputFiles = "";
        QString rasterInputs(argv[2]);
        QString delimiterPattern(";");
        QStringList inputFileList = rasterInputs.split(delimiterPattern);

        foreach (QString sFilename, inputFileList) {
            if (sFilename.compare("") != 0)
                sInputFiles.append(GetFile(sFilename, true)).append(";");
        }

        QString sOutputRaster = GetFile(argc, argv, 3, false);
        int eResult;

        eResult = RasterManager::Mosaic(sInputFiles.toStdString().c_str(), sOutputRaster.toStdString().c_str());

        std::cout << "\n\n" << RasterManager::GetReturnCodeAsString(eResult) << "\n";
    }
    catch (std::exception & ex)
    {
        std::cout <<"Error: " << ex.what() << std::endl;
    }
}

void RasterManEngine::MakeConcurrent(int argc, char * argv[])
{
    if (argc != 4)
    {
        std::cout << "\n Make two or more rasters concurrent with each other.";
        std::cout << "\n    Usage: rasterman mosaic <raster_input_paths> <raster_output_paths> ... <output_file_path>";
        std::cout << "\n ";
        std::cout << "\n Arguments:";
        std::cout << "\n    raster_input_paths: two or more raster file paths; semicolon delimited.";
        std::cout << "\n    raster_output_paths: two or more raster file paths; semicolon delimited.";
        std::cout << "\n                         Must match raster_input_paths.";
        std::cout << "\n ";
        return;
    }
    try
    {
        QString sInputFiles = "";
        QString sOutputFiles = "";

        QString rasterInputs(argv[2]);
        QString rasterOutputs(argv[3]);

        QString delimiterPattern(";");

        QStringList inputFileList = rasterInputs.split(delimiterPattern);
        QStringList outputFileList = rasterOutputs.split(delimiterPattern);

        foreach (QString sFilename, inputFileList) {
            if (sFilename.compare("") != 0)
                sInputFiles.append(GetFile(sFilename, true)).append(";");
        }
        foreach (QString sFilename, outputFileList) {
            if (sFilename.compare("") != 0)
                sOutputFiles.append(GetFile(sFilename, false)).append(";");
        }

        int eResult;

        eResult = RasterManager::MakeConcurrent(sInputFiles.toStdString().c_str(), sOutputFiles.toStdString().c_str());

        std::cout << "\n\n" << RasterManager::GetReturnCodeAsString(eResult) << "\n";
    }
    catch (std::exception & ex)
    {
        std::cout <<"Error: " << ex.what() << std::endl;
    }
}


void RasterManEngine::Mask(int argc, char * argv[])
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
        return;
    }
    try
    {
        QString sRasterInput = GetFile(argc, argv, 2, true);
        QString sRasterMask = GetFile(argc, argv, 3, true);
        QString sOutputRaster = GetFile(argc, argv, 4, false);
        int eResult;

        eResult =  RasterManager::Mask(sRasterInput.toStdString().c_str(),
                                       sRasterMask.toStdString().c_str(),
                                       sOutputRaster.toStdString().c_str());

        std::cout << "\n\n" <<  RasterManager::GetReturnCodeAsString(eResult) << "\n";
    }
    catch (std::exception & ex)
    {
        std::cout <<"Error: " << ex.what() << std::endl;
    }
}


void RasterManEngine::Slope(int argc, char * argv[])
{
    if (argc != 5)
    {
        std::cout << "\n Bilinear Resample Usage:";
        std::cout << "\n    Syntax: rasterman slope <raster_file_path> <output_file_path> <slope_type>";
        std::cout << "\n   Command: slope";
        std::cout << "\n";
        std::cout << "\n Arguments:";
        std::cout << "\n    raster_file_path: Absolute full path to existing raster file.";
        std::cout << "\n    output_file_path: Absolute full path to output, slope raster.";
        std::cout << "\n    slope_type: 0=degrees, 1= percent.";
        std::cout << "\n";
        return;
    }

    try
    {
        QString sOriginal = GetFile(argc, argv, 2, true);
        QString sOutput = GetFile(argc, argv, 3, false);

        int nSlopeType = GetInteger(argc, argv, 4);

        RasterManager::Raster rOriginal(sOriginal.toStdString().c_str());
        int eResult = rOriginal.Slope(sOutput.toStdString().c_str(), nSlopeType);

        std::cout << "\n" << RasterManager::GetReturnCodeAsString(eResult);
    }
    catch (std::exception & ex)
    {
        std::cout <<"Error: " << ex.what() << std::endl;
    }
}


void RasterManEngine::Hillshade(int argc, char * argv[])
{
    if (argc != 4)
    {
        std::cout << "\n Create Hillshade:";
        std::cout << "\n    Syntax: rasterman hillshade <raster_file_path> <output_file_path>";
        std::cout << "\n   Command: bilinear";
        std::cout << "\n";
        std::cout << "\n Arguments:";
        std::cout << "\n    raster_file_path: Absolute full path to existing raster file.";
        std::cout << "\n    output_file_path: Absolute full path to output, hillshade raster file.";
        std::cout << "\n";
        return;
    }

    try
    {
        QString sOriginal = GetFile(argc, argv, 2, true);
        QString sOutput = GetFile(argc, argv, 3, false);

        int nSlopeType = GetInteger(argc, argv, 4);

        RasterManager::Raster rOriginal(sOriginal.toStdString().c_str());
        int eResult = rOriginal.Hillshade(sOutput.toStdString().c_str());

        std::cout << "\n" << RasterManager::GetReturnCodeAsString(eResult);
    }
    catch (std::exception & ex)
    {
        std::cout <<"Error: " << ex.what() << std::endl;
    }
}


void RasterManEngine::CSVToRaster(int argc, char * argv[])
{
    if (argc < 8)
    {
        std::cout << "\n Convert a CSV file into a raster.";
        std::cout << "\n    Usage: gcd power <csv_file_path> <output_file_path> <XField> <YField> <DataField> [<top> <left> <rows> <cols> <cell_size> <no_data_val> <EPSG_Proj>] | <csv_meta_file_path>";
        std::cout << "\n ";
        std::cout << "\n Arguments:";
        std::cout << "\n    csv_file_path: Absolute full path to existing .csv file.";
        std::cout << "\n    output_file_path: Absolute full path to desired output raster file.";

        std::cout << "\n    XField: Name of the field to use for the x values";
        std::cout << "\n    YField: Name of the field to use for the x values";
        std::cout << "\n    DataField: Name of the field to use for the raster data.";


        std::cout << "\n    top: Top coordinate of the output raster extent.";
        std::cout << "\n    left: Left coordinate of the output raster extent.";
        std::cout << "\n    rows: Number of rows in the output raster.";
        std::cout << "\n    cols: Number of columns in the output raster.";
        std::cout << "\n    cell_size: Cell size for the output raster.";
        std::cout << "\n    no_data_val: number to use for no data val (use \"min\" for minimum float)";
        std::cout << "\n    EPSG_Proj: EPSG Coordinate projection (integer).";

        std::cout << "\n    output_file_path: (optional) csv file with single line:";
        std::cout << "\n       Structure:  top, left, rows, cols, cell_size, EPSG_Proj";

        std::cout << "\n ";
        return;
    }

    QString sCSVDataFile = GetFile(argc, argv, 2, true);
    QString sOutput = GetFile(argc, argv, 3, false);

    QString sXField = argv[4];
    QString sYField = argv[5];
    QString sDataField = argv[6];


    // Either all
    if (argc == 14){

        double dLeft, dTop, dCellSize, dNoDataVal;
        int nRows, nCols;
        int nEPSGproj;

        GetOutputRasterProperties(dTop, dLeft, nRows, nCols, dCellSize, argc, argv, 7);

        nEPSGproj = GetInteger(argc, argv, 13);

        QString sNoDataVal = argv[12];
        if (sNoDataVal.compare("min", Qt::CaseInsensitive) == 0)
            dNoDataVal = (double) std::numeric_limits<float>::lowest();
        else
            dNoDataVal = sNoDataVal.toDouble();

        RasterManager::Raster::CSVtoRaster(sCSVDataFile.toStdString().c_str(),
                                           sOutput.toStdString().c_str(),
                                           dTop, dLeft, nRows, nCols,
                                           dCellSize, dNoDataVal,
                                           nEPSGproj,
                                           sXField.toStdString().c_str(),
                                           sYField.toStdString().c_str(),
                                           sDataField.toStdString().c_str() );
    }
    // Otherwise a csv file is used
    else if (argc == 8){
        QString sCSVMetaFile = GetFile(argc, argv, 7, true);
        RasterManager::Raster::CSVtoRaster(sCSVDataFile.toStdString().c_str(),
                                           sOutput.toStdString().c_str(),
                                           sCSVMetaFile.toStdString().c_str(),
                                           sXField.toStdString().c_str(),
                                           sYField.toStdString().c_str(),
                                           sDataField.toStdString().c_str() );
    }
}

QString RasterManEngine::GetFile(int argc, char * argv[], int nIndex, bool bMustExist)
{
    QString sFile;
    \
    if (nIndex < argc)
    {
        // Enough arguments
        sFile = argv[nIndex];

        if (sFile.isNull() || sFile.isEmpty())
            throw std::runtime_error("Command line missing a file path.");
        else
        {
            GetFile(sFile, bMustExist);
        }
    }
    else
        throw std::runtime_error("Insufficient command line arguments for operation.");

    return sFile;
}

QString RasterManEngine::GetFile(QString sFile, bool bMustExist)
{
    // Check if the directory the file exists in is actually there
    QDir sFilePath = QFileInfo(sFile).absoluteDir();
    if (!sFilePath.exists()){
        throw  std::runtime_error("The directory of the file you specified does not exist.");
    }

    sFile = sFile.trimmed();
    sFile = sFile.replace("\"","");
    if (bMustExist)
    {
        if (!QFile::exists(sFile))
            throw  std::runtime_error("The specified input file does not exist.");
    }
    else
        if (QFile::exists(sFile))
            throw std::runtime_error("The specified output file already exists.");

    return sFile;
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
