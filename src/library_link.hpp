/*
 * library_link.h
 *
 *  Created on: Sep 13, 2013
 *      Author: kthierbach
 */

#ifndef LIBRARY_LINK_HPP_
#define LIBRARY_LINK_HPP_

#include "templates/image.hpp"
#include "mathlink.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "WolframLibrary.h"

#ifdef __cplusplus
extern "C"
{
#endif

DLLEXPORT int llGraphCut(WolframLibraryData libData, mint nargs, MArgument* input, MArgument output);
DLLEXPORT int llMultiLabelGraphcut(WolframLibraryData libData, mint nargs, MArgument* input, MArgument output);
DLLEXPORT int llDensity(WolframLibraryData libData, mint nargs, MArgument* input, MArgument output);
DLLEXPORT int llVersion(WolframLibraryData libData, mint nargs, MArgument* input, MArgument output);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LIBRARY_LINK_HPP_ */
