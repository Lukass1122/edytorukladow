#pragma once

struct Connection {
    int startComponent;
    int endComponent;
    int startX, startY;
    int endX, endY;

    // Konstruktor przyjmuj�cy indeksy komponent�w i wsp�rz�dne
    Connection(int sComp, int eComp, int sX, int sY, int eX, int eY)
        : startComponent(sComp), endComponent(eComp), startX(sX), startY(sY), endX(eX), endY(eY) {}
};
