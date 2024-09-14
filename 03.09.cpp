#include <windows.h>
#include "framework.h"
#include "03.09.h"
#include "Circuit.h"
#include "Resistor.h"
#include "Capacitor.h"
#include "Inductor.h"
#include "VoltageSource.h"
#include <list>

#define MAX_LOADSTRING 100

// Zdefiniowane identyfikatory dla przycisków i opcji
#define ID_BUTTON_RESISTOR 1001
#define ID_BUTTON_CAPACITOR 1002
#define ID_BUTTON_INDUCTOR 1003
#define ID_BUTTON_VOLTAGE 1004
#define ID_BUTTON_MOVE 1005
#define ID_MENU_DELETE 2001

// Zmienne globalne
HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];
Circuit circuit;
int resistorCount = 0;
int capacitorCount = 0;
int inductorCount = 0;
int voltageSourceCount = 0;
bool isDrawingLine = false;
int lineStartX = 0, lineStartY = 0;
int lineEndX = 0, lineEndY = 0;
int selectedComponentIndex = -1;
int draggingComponent = -1;
int lineStartComponent = -1;
int rightClickedComponentIndex = -1;
int rightClickedConnectionIndex = -1;
bool isDrawingFromLine = false;
int lineStartConnectionIndex = -1;
POINT lineStartPoint = { 0, 0 };
bool isMoveMode = false; // Flaga trybu przenoszenia

// Funkcja sprawdzająca, czy kliknięto na komponent (dowolny obszar komponentu)
bool isOnComponent(std::shared_ptr<Component> component, int x, int y) {
    return (x >= component->getX() - 30 && x <= component->getX() + 30 &&
        y >= component->getY() - 15 && y <= component->getY() + 15);
}

// Funkcja sprawdzająca, czy kliknięto na linię (połączenie)
bool isOnConnection(const Connection& conn, int x, int y) {
    // Wykorzystujemy odległość punktu od linii
    int dx = conn.endX - conn.startX;
    int dy = conn.endY - conn.startY;
    float lengthSquared = dx * dx + dy * dy;

    float t = max(0, min(1, (float)((x - conn.startX) * dx + (y - conn.startY) * dy) / lengthSquared));
    int closestX = conn.startX + t * dx;
    int closestY = conn.startY + t * dy;

    int distSquared = (x - closestX) * (x - closestX) + (y - closestY) * (y - closestY);
    return distSquared <= 100;  // Tolerancja 10 pikseli
}

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow) {
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_MY0309, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance(hInstance, nCmdShow)) {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MY0309));
    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance) {
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MY0309));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_MY0309);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow) {
    hInst = hInstance;
    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, 800, 600, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd) {
        return FALSE;
    }

    CreateWindow(L"BUTTON", L"Resistor", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        10, 10, 80, 30, hWnd, (HMENU)ID_BUTTON_RESISTOR, hInstance, NULL);
    CreateWindow(L"BUTTON", L"Capacitor", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        10, 50, 80, 30, hWnd, (HMENU)ID_BUTTON_CAPACITOR, hInstance, NULL);
    CreateWindow(L"BUTTON", L"Inductor", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        10, 90, 80, 30, hWnd, (HMENU)ID_BUTTON_INDUCTOR, hInstance, NULL);
    CreateWindow(L"BUTTON", L"Voltage", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        10, 130, 80, 30, hWnd, (HMENU)ID_BUTTON_VOLTAGE, hInstance, NULL);
    CreateWindow(L"BUTTON", L"Move", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        10, 170, 80, 30, hWnd, (HMENU)ID_BUTTON_MOVE, hInstance, NULL);

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

void ShowContextMenu(HWND hWnd, int x, int y) {
    HMENU hPopupMenu = CreatePopupMenu();
    AppendMenu(hPopupMenu, MF_STRING, ID_MENU_DELETE, L"Delete");

    TrackPopupMenu(hPopupMenu, TPM_TOPALIGN | TPM_LEFTALIGN, x, y, 0, hWnd, NULL);
    DestroyMenu(hPopupMenu);
}

void UpdateMoveButtonColor(HWND hWndMoveButton) {
    if (isMoveMode) {
        SetWindowText(hWndMoveButton, L"Move (ON)");
        SendMessage(hWndMoveButton, BM_SETSTYLE, BS_PUSHBUTTON, TRUE);
        SetWindowLong(hWndMoveButton, GWL_STYLE, GetWindowLong(hWndMoveButton, GWL_STYLE) | BS_OWNERDRAW);
        InvalidateRect(hWndMoveButton, NULL, TRUE);
        SetBkColor(GetDC(hWndMoveButton), RGB(0, 255, 0));  // Zielony
    }
    else {
        SetWindowText(hWndMoveButton, L"Move");
        SetBkColor(GetDC(hWndMoveButton), RGB(240, 240, 240));  // Domyślny kolor
        InvalidateRect(hWndMoveButton, NULL, TRUE);
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    static HWND hWndMoveButton = NULL;
    static int selectedComponent = -1;
    static POINT prevMousePos = { 0, 0 };

    switch (message) {
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case ID_BUTTON_RESISTOR:
            draggingComponent = ID_BUTTON_RESISTOR;
            break;
        case ID_BUTTON_CAPACITOR:
            draggingComponent = ID_BUTTON_CAPACITOR;
            break;
        case ID_BUTTON_INDUCTOR:
            draggingComponent = ID_BUTTON_INDUCTOR;
            break;
        case ID_BUTTON_VOLTAGE:
            draggingComponent = ID_BUTTON_VOLTAGE;
            break;
        case ID_BUTTON_MOVE:
            isMoveMode = !isMoveMode; // Przełącz tryb przenoszenia
            UpdateMoveButtonColor(hWndMoveButton);  // Zaktualizuj kolor przycisku
            break;
        case ID_MENU_DELETE:
            if (rightClickedComponentIndex != -1) {
                circuit.removeComponent(rightClickedComponentIndex);
                rightClickedComponentIndex = -1;
            }
            else if (rightClickedConnectionIndex != -1) {
                circuit.removeConnection(rightClickedConnectionIndex);
                rightClickedConnectionIndex = -1;
            }
            InvalidateRect(hWnd, NULL, TRUE);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;

    case WM_CREATE:
        hWndMoveButton = GetDlgItem(hWnd, ID_BUTTON_MOVE);  // Pobierz uchwyt do przycisku "Move"
        break;

    case WM_RBUTTONDOWN: {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);

        // Sprawdzenie, czy kliknięto na komponent
        for (int i = 0; i < circuit.getComponentCount(); ++i) {
            auto component = circuit.getComponent(i);
            if (isOnComponent(component, x, y)) {
                rightClickedComponentIndex = i;
                ShowContextMenu(hWnd, x, y);  // Pokaż menu kontekstowe obok kursora
                return 0;
            }
        }

        // Sprawdzenie, czy kliknięto na połączenie (linię)
        int index = 0;
        for (const auto& conn : circuit.getConnections()) {
            if (isOnConnection(conn, x, y)) {
                rightClickedConnectionIndex = index;
                ShowContextMenu(hWnd, x, y);  // Pokaż menu kontekstowe obok kursora
                return 0;
            }
            ++index;
        }
    }
                       break;

    case WM_LBUTTONDOWN: {
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);

        if (draggingComponent != -1) {
            switch (draggingComponent) {
            case ID_BUTTON_RESISTOR:
                circuit.addComponent(std::make_shared<Resistor>(100, "R" + std::to_string(++resistorCount)));
                break;
            case ID_BUTTON_CAPACITOR:
                circuit.addComponent(std::make_shared<Capacitor>(0.001, "C" + std::to_string(++capacitorCount)));
                break;
            case ID_BUTTON_INDUCTOR:
                circuit.addComponent(std::make_shared<Inductor>(0.01, "L" + std::to_string(++inductorCount)));
                break;
            case ID_BUTTON_VOLTAGE:
                circuit.addComponent(std::make_shared<VoltageSource>(12, "V" + std::to_string(++voltageSourceCount)));
                break;
            }

            auto component = circuit.getComponent(circuit.getComponentCount() - 1);
            component->setPosition(x, y);
            InvalidateRect(hWnd, NULL, TRUE);
            draggingComponent = -1;
        }
        else if (!isMoveMode) {  // Tryb rysowania linii
            for (int i = 0; i < circuit.getComponentCount(); ++i) {
                auto component = circuit.getComponent(i);
                if (isOnComponent(component, x, y)) {
                    lineStartComponent = i;
                    POINT circlePos = component->getCirclePosition();
                    lineStartX = circlePos.x;
                    lineStartY = circlePos.y;
                    isDrawingLine = true;
                    return 0;
                }
            }

            int index = 0;
            for (const auto& conn : circuit.getConnections()) {
                if (isOnConnection(conn, x, y)) {
                    lineStartConnectionIndex = index;
                    lineStartX = x;
                    lineStartY = y;
                    isDrawingLine = true;
                    return 0;
                }
                ++index;
            }
        }
        else {  // Tryb przenoszenia
            for (int i = 0; i < circuit.getComponentCount(); ++i) {
                auto component = circuit.getComponent(i);
                if (isOnComponent(component, x, y)) {
                    selectedComponentIndex = i;
                    prevMousePos.x = x;
                    prevMousePos.y = y;
                    return 0;
                }
            }
        }
    }
                       break;

    case WM_LBUTTONUP: {
        if (isDrawingLine) {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);

            for (int i = 0; i < circuit.getComponentCount(); ++i) {
                auto component = circuit.getComponent(i);
                if (isOnComponent(component, x, y)) {
                    POINT startCircle = { lineStartX, lineStartY };
                    POINT endCircle = component->getCirclePosition();

                    Connection conn = { lineStartComponent, i, startCircle.x, startCircle.y, endCircle.x, endCircle.y };
                    circuit.addConnection(conn);
                    isDrawingLine = false;
                    InvalidateRect(hWnd, NULL, TRUE);
                    return 0;
                }
            }

            int index = 0;
            for (const auto& conn : circuit.getConnections()) {
                if (isOnConnection(conn, x, y)) {
                    Connection newConn = { lineStartConnectionIndex, index, lineStartX, lineStartY, x, y };
                    circuit.addConnection(newConn);
                    isDrawingLine = false;
                    InvalidateRect(hWnd, NULL, TRUE);
                    return 0;
                }
                ++index;
            }

            isDrawingLine = false;
            InvalidateRect(hWnd, NULL, TRUE);
        }

        if (isMoveMode && selectedComponentIndex != -1) {
            selectedComponentIndex = -1;
        }
    }
                     break;

    case WM_MOUSEMOVE: {
        if (isDrawingLine) {
            lineEndX = LOWORD(lParam);
            lineEndY = HIWORD(lParam);
            InvalidateRect(hWnd, NULL, TRUE);
        }

        if (isMoveMode && selectedComponentIndex != -1) {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);

            auto component = circuit.getComponent(selectedComponentIndex);
            component->setPosition(component->getX() + (x - prevMousePos.x), component->getY() + (y - prevMousePos.y));
            circuit.updateConnections(selectedComponentIndex);
            prevMousePos.x = x;
            prevMousePos.y = y;
            InvalidateRect(hWnd, NULL, TRUE);
        }
    }
                     break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        HPEN hPen = CreatePen(PS_SOLID, 2, RGB(0, 0, 255));
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);

        if (isDrawingLine) {
            MoveToEx(hdc, lineStartX, lineStartY, NULL);
            LineTo(hdc, lineEndX, lineEndY);
        }

        circuit.draw(hdc);

        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
        EndPaint(hWnd, &ps);
    }
                 break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}
