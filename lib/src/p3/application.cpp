#include "application.h"

namespace Pitri
{
	/*
	AppEvent::AppEvent()
	{
		click = 0;
		keyboard = 0;
		pressed = 0;
		button = 0;
	}*/

	LRESULT CALLBACK ApplicationBase::StaticWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		if (msg == WM_NCCREATE)
			SetWindowLongPtr(hwnd, 0, reinterpret_cast<long>(reinterpret_cast<LPCREATESTRUCT>(lparam)->lpCreateParams));

		ApplicationBase *app = reinterpret_cast<ApplicationBase *>(GetWindowLongPtr(hwnd, 0));
		if (app)
			return app->MainWndProc(hwnd, msg, wparam, lparam);
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}
	LRESULT ApplicationBase::MainWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		CallResult data = { 0 };
		data.hwnd = hwnd;
		data.wparam = wparam;
		data.lparam = lparam;

		switch (msg)
		{
			case WM_DESTROY:
				PostQuitMessage(0);
				break;
			case WM_COMMAND:
			{
				if (machine)
					machine->WndProcCommand(&data);
				break;
			}
			case WM_PAINT:
			{
				if (machine)
					machine->WndProcDraw(&data);
				break;
			}
			case WM_GETMINMAXINFO:
			{
				MINMAXINFO *info = reinterpret_cast<MINMAXINFO *>(lparam);
				//Ugly and hardcoded values. I need a way to retreive the window border size.
				info->ptMinTrackSize.x = GetWindowWidth() + 16;
				info->ptMinTrackSize.y = GetWindowHeight() + 38;
				return 0;
			}
		}
		if (data.valid)
			return data.result;
		return WndProc(hwnd, msg, wparam, lparam);
	}
	LRESULT ApplicationBase::WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		return DefWindowProc(hwnd, msg, wparam, lparam);
	}

	ApplicationBase::ApplicationBase()
	{
		window = 0;
		instance = 0;
		machine = 0;
	}
	ApplicationBase::~ApplicationBase()
	{
		if (machine)
		{
			delete machine;
			machine = 0;
		}
	}

	bool ApplicationBase::Initialize(HINSTANCE hinstance)
	{
		CreateStateMachine();

		instance = hinstance;

		WNDCLASS wc = { 0 };
		wc.cbWndExtra = sizeof(this); //size of the pointer, not the object
		wc.hCursor = LoadCursor(0, IDC_ARROW);
		wc.hInstance = hinstance;
		wc.lpfnWndProc = StaticWndProc;
		wc.lpszClassName = "Main Window";
		wc.hbrBackground = CreateSolidBrush(0xffffff);
		wc.hIcon = GetIcon();
		RegisterClass(&wc);

		RECT rect;
		SetRect(&rect, 0, 0, GetWindowWidth(), GetWindowHeight());
		AdjustWindowRect(&rect, GetWindowStyle(), 0);
		window = CreateWindow(wc.lpszClassName, GetWindowTitle().c_str(), GetWindowStyle(), CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, 0, 0, hinstance, this);

		if (!window) return false;
		machine->ChangeState(0);
		return true;
	}
	void ApplicationBase::Run()
	{
		ShowWindow(window, SW_SHOWNORMAL);
		MSG msg;
		while (GetMessage(&msg, 0, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	HWND ApplicationBase::GetWindow()
	{
		return window;
	}

	unsigned ApplicationBase::GetWindowWidth()
	{
		return 640;
	}
	unsigned ApplicationBase::GetWindowHeight()
	{
		return 480;
	}
	DWORD ApplicationBase::GetWindowStyle()
	{
		return WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_SIZEBOX | WS_MAXIMIZEBOX;
	}
	std::string ApplicationBase::GetWindowTitle()
	{
		return "";
	}
	HICON ApplicationBase::GetIcon()
	{
		return 0;
	}


	AppStateMachineBase::AppStateMachineBase(ApplicationBase *application)
	{
		app = application;
		current = 0;
		past = future = -1;
	}

	void AppStateMachineBase::ChangeState(unsigned type)
	{
		future = type;
		if (past == -1)
		{
			past = type;
			CreateNewState();
		}
	}

	void AppStateMachineBase::CreateNewState()
	{
		if (current)
			delete current;
		current = ReturnNewState();
	}
	AppStateBase *AppStateMachineBase::ReturnNewState()
	{
		//Depending on the future variable, you want to return a new object inheriting from AppStateBase.
		return 0;
	}

	void AppStateMachineBase::WndProcTimer(CallResult *data)
	{
		if (current)
		{
			current->WndProcTimer(data, this);
			InvalidateRect(data->hwnd, 0, 0);
		}
	}
	void AppStateMachineBase::WndProcDraw(CallResult *data)
	{
		if (current && current->DrawFrames())
		{
			RECT area;
			GetClientRect(data->hwnd, &area);

			PAINTSTRUCT lp;
			HDC hdc = BeginPaint(data->hwnd, &lp);
			HDC memdc = CreateCompatibleDC(hdc);

			BYTE *bitmap;
			BITMAPINFO info;
			info.bmiHeader.biSize = sizeof(info);
			info.bmiHeader.biWidth = area.right - area.left;
			info.bmiHeader.biHeight = area.top - area.bottom; //negative value.
			info.bmiHeader.biPlanes = 1;
			info.bmiHeader.biBitCount = 32;
			info.bmiHeader.biCompression = BI_RGB;

			HBITMAP frame = CreateDIBSection(hdc, &info, DIB_RGB_COLORS, (void**)&bitmap, 0, 0);
			HGDIOBJ old = SelectObject(memdc, frame);

			int height = -MulDiv(10, GetDeviceCaps(hdc, LOGPIXELSY), 72);
			//HFONT font = CreateFont(height, 0, 0, 0, FW_DONTCARE, false, false, false, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, "Arial");
			HFONT font = current->GetFont();
			SelectObject(memdc, font);

			SetTextColor(memdc, RGB(0, 0, 0));
			SetBkMode(memdc, TRANSPARENT);

			current->WndProcDraw(data, memdc, bitmap, area);

			BitBlt(hdc, 0, 0, area.right - area.left, area.bottom - area.top, memdc, 0, 0, SRCCOPY);

			SelectObject(memdc, old);
			DeleteObject(font);
			DeleteObject(frame);
			DeleteDC(memdc);
			EndPaint(data->hwnd, &lp);
		}
	}
	void AppStateMachineBase::WndProcCommand(CallResult *data)
	{
		if (current)
			current->WndProcCommand(data, this);
		if (past != future)
		{
			CreateNewState();
			past = future;
		}
	}

	AppStateBase::~AppStateBase()
	{
	}

	void AppStateBase::Initialize(ApplicationBase *app)
	{
		std::string title = app->GetWindowTitle();
		std::string name = GetName();
		if (!title.empty())
		{
			if (!name.empty())
				title += " - " + name;
		}
		else title = name;
		SetWindowText(app->GetWindow(), title.c_str());

		InvalidateRect(app->GetWindow(), 0, 0);
		font = GetFont();
	}

	void AppStateBase::Terminate()
	{
	}

	HWND AppStateBase::CreateButton(ApplicationBase *app, std::string text, Pitri::Rect<int> rect)
	{
		HWND button = CreateWindow("BUTTON", text.c_str(), WS_CHILD | WS_VISIBLE, rect.pos.x, rect.pos.y, rect.size.x, rect.size.y, app->GetWindow(), 0, 0, 0);
		SendMessage(button, WM_SETFONT, reinterpret_cast<WPARAM>(font), 1);
		return button;
	}

	unsigned AppStateBase::GetFontHeight()
	{
		return 16;
	}
	HFONT AppStateBase::GetFont()
	{
		return CreateFont(GetFontHeight(), 0, 0, 0, FW_DONTCARE, 0, 0, 0, 0, OUT_CHARACTER_PRECIS, 0, 0, 0, "Arial");
	}
	std::string AppStateBase::GetName()
	{
		return "";
	}
	bool AppStateBase::DrawFrames()
	{
		return false;
	}


	void AppStateBase::WndProcTimer(CallResult *data, AppStateMachineBase *machine)
	{
	}
	void AppStateBase::WndProcDraw(CallResult *data, HDC hdc, BYTE *bitmap, RECT area)
	{
	}
	void AppStateBase::WndProcCommand(CallResult *data, AppStateMachineBase *machine)
	{
	}
}