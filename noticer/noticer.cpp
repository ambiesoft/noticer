// dater.cpp
//

#include "stdafx.h"

#include "../../lsMisc/GetFilesInfo.h"
#include "../../lsMisc/HighDPI.h"

#include "resource.h"

#pragma comment(lib, "Shlwapi.lib")

using namespace std;
using namespace Ambiesoft::stdwin32;
using namespace Ambiesoft;
using namespace Ambiesoft::stdosd;

static LPCTSTR pDefaultFormat = _T("%x (%a) %X");


void ShowHelp()
{
	tstring msg = _T("noticer [/locale locale] [/format format] [/balloon] [/count count] subcommand\n");
	msg += _T("\n");

	msg += _T("locale: string passed to setlocale()\n");
	msg += _T("\n");

	msg += _T("format: format string passed to strftime(), default format is \"");
	msg += pDefaultFormat;
	msg += _T("\"\n");
	msg += _T("\n");

	msg += _T("/balloon: Use balloon\n");
	msg += _T("\n");

	msg += _T("count: Number of seconds to automatically close\n");
	msg += _T("\n");

	msg += _T("subcommand:\n");
	msg += _T("  clipboard: show text on the clipboard\n");
	msg += _T("  date: show date\n");
	msg += _T("  desktopfilesize: show the file sizes on Desktop\n");

	MessageBox(NULL,
		msg.c_str(),
		APPNAME L" ver " APPVERSION,
		MB_ICONINFORMATION);
}
void ErrorQuit(LPCWSTR pMessage)
{
	MessageBox(NULL,
		pMessage,
		APPNAME,
		MB_ICONERROR);

	ExitProcess(1);
}
void ErrorQuit(const tstring& message)
{
	ErrorQuit(message.c_str());
}
int APIENTRY _tWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR     lpCmdLine,
	int       nCmdShow)
{
	Ambiesoft::InitHighDPISupport();

	tstring localestring;
	tstring format;
	bool isHelp = false;
	bool isBalloon = false;
	int count = 0;
	tstring subcommand;

	CCommandLineParser parser;
	parser.AddOption(_T("/locale"), 1, &localestring);
	parser.AddOption(_T("/format"), 1, &format);
	parser.AddOption(_T("/balloon"), 0, &isBalloon);
	parser.AddOption(_T("/h"), _T("/?"), 0, &isHelp);
	parser.AddOption(_T("/count"), 1, &count);
	parser.AddOption(_T(""), 1, &subcommand);

	parser.Parse();

	if (isHelp)
	{
		ShowHelp();
		return 0;
	}
	if (parser.hadUnknownOption())
	{
		tstring msg;
		msg += I18N(_T("Unknow Option:\n\n"));
		msg += parser.getUnknowOptionStrings();
		ErrorQuit(msg.c_str());
	}

	if (subcommand.empty())
	{
		ErrorQuit(I18N(L"Subcommand is empty"));
	}

	_tsetlocale(LC_ALL, localestring.c_str());


	tstring outmessage;
	if (false) {}
	else if (subcommand == L"clipboard")
	{
		if (!IsClipboardFormatAvailable(CF_UNICODETEXT))
		{
			ErrorQuit(I18N(L"The Clipboard does not have text."));
		}
		if (!GetClipboardText(NULL, outmessage))
		{
			ErrorQuit(GetLastErrorString(GetLastError()));
		}
	}
	else if (subcommand == L"date")
	{
		TCHAR buff[256]; buff[0] = 0;
		time_t now = time(NULL);
		struct tm tmnow;
		localtime_s(&tmnow, &now);

		_tcsftime(buff, _countof(buff), format.empty() ? pDefaultFormat : format.c_str(), &tmnow);
		outmessage = buff;
	}
	else if (subcommand == L"desktopfilesize")
	{
		tstring desktopdir;
		desktopdir.reserve(MAX_PATH);
		if(!SHGetSpecialFolderPath(HWND_DESKTOP, &desktopdir[0], CSIDL_DESKTOPDIRECTORY, FALSE))
		{
			ErrorQuit(tstring(I18N(L"Failed to get Desktop path")));
		}
		
		FILESINFOW fis;
		if (!GetFilesInfoW(desktopdir.c_str(), fis))
		{
			ErrorQuit(tstring(I18N(L"Failed to get file infoes")));
		}
		fis.Sort();
		tstring result;
		for(size_t i=0 ; i < fis.GetCount(); ++i)
		{
			tstring file = fis[i].cFileName;

			LARGE_INTEGER li;
			li.HighPart = fis[i].nFileSizeHigh;
			li.LowPart = fis[i].nFileSizeLow;
			result += stdFormat(L"%s: %llu\r\n", file.c_str(), li.QuadPart);
		}

		outmessage = result;
	}
	else
	{
		ErrorQuit(tstring(I18N(L"Unknown subcommand")) + L": '" + subcommand + L"'");
	}


	count = count <= 0 ? 10 : count;
	int millisec = count * 1000;
	tstring title = subcommand + L" | " + APPNAME;
	if (isBalloon)
	{
		tstring strarg = stdFormat(_T("/title:%s /icon:\"%s\" /duration %d /balloonicon:1 \"%s\""),
			Utf8UrlEncode(title.c_str()).c_str(),
			stdGetModuleFileName().c_str(),
			millisec,
			Utf8UrlEncode(outmessage).c_str()
		);

		wstring balloonexe = stdCombinePath(
			stdGetParentDirectory(stdGetModuleFileName()),
			L"showballoon.exe");
		//	L"argCheck.exe");


		HANDLE hProcess = NULL;
		if (!OpenCommon(NULL, balloonexe.c_str(), strarg.c_str(), NULL, &hProcess))
		{
			return 1;
		}

		WaitForSingleObject(hProcess, millisec);
		CloseHandle(hProcess);
	}
	else
	{
		HMODULE hModule = LoadLibrary(L"TimedMessageBox.dll");
		if (!hModule)
			ErrorQuit(L"Failed to load TimedMessageBox.dll");

		FNTimedMessageBox2 func2 = NULL;
		func2 = (FNTimedMessageBox2)GetProcAddress(hModule, "fnTimedMessageBox2");
		if (!func2)
			ErrorQuit(L"Faied GetProcAddress");

		TIMEDMESSAGEBOX_PARAMS tp;
		tp.size = sizeof(tp);
		tp.flags = TIMEDMESSAGEBOX_FLAGS_POSITION | TIMEDMESSAGEBOX_FLAGS_SHOWCMD | TIMEDMESSAGEBOX_FLAGS_TOPMOST;
		tp.hWndCenterParent = NULL;
		tp.position = TIMEDMESSAGEBOX_POSITION_BOTTOMRIGHT;
		tp.nShowCmd = SW_SHOWNOACTIVATE;

		func2(NULL, count, title.c_str(), outmessage.c_str(), &tp);
	}
	return 0;
}


