#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN
struct IUnknown; // Workaround for "combaseapi.h(229): error C2187: syntax error: 'identifier' was unexpected here" 
#include <windows.h>
#include <Shlwapi.h>
#include <ShlObj.h>


#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <ctime>
#include <thread>

#include "C:\\Linkout\\CommonDLL\\TimedMessageBox.h"

#include "../../lsMisc/tstring.h"
#include "../../lsMisc/UTF16toUTF8.h"
#include "../../lsMisc/UrlEncode.h"
#include "../../lsMisc/OpenCommon.h"
#include "../../lsMisc/stdosd/stdosd.h"
#include "../../lsMisc/CommandLineParser.h"
#include "../../lsMisc/GetClipboardText.h"
#include "../../lsMisc/GetLastErrorString.h"
#include "../../lsMisc/GetFilesInfo.h"
#include "../../lsMisc/GetFilesInfo.h"
#include "../../lsMisc/HighDPI.h"
#include "../../lsMisc/GetLastErrorString.h"
#include "../../lsMisc/SetClipboardText.h"

#define APPNAME L"noticer"
#define APPVERSION L"1.0.10"
#define I18N(s) s
