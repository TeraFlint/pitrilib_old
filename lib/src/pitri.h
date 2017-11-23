#pragma once

#define P3_LIB_MAINVERSION 1
#define P3_LIB_SUBVERSION 0
#define P3_LIB_VERSION P3_LIB_MAINVERSION * 1000 + P3_LIB_SUBVERSION

#define P3_NAMESPACE Pitri

//platform < parameters
#include "p3/parameters.h"

//stringmanip < filenamip
#include "p3/filemanip.h"

//compointer < image
//image < imagemanip
//imagemanip < user
#include "p3/user.h"

#include "p3/application.h"

#include "p3/compression.h"

#include "p3/zip.h"