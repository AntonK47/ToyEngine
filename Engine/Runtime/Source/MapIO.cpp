#include "Editor.h"

auto mapWindowIoToImGuiIo(const toy::io::WindowIo& windowIo, ImGuiIO& io) -> void
{
	io.AddMousePosEvent(windowIo.mouseState.position.x, windowIo.mouseState.position.y);
	io.AddMouseButtonEvent(0, windowIo.mouseState.leftButton == toy::io::ButtonState::pressed);
	io.AddMouseButtonEvent(1, windowIo.mouseState.rightButton == toy::io::ButtonState::pressed);
	io.AddMouseButtonEvent(2, windowIo.mouseState.middleButton == toy::io::ButtonState::pressed);
	io.AddMouseWheelEvent(windowIo.mouseState.wheel.x, windowIo.mouseState.wheel.y);

	io.AddKeyEvent(ImGuiKey::ImGuiKey_0, windowIo.keyboardState.zero == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_1, windowIo.keyboardState.one == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_2, windowIo.keyboardState.two == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_3, windowIo.keyboardState.three == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_4, windowIo.keyboardState.four == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_5, windowIo.keyboardState.five == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_6, windowIo.keyboardState.six == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_7, windowIo.keyboardState.seven == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_8, windowIo.keyboardState.eight == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_9, windowIo.keyboardState.nine == toy::io::ButtonState::pressed);

	io.AddKeyEvent(ImGuiKey::ImGuiKey_A, windowIo.keyboardState.a == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_B, windowIo.keyboardState.b == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_C, windowIo.keyboardState.c == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_D, windowIo.keyboardState.d == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_E, windowIo.keyboardState.e == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_F, windowIo.keyboardState.f == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_G, windowIo.keyboardState.g == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_H, windowIo.keyboardState.h == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_I, windowIo.keyboardState.i == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_J, windowIo.keyboardState.j == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_K, windowIo.keyboardState.k == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_L, windowIo.keyboardState.l == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_M, windowIo.keyboardState.m == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_O, windowIo.keyboardState.o == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_P, windowIo.keyboardState.p == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_Q, windowIo.keyboardState.q == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_R, windowIo.keyboardState.r == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_S, windowIo.keyboardState.s == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_T, windowIo.keyboardState.t == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_U, windowIo.keyboardState.u == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_V, windowIo.keyboardState.v == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_W, windowIo.keyboardState.w == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_X, windowIo.keyboardState.x == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_Y, windowIo.keyboardState.y == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_Z, windowIo.keyboardState.z == toy::io::ButtonState::pressed);

	io.AddKeyEvent(ImGuiKey::ImGuiKey_Space, windowIo.keyboardState.space == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_Backspace, windowIo.keyboardState.backspace == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_Enter, windowIo.keyboardState.enter == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_LeftShift, windowIo.keyboardState.shiftLeft == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_RightShift, windowIo.keyboardState.shiftRight == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_LeftBracket, windowIo.keyboardState.bracketLeft == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_RightBracket, windowIo.keyboardState.bracketRight == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_Backslash, windowIo.keyboardState.backslash == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_LeftAlt, windowIo.keyboardState.altLeft == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_RightAlt, windowIo.keyboardState.altRight == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_Semicolon, windowIo.keyboardState.semicolon == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_Apostrophe, windowIo.keyboardState.apostrophe == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_Comma, windowIo.keyboardState.comma == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_Period, windowIo.keyboardState.period == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_Slash, windowIo.keyboardState.slash == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_GraveAccent, windowIo.keyboardState.graveAccent == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_Minus, windowIo.keyboardState.minus == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_Equal, windowIo.keyboardState.equal == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_LeftCtrl, windowIo.keyboardState.controlLeft == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_RightCtrl, windowIo.keyboardState.controlRight == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_Escape, windowIo.keyboardState.escape == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_CapsLock, windowIo.keyboardState.capsLock == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_Tab, windowIo.keyboardState.tab == toy::io::ButtonState::pressed);

	io.AddKeyEvent(ImGuiKey::ImGuiKey_F1, windowIo.keyboardState.f1 == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_F2, windowIo.keyboardState.f2 == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_F3, windowIo.keyboardState.f3 == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_F4, windowIo.keyboardState.f4 == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_F5, windowIo.keyboardState.f5 == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_F6, windowIo.keyboardState.f6 == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_F7, windowIo.keyboardState.f7 == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_F8, windowIo.keyboardState.f8 == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_F9, windowIo.keyboardState.f9 == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_F10, windowIo.keyboardState.f10 == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_F11, windowIo.keyboardState.f11 == toy::io::ButtonState::pressed);
	io.AddKeyEvent(ImGuiKey::ImGuiKey_F12, windowIo.keyboardState.f12 == toy::io::ButtonState::pressed);

	for (const auto& c : windowIo.textState.text)
	{
		io.AddInputCharacter(c);
	}
}