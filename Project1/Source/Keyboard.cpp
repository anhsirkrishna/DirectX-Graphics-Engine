#include "Keyboard.h"

bool Keyboard::isKeyPressed(unsigned char keycode) const noexcept {
	return keystates[keycode];
}

Keyboard::Event Keyboard::PopKeyBuffer() noexcept {
	if (keybuffer.size() > 0) {
		Keyboard::Event e = keybuffer.front();
		keybuffer.pop();
		return e;
	}
	else {
		return Keyboard::Event();
	}
}

bool Keyboard::KeyBufferIsEmpty() const noexcept
{
	return keybuffer.empty();
}

void Keyboard::FlushKeyBuffer() noexcept {
	keybuffer = std::queue<Event>();
}

char Keyboard::PopCharBuffer() noexcept {
	if (charbuffer.size() > 0) {
		unsigned char charcode = charbuffer.front();
		charbuffer.pop();
		return charcode;
	}
	else {
		return 0;
	}
}

bool Keyboard::CharBufferIsEmpty() const noexcept {
	return charbuffer.empty();
}

void Keyboard::FlushCharBuffer() noexcept {
	charbuffer = std::queue<char>();
}

void Keyboard::Flush() noexcept {
	FlushCharBuffer();
	FlushKeyBuffer();
}

void Keyboard::EnableAutorepeat() noexcept {
	autorepeatEnabled = true;
}

void Keyboard::DisableAutorepeat() noexcept {
	autorepeatEnabled = false;
}

bool Keyboard::AutorepeatIsEnabled() const noexcept {
	return autorepeatEnabled;
}

void Keyboard::OnKeyPressed(unsigned char keycode) noexcept {
	keystates[keycode] = true;
	keybuffer.push(Keyboard::Event(Keyboard::Event::Type::Press, keycode));
	TrimBuffer(keybuffer);
}

void Keyboard::OnKeyReleased(unsigned char keycode) noexcept {
	keystates[keycode] = false;
	keybuffer.push(Keyboard::Event(Keyboard::Event::Type::Release, keycode));
	TrimBuffer(keybuffer);
}

void Keyboard::OnChar(char character) noexcept {
	charbuffer.push(character);
	TrimBuffer(charbuffer);
}

void Keyboard::ClearState() noexcept {
	keystates.reset();
}
