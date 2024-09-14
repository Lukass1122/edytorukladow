#pragma once
#include <string>
#include <windows.h>

class Component {
protected:
    std::string name;
    double value;
    int x, y;
    std::string identifier;
    int rotation;

public:
    Component(const std::string& name, double value, const std::string& identifier)
        : name(name), value(value), x(0), y(0), identifier(identifier), rotation(0) {}

    virtual ~Component() {}

    std::string getName() const { return name; }
    double getValue() const { return value; }
    std::string getIdentifier() const { return identifier; }

    int getX() const { return x; }
    int getY() const { return y; }
    void setPosition(int newX, int newY) {
        x = newX;
        y = newY;
    }

    int getRotation() const { return rotation; }

    void rotate() {
        rotation = (rotation + 90) % 360;
    }

    virtual POINT getCirclePosition() const {
        POINT pt;
        pt.x = x;
        pt.y = y - 20;  // Kó³eczko jest 20 pikseli nad komponentem
        return pt;
    }

    virtual void draw(HDC hdc) = 0;
    virtual double calculateCurrent(double voltage) = 0;
};
