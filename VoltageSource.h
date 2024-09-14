#pragma once
#include "Component.h"
#include <windows.h>

class VoltageSource : public Component {
public:
    VoltageSource(double voltage, const std::string& identifier)
        : Component("VoltageSource", voltage, identifier) {}

    void draw(HDC hdc) override {
        XFORM xForm;
        SetGraphicsMode(hdc, GM_ADVANCED);
        GetWorldTransform(hdc, &xForm);

        XFORM translateForm = { 0 };
        translateForm.eM11 = 1.0f;
        translateForm.eM12 = 0.0f;
        translateForm.eM21 = 0.0f;
        translateForm.eM22 = 1.0f;
        translateForm.eDx = static_cast<float>(x);
        translateForm.eDy = static_cast<float>(y);
        ModifyWorldTransform(hdc, &translateForm, MWT_LEFTMULTIPLY);

        XFORM rotateComponent = { 0 };
        rotateComponent.eM11 = cos(rotation * 3.14159 / 180);
        rotateComponent.eM12 = sin(rotation * 3.14159 / 180);
        rotateComponent.eM21 = -sin(rotation * 3.14159 / 180);
        rotateComponent.eM22 = cos(rotation * 3.14159 / 180);
        rotateComponent.eDx = 0.0f;
        rotateComponent.eDy = 0.0f;
        ModifyWorldTransform(hdc, &rotateComponent, MWT_LEFTMULTIPLY);

        Rectangle(hdc, -30, -15, 30, 15);
        TextOutA(hdc, -10, -5, identifier.c_str(), identifier.length());
        Ellipse(hdc, -5, -25, 5, -15);

        SetWorldTransform(hdc, &xForm);
    }

    double calculateCurrent(double resistance) override {
        return 0.0;
    }
};
