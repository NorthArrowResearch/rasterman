#ifndef RASTER2PNG_EXCEPTION_H
#define RASTER2PNG_EXCEPTION_H

#include <QString>
#include "exception"

namespace Raster2PNG {

enum Raster2PNGOutputCodes {
    PROCESS_OK,
};
class Raster2PNGException :public std::exception
{
public:
    inline Raster2PNGException(int nErrorCode){ Init(nErrorCode, "");}
    inline Raster2PNGException(int nErrorCode, QString sMsg){ Init(nErrorCode, sMsg); }
    inline ~Raster2PNGException()throw(){}
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

        default:
            std::string errMsg = QString("Unhandled Raster Manager return code: ").arg(eErrorCode).toStdString();
            return errMsg.c_str();
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

private:
    int m_nErrorCode;
    QString m_sEvidence;
};

}

#endif // RASTER2PNG_EXCEPTION_H
