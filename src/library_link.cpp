/*
 * library_link.cpp
 *
 *  Created on: Sep 13, 2013
 *      Author: kthierbach
 */

#include "library_link.hpp"

DLLEXPORT int llVersion(WolframLibraryData libData, mint nargs, MArgument* input, MArgument output)
{
#ifndef REVISION
	MArgument_setUTF8String(output, "Unknown");
#else
	MArgument_setUTF8String(output, REVISION);
#endif
	return LIBRARY_NO_ERROR;
}
