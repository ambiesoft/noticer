#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN

#include <windows.h>


#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <ctime>


#include "C:\\Linkout\\CommonDLL\\TimedMessageBox.h"

#include "../../lsMisc/tstring.h"
#include "../../lsMisc/UTF16toUTF8.h"
#include "../../lsMisc/UrlEncode.h"
#include "../../lsMisc/OpenCommon.h"
#include "../../lsMisc/stdwin32/stdwin32.h"
#include "../../lsMisc/stdosd/stdosd.h"
#include "../../lsMisc/CommandLineParser.h"
#include "../../lsMisc/GetClipboardText.h"
#include "../../lsMisc/GetLastErrorString.h"

#define APPNAME L"noticer"
#define APPVERSION L"1.0.3"
#define I18N(s) s