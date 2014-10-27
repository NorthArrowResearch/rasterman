#ifndef RMEXCEPTION_H
#define RMEXCEPTION_H

#include <stdexcept>

namespace RasterManager {

/**
 * @brief Generic raster manager exception.
 *
 * This class is currently under utilized, but is included for completeness
 * and expansion in the future.
 *
 * Note that exceptions can be thrown and caught across C++ assembly boundaries
 * but not across C boundaries. For example, other C++ code that uses the
 * Raster class (defined in raster.h) can catch RMExceptions. However, .Net
 * code that uses the pure C exported methods cannot catch and use these exceptions.
 *
 * For this reason, any methods that are exported for use by .Net code should use the
 * integer RasterManagerOutputCodes enumeration defined in rastermanager_interface.h
 */
class RMException : public std::runtime_error
{
public:
    /**
     * @brief
     *
     * @param msg
     */
    RMException(std::string const& msg) : std::runtime_error(msg) {};
};

}

#endif // RMEXCEPTION_H
