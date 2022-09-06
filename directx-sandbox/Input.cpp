#include "Input.h"

Input::Input() {
}

Input::Input(const Input& other) {
}

Input::~Input() {
}

void Input::Init() {
	// Init all keys to cleared state
	for (int i = 0; i < 256; i++)
	{
		this->Keys[i] = false;
	}
}


void Input::KeyDown(unsigned int input) {
	this->Keys[input] = true;
}


void Input::KeyUp(unsigned int input) {
	this->Keys[input] = false;
	this->Keys[input] = false;
}


bool Input::IsKeyDown(unsigned int key) {
	return this->Keys[key];
}