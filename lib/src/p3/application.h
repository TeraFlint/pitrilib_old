#pragma once
#include "helpers.h"
#include <Windows.h>

namespace Pitri
{
	struct CallResult
	{
		//in
		HWND hwnd;
		WPARAM lparam;
		LPARAM wparam;

		//out
		bool valid;
		LRESULT result;
	};

	class AppStateMachineBase;
	class AppStateBase;

	class ApplicationBase
	{
	protected:
		HWND window;
		HINSTANCE instance;
		AppStateMachineBase *machine;

		static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
		LRESULT MainWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
		virtual LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

	public:
		ApplicationBase();
		~ApplicationBase();

		bool Initialize(HINSTANCE hinstance);
		void Run();
		HWND GetWindow();

		virtual void CreateStateMachine() = 0;

		virtual unsigned GetWindowWidth();
		virtual unsigned GetWindowHeight();
		virtual DWORD GetWindowStyle();
		virtual std::string GetWindowTitle();
		virtual HICON GetIcon();
	};

	class AppStateMachineBase
	{
	protected:
		ApplicationBase *app;
		unsigned past, future;
		AppStateBase *current;

	public:
		AppStateMachineBase(ApplicationBase *application);

		void ChangeState(unsigned type);

		void CreateNewState();
		virtual AppStateBase *ReturnNewState();

		void WndProcTimer(CallResult *data);
		void WndProcDraw(CallResult *data);
		void WndProcCommand(CallResult *data);
	};

	class AppStateBase
	{
	protected:
		HFONT font;

	public:
		virtual ~AppStateBase();

		void Initialize(ApplicationBase *app);
		void Terminate();

		HWND CreateButton(ApplicationBase *app, std::string text, Pitri::Rect<int> rect);

		virtual unsigned GetFontHeight();
		virtual HFONT GetFont();
		virtual std::string GetName();
		virtual bool DrawFrames();

		virtual void WndProcTimer(CallResult *data, AppStateMachineBase *machine);
		virtual void WndProcDraw(CallResult *data, HDC hdc, BYTE *bitmap, RECT area);
		virtual void WndProcCommand(CallResult *data, AppStateMachineBase *machine);
	};
}