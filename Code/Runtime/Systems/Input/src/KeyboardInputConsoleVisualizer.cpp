#include "TestVisualizers/KeyboardInputConsoleVisualizer.h"

#include <iostream>

namespace CKE {
	void KeyboardInputVisualizer::Update(Keyboard const* keyboard, KeyCode const key) {
		if (keyboard->GetKeyPressed(key)) {
			std::cout << "P";
		}
		else if (keyboard->GetKeyHeld(key)) {
			std::cout << "H";
		}
		else if (keyboard->GetKeyReleased(key)) {
			std::cout << "R";
		}
		else {
			std::cout << "O";
		}
	}
}
