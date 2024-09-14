#pragma once
#include "Component.h"
#include <windows.h>

class Resistor : public Component {
public:
    Resistor(double resistance, const std::string& identifier)
        : Component("Resistor", resistance, identifier) {}

    void draw(HDC hdc) override {
        Ellipse(hdc, x - 30, y - 15, x + 30, y + 15);
        TextOutA(hdc, x - 10, y - 5, identifier.c_str(), identifier.length());
        Ellipse(hdc, x - 5, y - 25, x + 5, y - 15);  // Kó³eczko
    }

    double calculateCurrent(double voltage) override {
        return voltage / value;  // Prawo Ohma: I = U / R
    }
};
