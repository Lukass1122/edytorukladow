#pragma once
#include <vector>
#include <memory>
#include "Connection.h"
#include "Component.h"

class Circuit {
    std::vector<std::shared_ptr<Component>> components;
    std::vector<Connection> connections;

public:
    void addComponent(std::shared_ptr<Component> component) {
        components.push_back(component);
    }

    int getComponentCount() const {
        return static_cast<int>(components.size());
    }

    std::shared_ptr<Component> getComponent(int index) {
        if (index >= 0 && index < static_cast<int>(components.size())) {
            return components[index];
        }
        return nullptr;
    }

    void addConnection(const Connection& connection) {
        connections.push_back(connection);
    }

    const std::vector<Connection>& getConnections() const {
        return connections;
    }

    void removeComponent(int index) {
        if (index >= 0 && index < static_cast<int>(components.size())) {
            components.erase(components.begin() + index);
            // Usuñ równie¿ wszystkie po³¹czenia zwi¹zane z tym komponentem
            connections.erase(std::remove_if(connections.begin(), connections.end(),
                [index](const Connection& conn) {
                    return conn.startComponent == index || conn.endComponent == index;
                }), connections.end());
        }
    }

    void removeConnection(int index) {
        if (index >= 0 && index < static_cast<int>(connections.size())) {
            connections.erase(connections.begin() + index);
        }
    }

    void draw(HDC hdc) {
        // Rysowanie komponentów
        for (const auto& component : components) {
            component->draw(hdc);
        }

        // Rysowanie po³¹czeñ
        for (const auto& connection : connections) {
            MoveToEx(hdc, connection.startX, connection.startY, NULL);
            LineTo(hdc, connection.endX, connection.endY);
        }
    }

    void updateConnections(int movedComponentIndex) {
        // Zaktualizuj pozycje po³¹czeñ, jeœli komponent jest przenoszony
        for (auto& conn : connections) {
            if (conn.startComponent == movedComponentIndex) {
                auto component = components[conn.startComponent];
                POINT circlePos = component->getCirclePosition();
                conn.startX = circlePos.x;
                conn.startY = circlePos.y;
            }
            if (conn.endComponent == movedComponentIndex) {
                auto component = components[conn.endComponent];
                POINT circlePos = component->getCirclePosition();
                conn.endX = circlePos.x;
                conn.endY = circlePos.y;
            }
        }
    }
};
