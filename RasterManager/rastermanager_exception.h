#ifndef RASTERMANAGER_EXCEPTION_H
#define RASTERMANAGER_EXCEPTION_H

#include <QString>
#include "exception"

namespace RasterManager{

enum RasterManagerOutputCodes {
    PROCESS_OK = 0
    , INPUT_FILE_ERROR
    , INPUT_FILE_TRANSFORM_ERROR
    , INPUT_FILE_NOT_VALID

    , PATH_ERROR
    , HISTOGRAM_ERROR
    , OUTPUT_FILE_MISSING
    , OUTPUT_FILE_ERROR
    , OUTPUT_NO_DATA_ERROR
    , OUTPUT_FILE_EXT_ERROR
    , OUTPUT_UNHANDLED_DRIVER

    , CELL_SIZE_ERROR
    , LEFT_ERROR
    , TOP_ERROR
    , ROWS_ERROR
    , COLS_ERROR
    , NO_OPERATION_SPECIFIED
    , MISSING_ARGUMENT
    , ARGUMENT_VALIDATION

    , GDALVERSION
    , RASTERMAN_VERSION

    , VECTOR_LAYER_NOT_FOUND
    , VECTOR_LAYER_NOT_POLYGON
    , VECTOR_FIELD_NOT_VALID

    , RASTER_CONCURRENCY
    , RASTER_ORTHOGONAL
    , RASTER_COMPARISON

    , DIVISION_BY_ZERO

    , RM_PNG_QUALITY
    , RM_PNG_TRANSPARENCY
    , RM_PNG_LONG_AXIS
    , GDAL_ERROR
    , OTHER_ERROR = 999
};


class RasterManagerException :public std::exception
{
public:
    inline RasterManagerException(int nErrorCode){ Init(nErrorCode, "");}
    inline RasterManagerException(int nErrorCode, QString sMsg){ Init(nErrorCode, sMsg); }

    inline ~RasterManagerException() throw() {}
    /**
     * @brief init
     */
    inline void Init(int nErrorCode, QString sMsg){
        m_nErrorCode = nErrorCode;
        m_sEvidence = sMsg;
    }
    /**
     * @brief GetErrorCode
     * @return
     */
    inline int GetErrorCode(){ return m_nErrorCode; }

    // Define the error messages
    inline static const char * GetReturnCodeOnlyAsString(int eErrorCode){
        switch (eErrorCode)
        {
        case PROCESS_OK:
            return "process completed successfully";
            break;

        case INPUT_FILE_ERROR:
            return "input file error";
            break;

        case INPUT_FILE_TRANSFORM_ERROR:
            return "input raster map projection error";
            break;

        case OUTPUT_FILE_MISSING:
            return "input raster file is missing or cannot be found";
            break;

        case  OUTPUT_FILE_ERROR:
            return "output file error";
            break;

        case  OUTPUT_NO_DATA_ERROR:
            return "NoData error on output raster";
            break;

        case  OUTPUT_FILE_EXT_ERROR:
            return "Output raster file extension error";
            break;

        case OUTPUT_UNHANDLED_DRIVER:
            return "Unhandled output raster type";
            break;

        case CELL_SIZE_ERROR:
            return "Cell size error";
            break;

        case LEFT_ERROR:
            return "Invalid raster left coordinate";
            break;

        case TOP_ERROR:
            return "Invalid raster top coordinate";
            break;

        case ROWS_ERROR:
            return "Invalid raster number of rows";
            break;

        case COLS_ERROR:
            return "Invalid raster number of columns";
            break;

        case NO_OPERATION_SPECIFIED:
            return "No operation specified";
            break;

        case MISSING_ARGUMENT:
            return "Missing argument";
            break;

        case OTHER_ERROR:
            return "Unspecified error";
            break;

        case RM_PNG_QUALITY:
            return "Invalid image quality";
            break;

        case RM_PNG_TRANSPARENCY:
            return "Invalid PNG transparency";
            break;

        case RM_PNG_LONG_AXIS:
            return "Invalid Long Axis specified";
            break;

        case ARGUMENT_VALIDATION:
            return "Argument Validation Error";
            break;

        case INPUT_FILE_NOT_VALID:
            return "Input Exists but is Invalid";
            break;

        case GDAL_ERROR:
            return "GDAL Error";
            break;

        case RASTERMAN_VERSION:
            return "Rasterman executable is at a different version than its DLL";
            break;

        case PATH_ERROR:
            return "File Path Error";
            break;

        case RASTER_CONCURRENCY:
            return "Rasters are not concurrent";
            break;

        case RASTER_ORTHOGONAL:
            return "Rasters are not Orthogonal";

        case HISTOGRAM_ERROR:
            return "Error calculating histogram";

        case RASTER_COMPARISON:
            return "Rasters are not the same";

        case GDALVERSION:
            return "Insufficient GDAL version detected.";
            break;
        case DIVISION_BY_ZERO:
            return "Division by Zero.";
            break;
        default:
            return "Unhandled Raster Manager Error.";
            break;
        }
    };

    /**
     * @brief GetReturnMsgAsString: This function returns more information than GetReturnCodeOnlyAsString
     * @return
     */
    inline QString GetReturnMsgAsString(){
        QString sOutput = "";
        QString sErrMsg = GetReturnCodeOnlyAsString(m_nErrorCode);
        if (m_sEvidence.length() > 0){
            sOutput = sErrMsg + ": " + m_sEvidence;
        }
        return sOutput;
    }

    /**
     * @brief GetEvidence: Just return the evidence string
     * @return
     */
    inline QString GetEvidence(){ return m_sEvidence; }

private:
    int m_nErrorCode;
    QString m_sEvidence;
};

}
#endif // RASTERMANAGER_EXCEPTION_H
