#pragma once

struct Connection {
    int startComponent;
    int endComponent;
    int startX, startY;
    int endX, endY;

    // Konstruktor przyjmuj¹cy indeksy komponentów i wspó³rzêdne
    Connection(int sComp, int eComp, int sX, int sY, int eX, int eY)
        : startComponent(sComp), endComponent(eComp), startX(sX), startY(sY), endX(eX), endY(eY) {}
};
