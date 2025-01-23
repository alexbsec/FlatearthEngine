#include "Platform.hpp"

#if FEPLATFORM_WINDOWS

#include "Core/Logger.hpp"

#include <stdexcept>
#include <windows.h>
#include <windowsx.h>

struct InternalState {
  HINSTANCE hInstance;
  HWND hwnd;
};

static float64 clockFrequency;
static LARGE_INTEGER startTime;

LRESULT CALLBACK win32ProcessMessage(HWND hwnd, uint32 msg, WPARAM wParam,
                                     LPARAM lParam);

namespace flatearth {
namespace platform {

Platform::Platform(const string &applicationName, sint32 x, sint32 y,
                   sint32 width, sint32 height)
    : _x_pos(x), _y_pos(y), _width(width), _height(height) {
  _state = new PlatformState();
  _state->internalState = make_unique_void<InternalState>(new InternalState());
  InternalState *inStatePtr =
      get_unique_void_ptr<InternalState>(_state->internalState);

  inStatePtr->hInstance = GetModuleHandleA(0);

  HICON icon = LoadIcon(inStatePtr->hInstance, IDI_APPLICATION);
  WNDCLASSA wca;
  memset(&wca, 0, sizeof(wca));
  wca.style = CS_DBLCLKS;
  wca.lpfnWndProc = win32ProcessMessage;
  wca.cbClsExtra = 0;
  wca.cbWndExtra = 0;
  wca.hInstance = inStatePtr->hInstance;
  wca.hIcon = icon;
  wca.hCursor = LoadCursor(NULL, IDC_ARROW);
  wca.hbrBackground = nullptr;
  wca.lpszClassName = "flatearth_window_class";

  // In case of the window registration fails
  if (!RegisterClass(&wca)) {
    MessageBoxA(0, "Window registration failed", "Error",
                MB_ICONEXCLAMATION | MB_OK);
    FFATAL("Window registration failed.");
    throw std::runtime_error("Platform: window registration failed.");
  }

  // Create window
  uint32 clientX = _x_pos;
  uint32 clientY = _y_pos;
  uint32 clientWidth = _width;
  uint32 clientHeight = _height;

  uint32 windowX = clientX;
  uint32 windowY = clientY;
  uint32 windowWidth = clientWidth;
  uint32 windowHeight = clientHeight;

  uint32 windowStyle = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
  uint32 windowExStyle = WS_EX_APPWINDOW;

  windowStyle |= WS_MAXIMIZEBOX;
  windowStyle |= WS_MINIMIZEBOX;
  windowStyle |= WS_THICKFRAME;

  RECT borderRect = {0, 0, 0, 0};
  AdjustWindowRectEx(&borderRect, windowStyle, 0, windowExStyle);

  windowWidth += borderRect.right - borderRect.left;
  windowHeight += borderRect.bottom - borderRect.top;

  HWND handle = CreateWindowExA(windowExStyle, "flatearth_window_class",
                                applicationName.c_str(), windowStyle, windowX,
                                windowY, windowWidth, windowHeight, 0, 0,
                                inStatePtr->hInstance, 0);

  // Check handle
  if (handle == 0) {
    MessageBoxA(0, "Window creation failed", "Error",
                MB_ICONEXCLAMATION | MB_OK);
    FFATAL("Platform: window creation failed");
    throw std::runtime_error("Window creation failed");
  }

  inStatePtr->hwnd = handle;

  // TODO: this should be false, as the window should not accept input
  bool shouldActivate = FeTrue;
  sint32 showWindowCommandFlags = shouldActivate ? SW_SHOW : SW_SHOWNOACTIVATE;
  ShowWindow(inStatePtr->hwnd, showWindowCommandFlags);

  // Clock setup
  LARGE_INTEGER freq;
  QueryPerformanceFrequency(&freq);
  clockFrequency = 1.0 / (float64)freq.QuadPart;
  QueryPerformanceCounter(&startTime);
}

Platform::~Platform() {
  // Cold cast state
  InternalState *inStatePtr =
      get_unique_void_ptr<InternalState>(_state->internalState);

  if (inStatePtr->hwnd) {
    DestroyWindow(inStatePtr->hwnd);
    inStatePtr->hwnd = 0;
  }

  delete _state;
}

bool Platform::PollEvents() {
  MSG message;
  while (PeekMessageA(&message, nullptr, 0, 0, PM_REMOVE)) {
    TranslateMessage(&message);
    DispatchMessageA(&message);
  }

  return FeFalse;
}

void Platform::ConsoleWrite(const string &message, uchar color) {
  HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
  // FATAL, ERROR, WARN, INFO, DEBUG, TRACE
  static uchar levels[6] = {64, 4, 6, 2, 1, 8};
  string sOut = message + "\n";
  SetConsoleTextAttribute(consoleHandle, levels[color]);
  OutputDebugStringA(sOut.c_str());
  uint64 length = (uint64)sOut.length();
  LPDWORD numberWritten = 0;
  WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), sOut.c_str(), (DWORD)length,
                numberWritten, 0);
}

void Platform::ConsoleError(const string &message, uchar color) {
  HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
  // FATAL, ERROR, WARN, INFO, DEBUG, TRACE
  static uchar levels[6] = {64, 4, 6, 2, 1, 8};
  string sOut = message + "\n";
  SetConsoleTextAttribute(consoleHandle, levels[color]);
  OutputDebugStringA(sOut.c_str());
  uint64 length = (uint64)sOut.length();
  LPDWORD numberWritten = 0;
  WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), sOut.c_str(), (DWORD)length,
                numberWritten, 0);
}

} // namespace platform
} // namespace flatearth

LRESULT CALLBACK win32ProcessMessage(HWND hwnd, uint32 msg, WPARAM wParam,
                                     LPARAM lParam) {
  switch (msg) {
  case WM_ERASEBKGND:
    // Notify the OS that erasing will be handled by the application to prevent
    // flicker
    return 1;

  case WM_CLOSE:
    // TODO: fire event to quit application
    return 0;

  case WM_DESTROY:
    PostQuitMessage(0);
    return 0;

  case WM_SIZE: {
    // Get the updated size
    // RECT r;
    // GetClientRect(hwnd, &r);
    // u32 width = r.right - r.left;
    // u32 height = r.bottom - r.top;
    // TODO: Fire event to resize window
  } break;

  case WM_KEYDOWN:
  case WM_SYSKEYDOWN:
  case WM_KEYUP:
  case WM_SYSKEYUP: {
    // b8 pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);
    //  TODO: input processing
  } break;

  case WM_MOUSEMOVE: {
    // i32 x_pos = GET_X_LPARAM(l_param);
    // i32 y_pos = GET_Y_LPARAM(l_param);
    //  TODO: input processing
  } break;

  case WM_MOUSEHWHEEL: {
    // i32 z_delta = GET_WHEEL_DELTA_WPARAM(w_param);
    // if (z_delta != 0) {
    //   // Flatten the input to OS independent
    //   z_delta = (z_delta < 0) ? -1 : 1;
    //   // TODO: input processing
  } break;

  case WM_LBUTTONDOWN:
  case WM_MBUTTONDOWN:
  case WM_RBUTTONDOWN:
  case WM_LBUTTONUP:
  case WM_MBUTTONUP:
  case WM_RBUTTONUP: {
    // b8 pressed = (msg == WM_LBUTTONDOWN || msg == WM_MBUTTONDOWN || msg ==
    // WM_RBUTTONDOWN);
    //  TODO: input processing
  } break;
  }

  return DefWindowProcA(hwnd, msg, wParam, lParam);
}

#endif
