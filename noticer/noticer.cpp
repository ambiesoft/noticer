// dater.cpp
//

#include "stdafx.h"

#include "resource.h"

#pragma comment(lib, "Shlwapi.lib")

using namespace std;
using namespace Ambiesoft;
using namespace Ambiesoft::stdosd;

static LPCTSTR pDefaultFormat = L"%x (%a) %X";

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

wstring getDate(const wstring& format)
{
	TCHAR buff[256]; buff[0] = 0;
	time_t now = time(NULL);
	struct tm tmnow;
	localtime_s(&tmnow, &now);

	_tcsftime(buff, _countof(buff), format.empty() ? pDefaultFormat : format.c_str(), &tmnow);
	return buff;
}
int APIENTRY _tWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR     lpCmdLine,
	int       nCmdShow)
{
	Ambiesoft::InitHighDPISupport();
	Ambiesoft::i18nInitLangmap();

	tstring localestring;
	tstring format;
	bool isHelp = false;
	wstring countString;
	tstring message;
	wstring windowposString;

	bool isViewWindow = false;
	bool isViewBalloon = false;
	bool isViewClipboard = false;
	
	bool isKeepTopMost = false;

	CCommandLineParser parser(I18N(L"Shows notification window"));
	
	parser.AddOption(L"/locale", 1, &localestring,
		ArgEncodingFlags::ArgEncodingFlags_Default, I18N(L"Local to set"));
	
	parser.AddOption(L"/format", 1, &format,
		ArgEncodingFlags::ArgEncodingFlags_Default, I18N(L"time format"));
	
	parser.AddOption(L"/view=window", 0, &isViewWindow,
		ArgEncodingFlags::ArgEncodingFlags_Default, I18N(L"View in Window"));
	parser.AddOption(L"/view=balloon", 0, &isViewBalloon,
		ArgEncodingFlags::ArgEncodingFlags_Default, I18N(L"View in Balloon"));
	parser.AddOption(L"/view=clipboard", 0, &isViewClipboard,
		ArgEncodingFlags::ArgEncodingFlags_Default, I18N(L"View in Clipboard"));

	parser.AddOptionRange({ L"/h", L"/?" }, 0, &isHelp,
		ArgEncodingFlags::ArgEncodingFlags_Default, I18N(L"shows help"));
	
	parser.AddOption(L"/count", 1, &countString,
		ArgEncodingFlags::ArgEncodingFlags_Default, I18N(L"seconds to close window, or 'inf' to not close"));

	parser.AddOption(L"/message", 1, &message,
		ArgEncodingFlags::ArgEncodingFlags_Default, I18N(L"Message to show when the subcommand is 'message'"));

	parser.AddOption(L"/windowpos", 1, &windowposString,
		ArgEncodingFlags::ArgEncodingFlags_Default,
		I18N(L"Specify a Window position. The value can be one of 'topleft', 'topright', 'bottomleft', 'bottomright', 'centerscreen'"));

	parser.AddOption(L"/keeptop", 0, &isKeepTopMost,
		ArgEncodingFlags::ArgEncodingFlags_Default,
		I18N(L"Periodically set the window TopMost"));

	COption mainArg(L"",
		ArgCount::ArgCount_ZeroToInfinite,
		ArgEncodingFlags::ArgEncodingFlags_Default,
		I18N(L"subcommand: one of 'date', 'datej', 'showclip', 'desktopfilesize' or 'message'."));
	parser.AddOption(&mainArg);
	
	parser.Parse();

	if (isHelp)
	{
		MessageBox(NULL,
			parser.getHelpMessage().c_str(),
			APPNAME,
			MB_ICONINFORMATION);
		return 0;
	}
	if (parser.hadUnknownOption())
	{
		wstringstream msg;
		msg << I18N(L"Unknow Option:") << endl << endl;
		msg << parser.getUnknowOptionStrings();
		ErrorQuit(msg.str().c_str());
	}

	int userCount = 0;
	if (countString == L"inf")
		userCount = -1;
	else
		userCount = _wtoi(countString.c_str());

	// TODO: must be single
	vector<tstring> subcommands;
	for (size_t i = 0; i < mainArg.getValueCount(); ++i)
		subcommands.push_back(mainArg.getValue(i));
	
	if (subcommands.empty())
	{
		tstring msg;
		msg += I18N(L"Subcommand is empty");
		msg += L"\r\n\r\n";
		msg += parser.getHelpMessage();
		ErrorQuit(msg);
	}

	if (!isViewWindow && !isViewBalloon && !isViewClipboard)
		isViewWindow = true;

	const int actualCount = userCount == 0 ? 10 : userCount;

	_tsetlocale(LC_ALL, localestring.c_str());
	vector<thread> threads;
	for (auto&& subcommand : subcommands)
	{
		tstring outmessage;
		if (false) {}
		else if (subcommand == L"showclip")
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
			outmessage = getDate(format);
		}
		else if (subcommand == L"datej")
		{
			// Japanese date
			SYSTEMTIME loctime;
			GetLocalTime(&loctime);
			WCHAR dateStr[128];
			if (0 == GetDateFormat(
				MAKELCID(LANG_JAPANESE, SORT_DEFAULT),
				DATE_LONGDATE | DATE_USE_ALT_CALENDAR,
				&loctime,
				NULL,
				dateStr,
				_countof(dateStr)))
			{
				ErrorQuit(GetLastErrorString(GetLastError()));
			}
			WCHAR timeStr[128];
			if (0 == GetTimeFormat(
				MAKELCID(LANG_JAPANESE, SORT_DEFAULT),
				0, //TIME_NOSECONDS,
				&loctime,
				nullptr, //L"format",
				timeStr,
				_countof(timeStr)))
			{
				ErrorQuit(GetLastErrorString(GetLastError()));
			}
			wstringstream wss;
			wss << getDate(format) << L"\r\n" << dateStr << L" " << timeStr;
			outmessage = wss.str();
		}
		else if (subcommand == L"desktopfilesize")
		{
			tstring desktopdir;
			desktopdir.reserve(MAX_PATH);
			if (!SHGetSpecialFolderPath(HWND_DESKTOP, &desktopdir[0], CSIDL_DESKTOPDIRECTORY, FALSE))
			{
				ErrorQuit(tstring(I18N(L"Failed to get Desktop path")));
			}

			FILESINFOW fis;
			if (!GetFilesInfoW(desktopdir.c_str(), fis))
			{
				ErrorQuit(tstring(I18N(L"Failed to get file information")));
			}
			fis.Sort();
			tstring result;
			for (size_t i = 0; i < fis.GetCount(); ++i)
			{
				tstring file = fis[i].cFileName;

				LARGE_INTEGER li;
				li.HighPart = fis[i].nFileSizeHigh;
				li.LowPart = fis[i].nFileSizeLow;
				result += stdFormat(L"%s: %llu\r\n", file.c_str(), li.QuadPart);
			}

			outmessage = result;
		}
		else if (subcommand == L"message")
		{
			outmessage += message;
		}
		else
		{
			ErrorQuit(tstring(I18N(L"Unknown subcommand")) + L": '" + subcommand + L"'");
		}

		TIMEDMESSAGEBOX_POSITION windowpos = TIMEDMESSAGEBOX_POSITION_BOTTOMRIGHT;
		if (!windowposString.empty())
		{
			if (false) {}
			else if (windowposString == L"topleft")
			{
				windowpos = TIMEDMESSAGEBOX_POSITION_TOPLEFT;
			}
			else if (windowposString == L"topright")
			{
				windowpos = TIMEDMESSAGEBOX_POSITION_TOPRIGHT;
			}
			else if (windowposString == L"bottomleft")
			{
				windowpos = TIMEDMESSAGEBOX_POSITION_BOTTOMLEFT;
			}
			else if (windowposString == L"bottomright")
			{
				windowpos = TIMEDMESSAGEBOX_POSITION_BOTTOMRIGHT;
			}

			else if (windowposString == L"centerscreen")
			{
				windowpos = TIMEDMESSAGEBOX_POSITION_CENTERSCREEN;
			}
			else
			{
				ErrorQuit(tstring(I18N(L"Unkown window position:") + windowposString));
			}
		}

		const tstring title = subcommand + L" | " + APPNAME + L" | " + stdGetComputerName();
		
		if(isViewBalloon)
		{
			const int millisec = (actualCount > 0) ? actualCount * 1000 : INT_MAX;
			threads.push_back(
				thread([](int millisec, tstring title, tstring outmessage)
				{
					tstring strarg = stdFormat(L"/title:%s /icon:\"%s\" /duration %d /balloonicon:1 \"%s\"",
						UrlEncodeStd(title.c_str()).c_str(),
						stdGetModuleFileName().c_str(),
						millisec,
						UrlEncodeStd(outmessage.c_str()).c_str()
					);

					wstring balloonexe = stdCombinePath(
						stdGetParentDirectory(stdGetModuleFileName()),
						L"showballoon.exe");

					HANDLE hProcess = NULL;
					if (!OpenCommon(NULL, balloonexe.c_str(), strarg.c_str(), NULL, &hProcess))
					{
						return;
					}

					WaitForSingleObject(hProcess, millisec);
					CloseHandle(hProcess);
				},
				// arg
				millisec, title, outmessage
			));
		}
		if(isViewWindow)
		{
			threads.push_back(thread([](int actualCount, tstring title, tstring outmessage,
				TIMEDMESSAGEBOX_POSITION windowpos, bool isKeepTop)
				{
					HMODULE hModule = LoadLibrary(L"TimedMessageBox.dll");
					if (!hModule)
						ErrorQuit(I18N(L"Failed to load TimedMessageBox.dll"));

					FNTimedMessageBox2 func2 = NULL;
					func2 = (FNTimedMessageBox2)GetProcAddress(hModule, "fnTimedMessageBox2");
					if (!func2)
						ErrorQuit(I18N(L"GetProcAddress failed"));

					TIMEDMESSAGEBOX_PARAMS tp;
					tp.size = sizeof(tp);
					tp.flags =
						TIMEDMESSAGEBOX_FLAGS_POSITION |
						TIMEDMESSAGEBOX_FLAGS_SHOWCMD |
						TIMEDMESSAGEBOX_FLAGS_TOPMOST |
						TIMEDMESSAGEBOX_FLAGS_HIDERETRY;
					tp.flags |= (isKeepTop ? TIMEDMESSAGEBOX_FLAGS_KEEPTOPMOST : 0);
					tp.hWndCenterParent = NULL;
					tp.position = windowpos;
					tp.nShowCmd = SW_SHOWNOACTIVATE;

					func2(NULL, actualCount, title.c_str(), outmessage.c_str(), &tp);
				},
				actualCount, title, outmessage, windowpos, isKeepTopMost));
		}
		if(isViewClipboard)
		{
			if(!SetClipboardText(nullptr,outmessage.c_str()))
			{
				ErrorQuit(stdFormat(I18N(L"Failed to set to the clipboard. (%s)"),
					GetLastErrorString(GetLastError()).c_str()));
			}
		}
	} // for subcommands

	if(actualCount >= 0)
		this_thread::sleep_for(std::chrono::seconds(actualCount));
	for (auto&& th : threads)
		th.join();

	return 0;
}


