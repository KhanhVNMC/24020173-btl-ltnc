#include <windows.h>
#include <vector>
#include <queue>
#include <functional>
#include <map>
#include <string>
#include <thread>
#include <chrono>
#include <sstream>
#include "sbg.h"

/**
 * ROUGH TRANSLATION OF JFRAME TetrisTest.java
 */
using namespace std;
const int WIDTH = 550;
const int HEIGHT = 750;
TetrisEngine *engine = nullptr;
HWND g_hwnd = nullptr;
static long dt = -1;
static std::string statusText = "";
const int HOLD_DISPLAY_OFFSET = 70;
const int COLORS[9] = {
        -1,         // 0 is empty.
        0xa000f0,   // T_Mino (Purple)
        0xf00000,   // Z_Mino (Red)
        0x00f000,   // S_Mino (Green)
        0xf0a000,   // L_Mino (Orange)
        0x3333e6,   // J_Mino (Blue)
        0x00f0f0,   // I_Mino (Cyan)
        0xf0f000,   // O_Mino (Yellow)
        0x969696    // Extra
};
const int GHOST = 0x3d3d3d;

COLORREF IntToCOLORREF(int color) {
    int r = (color >> 16) & 0xFF;
    int g = (color >> 8) & 0xFF;
    int b = (color) & 0xFF;
    return RGB(r, g, b);
}

void squareAt(HDC hdc, int x, int y, int rad, COLORREF color) {
    RECT rect = {x, y, x + rad, y + rad};
    HBRUSH brush = CreateSolidBrush(color);
    FillRect(hdc, &rect, brush);
    DeleteObject(brush);
}

void updateAndPaintMatrix(HDC hdc);

// split a string by '\n'
vector<string> splitString(const std::string &str, char delim) {
    vector<string> tokens;
    std::istringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delim)) {
        tokens.push_back(token);
    }
    return tokens;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            g_hwnd = hwnd;
            SevenBagGenerator *generator = new SevenBagGenerator(123);
            TetrisConfig* config = TetrisConfig::builder();
            config->setLineClearsDelay(0.5);

            engine = new TetrisEngine(config, generator);

            engine->runOnGameOver([]() {
                int msg = MessageBoxA(NULL, "GAME OVER. You topped out! Click Retry to reset your board!", "TETRIS: C++ EDITION", MB_RETRYCANCEL | MB_ICONINFORMATION);

                if (msg == IDRETRY) {
                    engine->resetPlayfield();
                    return;
                }

                PostQuitMessage(0);
                exit(0);
            });

            engine->onPlayfieldEvent([](const PlayfieldEvent &e) {
                std::string message;
                if (e.isSpin() || e.isMiniSpin()) {
                    std::string minoLetter = e.getLastMino()->name().substr(0, 1);
                    if (e.isMiniSpin())
                        message += "Mini\n";
                    message += minoLetter + "-Spin\n";
                }
                int linesCleared = static_cast<int>(e.getLinesCleared().size());
                switch (linesCleared) {
                    case 1:
                        message += "Single";
                        break;
                    case 2:
                        message += "Double";
                        break;
                    case 3:
                        message += "Triple";
                        break;
                    case 4:
                        message += "TETRIS!";
                        break;
                    default:
                        break;
                }
                statusText = message;
                if (e.isPerfectClear()) {
                    statusText += "\n\nPERFECT\nCLEAR!";
                }
                engine->cancelTask(dt);
                dt = engine->scheduleDelayedTask(60 * 2, []() {
                    statusText = "";
                });
            });

            engine->runOnTickEnd([hwnd]() {
                PostMessage(hwnd, WM_USER + 1, 0, 0);
            });
            std::thread engineThread([]() {
                engine->start();
            });
            engineThread.detach();

            break;
        }
        case WM_USER + 1: {
            InvalidateRect(hwnd, NULL, TRUE);
            break;
        }
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            updateAndPaintMatrix(hdc);
            EndPaint(hwnd, &ps);
            break;
        }
        case WM_KEYDOWN: {
            if (engine) {
                switch (wParam) {
                    case VK_LEFT:
                        engine->moveLeft();
                        break;
                    case VK_RIGHT:
                        engine->moveRight();
                        break;
                    case VK_UP:
                    case 'X':
                        engine->rotateCW();
                        break;
                    case 'Z':
                        engine->rotateCCW();
                        break;
                    case VK_DOWN:
                        engine->softDropToggle(true);
                        break;
                    case VK_SPACE:
                        engine->hardDrop();
                        break;
                    case 'C':
                        engine->hold();
                        break;
                    case 'T':
                        engine->raiseGarbage(3, 9);
                        break;
                    default:
                        break;
                }
            }
            break;
        }
        case WM_KEYUP: {
            if (engine && wParam == VK_DOWN) {
                engine->softDropToggle(false);
            }
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

void updateAndPaintMatrix(HDC hdc) {
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, WIDTH, HEIGHT);
    HBITMAP oldBmp = (HBITMAP) SelectObject(memDC, memBitmap);

    RECT rect = {0, 0, WIDTH, HEIGHT};
    HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(memDC, &rect, blackBrush);
    DeleteObject(blackBrush);

    SetBkMode(memDC, TRANSPARENT);
    SetTextColor(memDC, RGB(255, 255, 255));

    // Create fonts.
    HFONT font18 = CreateFontA(18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                               ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                               CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                               DEFAULT_PITCH | FF_SWISS, "Arial");
    HFONT font25 = CreateFontA(25, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                               ANSI_CHARSET, OUT_DEFAULT_PRECIS,
                               CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
                               DEFAULT_PITCH | FF_SWISS, "Arial");

    HFONT oldFont = (HFONT) SelectObject(memDC, font18);
    TextOutA(memDC, 50, 25, "HOLD", 4);
    TextOutA(memDC, WIDTH - 110, 25, "NEXT", 4);

    // Draw status text if available.
    if (!statusText.empty()) {
        vector<string> lines = splitString(statusText, '\n');
        int yOffset = 200;
        TEXTMETRIC tm;
        GetTextMetrics(memDC, &tm);
        for (const auto &line: lines) {
            std::string upper;
            for (char c: line)
                upper.push_back(toupper(c));
            TextOutA(memDC, 40, yOffset, upper.c_str(), (int) upper.size());
            yOffset += tm.tmHeight;
        }
    }

    // draw debug info.
    SelectObject(memDC, font25);
    char debugStr[128] = {0};
    sprintf_s(debugStr, "Frame Time / FPS: %d.0ms %.1f",
              engine->lastTickTime,
              ((float) engine->ticksPassed / ((System::currentTimeMillis() - engine->startedAt) / 1000.0)));
    TextOutA(memDC, 20, HEIGHT - 70, debugStr, (int) strlen(debugStr));

    SelectObject(memDC, oldFont);
    DeleteObject(font18);
    DeleteObject(font25);

    // --- Draw the playfield ---
    const vector<vector<int>> &board = engine->getBoardBuffer();
    int bx = 70 + HOLD_DISPLAY_OFFSET;
    int by = 50;

    for (size_t col = 0; col < board.size(); col++) {
        for (int row = 0; row < 22; row++) {
            int nodeX = bx + (26 * col);
            int nodeY = by + (26 * row);
            // Offset row index to draw bottom 22 rows.
            int boardRow = row + (40 - 22);
            int cell = board[col][boardRow];
            int colInt;
            if (cell == GHOST_PIECE_CONVENTION)
                colInt = GHOST;
            else
                colInt = COLORS[abs(cell)];
            squareAt(memDC, nodeX, nodeY, 25, IntToCOLORREF(colInt == -1 ? 0x212121 : colInt));
        }
    }

    // --- Draw the Next queue ---
    int qx = 350 + HOLD_DISPLAY_OFFSET;
    int qy = 70;
    int index = 0;

    std::queue<MinoTypeEnum *> nextQueue = engine->getNextQueue();
    while (!nextQueue.empty()) {
        MinoTypeEnum *piece = nextQueue.front();
        nextQueue.pop();

        if (index > 4) break; // only renders up to 5 pieces

        int py = qy + (index * 80);
        for (int gx = 0; gx < 4; gx++) {
            for (int gy = 0; gy < 2; gy++) {
                int nX = qx + (26 * gx);
                int nY = py + (26 * gy);
                int legacy = piece->renderMatrix[gy][gx];
                int pieceColor = piece->ordinal + 1;
                int colorVal = (legacy == 1 ? COLORS[pieceColor] : 0);
                if (colorVal != 0) {
                    squareAt(memDC, nX, nY, 25, IntToCOLORREF(colorVal));
                }
            }
        }
        index++;
    }

    // --- Draw the Hold piece ---
    int ohx = 25;
    int ohy = 70;

    if (engine->getHoldPiece() != nullptr) {
        MinoTypeEnum *holdPiece = engine->getHoldPiece();

        // legacy shape size is 2x4 (that's why its there)
        for (int hx = 0; hx < 4; hx++) {
            for (int hy = 0; hy < 2; hy++) {
                int nX = ohx + (26 * hx);
                int nY = ohy + (26 * hy);
                int pieceColor = holdPiece->ordinal + 1;
                int colorVal = (holdPiece->renderMatrix[hy][hx] == 1 ? COLORS[pieceColor] : 0);
                if (colorVal != 0) {
                    int finalColor = engine->canUseHold() ? colorVal : 0x636363;
                    squareAt(memDC, nX, nY, 25, IntToCOLORREF(finalColor));
                }
            }
        }
    }

    BitBlt(hdc, 0, 0, WIDTH, HEIGHT, memDC, 0, 0, SRCCOPY);
    SelectObject(memDC, oldBmp);
    DeleteObject(memBitmap);
    DeleteDC(memDC);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "TetrisWindowClass";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassEx(&wc);

    // main window
    HWND hwnd = CreateWindowEx(
            0,
            "TetrisWindowClass",
            "Tetris Engine: C++ Edition - Native Windows.h API Version",
            WS_CAPTION | WS_SYSMENU | WS_MINIMIZE | WS_OVERLAPPED, // smol window
            CW_USEDEFAULT, CW_USEDEFAULT,
            WIDTH, HEIGHT,
            NULL,
            NULL,
            hInstance,
            NULL
    );
    if (!hwnd) { return 0; }
    ShowWindow(hwnd, nCmdShow);
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
