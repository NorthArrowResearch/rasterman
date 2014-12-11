#ifndef RASTERMANAGER_EXCEPTION_H
#define RASTERMANAGER_EXCEPTION_H

#include <QString>
#include "exception"

namespace RasterManager{

enum RasterManagerOutputCodes {
    PROCESS_OK = 0
    , INPUT_FILE_ERROR = 1
    , INPUT_FILE_TRANSFORM_ERROR = 2
    , INPUT_FILE_NOT_VALID = 26

    , PATH_ERROR = 27

    , OUTPUT_FILE_MISSING = 3
    , OUTPUT_FILE_ERROR = 4
    , OUTPUT_NO_DATA_ERROR = 5
    , OUTPUT_FILE_EXT_ERROR = 6
    , OUTPUT_UNHANDLED_DRIVER = 7

    , CELL_SIZE_ERROR = 8
    , LEFT_ERROR = 9
    , TOP_ERROR = 10
    , ROWS_ERROR = 11
    , COLS_ERROR = 12
    , NO_OPERATION_SPECIFIED = 13
    , MISSING_ARGUMENT = 14
    , ARGUMENT_VALIDATION = 15

    , RM_PNG_QUALITY = 22
    , RM_PNG_TRANSPARENCY = 23
    , RM_PNG_LONG_AXIS =24
    , GDAL_ERROR = 25
    , OTHER_ERROR = 999
};


class RasterManagerException :public std::exception
{
public:
    inline RasterManagerException(int nErrorCode){ Init(nErrorCode, "");}
    inline RasterManagerException(int nErrorCode, QString sMsg){ Init(nErrorCode, sMsg); }
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
            return "process completed successfully.";

        case INPUT_FILE_ERROR:
            return "input file error.";

        case INPUT_FILE_TRANSFORM_ERROR:
            return "input raster map projection error.";

        case OUTPUT_FILE_MISSING:
            return "input raster file is missing or cannot be found.";

        case  OUTPUT_FILE_ERROR:
            return "output file error";

        case  OUTPUT_NO_DATA_ERROR:
            return "NoData error on output raster";

        case  OUTPUT_FILE_EXT_ERROR:
            return "Output raster file extension error.";

        case OUTPUT_UNHANDLED_DRIVER:
            return "Unhandled output raster type.";

        case CELL_SIZE_ERROR:
            return "Cell size error.";

        case LEFT_ERROR:
            return "Invalid raster left coordinate.";

        case TOP_ERROR:
            return "Invalid raster top coordinate.";

        case ROWS_ERROR:
            return "Invalid raster number of rows.";

        case COLS_ERROR:
            return "Invalid raster number of columns.";

        case NO_OPERATION_SPECIFIED:
            return "No operation specified.";

        case MISSING_ARGUMENT:
            return "Missing argument";

        case OTHER_ERROR:
            return "Unspecified error.";

        case RM_PNG_QUALITY:
            return "Invalid image quality.";

        case RM_PNG_TRANSPARENCY:
            return "Invalid PNG transparency.";

        case RM_PNG_LONG_AXIS:
            return "Invalid Long Axis specified.";

        case ARGUMENT_VALIDATION:
            return "Argument Validation Error";

        case INPUT_FILE_NOT_VALID:
            return "Input Exists but is Invalid";

        case GDAL_ERROR:
            return "GDAL Error";

        case PATH_ERROR:
            return "File Path Error";

        default:
            std::string errMsg = "Unhandled Raster Manager return code: " + std::to_string((long long)eErrorCode);
            return errMsg.c_str();
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

private:
    int m_nErrorCode;
    QString m_sEvidence;
};

}
#endif // RASTERMANAGER_EXCEPTION_H
