#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <optional>
#include <ctime>
#include <cstdlib>
#include <cstdio>
#include <algorithm>

using namespace Gdiplus;
using namespace std;
#pragma comment(lib, "Gdiplus.lib")

enum Ksztalt { KWADRAT, TROJKAT, KOLO };

struct Blok {
    int x, y;
    Ksztalt ksztalt;
    int waga;
};

ULONG_PTR tokenGDI;

vector<Blok> wiezaKwadratow;
vector<Blok> wiezaTrojkotow;
vector<Blok> wiezaKol;

vector<Blok> wiezaBota1;
vector<Blok> wiezaBota2;

optional<Blok> nowyBlok;

bool czyNiesie = false;
Blok niesionyBlok;

int hakX = 380, hakY = 100;
const int minX = 80, maxX = 600;
const int minY = 60, maxY = 350;
const int podstawaY = 350;

const int wiezaBot1X = 80;
const int wiezaBot2X = 160;
const int kwadratX = 280;
const int trojkatX = 360;
const int koloX = 440;
const int spawnX = 220;

Ksztalt sekwencjaBota1[3] = { KWADRAT, TROJKAT, KOLO };
Ksztalt sekwencjaBota2[3] = { KOLO, KWADRAT, TROJKAT };

int krokBota1 = 0;
int krokBota2 = 0;
bool bot1Dziala = false;
bool bot2Dziala = false;
bool animacjaBota = false;

int krokAnimacji = 0;
int animHakX = 0, animHakY = 0;
int animBlokX = 0, animBlokY = 0;
Ksztalt animKsztalt;
int animWaga;
int animCelX = 0;
int animStartX = 0;

HWND przyciskStartBot1 = NULL;
HWND przyciskStartBot2 = NULL;
HWND oknoGlowne = NULL;

Ksztalt LosowyKsztalt() {
    return (Ksztalt)(rand() % 3);
}

void StworzNowyBlok() {
    Ksztalt ksztalt = LosowyKsztalt();
    int waga = rand() % 150 + 1;
    nowyBlok = Blok{ spawnX, 40, ksztalt, waga };
}

void ResetujBiezacyBlok() {
    if (!czyNiesie && !animacjaBota) {
        nowyBlok.reset();
        StworzNowyBlok();
        InvalidateRect(oknoGlowne, NULL, TRUE);
    }
}

void RysujBlok(Graphics& grafika, const Blok& blok, bool czyNiesiony = false) {
    SolidBrush wypelnienie(Color::LightGray);
    Pen kontur(Color::Black, 2);
    int rysujY = podstawaY - blok.y;
    if (czyNiesiony) rysujY = blok.y;

    Rect granice(blok.x - 20, rysujY, 40, 40);

    switch (blok.ksztalt) {
    case KWADRAT:
        grafika.FillRectangle(&wypelnienie, granice);
        grafika.DrawRectangle(&kontur, granice);
        break;
    case TROJKAT: {
        Point punkty[3] = {
            Point(blok.x, rysujY),
            Point(blok.x - 20, rysujY + 40),
            Point(blok.x + 20, rysujY + 40)
        };
        grafika.FillPolygon(&wypelnienie, punkty, 3);
        grafika.DrawPolygon(&kontur, punkty, 3);
        break;
    }
    case KOLO:
        grafika.FillEllipse(&wypelnienie, granice);
        grafika.DrawEllipse(&kontur, granice);
        break;
    }

    char tekst[16];
    sprintf(tekst, "%dkg", blok.waga);
    Font czcionka(L"Arial", 8);
    SolidBrush pedzelTekstu(Color::Black);
    StringFormat format;
    format.SetAlignment(StringAlignmentCenter);

    RectF obszarTekstu;
    if (blok.ksztalt == KOLO) {
        format.SetLineAlignment(StringAlignmentCenter);
        obszarTekstu = RectF(granice.X, granice.Y, granice.Width, granice.Height);
    }
    else {
        format.SetLineAlignment(StringAlignmentNear);
        obszarTekstu = RectF(granice.X, granice.Y + 26, granice.Width, granice.Height);
    }

    WCHAR wtekst[16];
    MultiByteToWideChar(CP_ACP, 0, tekst, -1, wtekst, 16);

    grafika.DrawString(wtekst, -1, &czcionka, obszarTekstu, &format, &pedzelTekstu);
}

void RysujWieze(Graphics& grafika, const vector<Blok>& wieza) {
    for (const auto& blok : wieza)
        RysujBlok(grafika, blok);
}

void RysujPodpowiedzi(Graphics& grafika) {
    SolidBrush podpowiedz(Color::LightGray);
    Pen kontur(Color::Black, 1);

    grafika.FillRectangle(&podpowiedz, wiezaBot1X - 25, podstawaY, 50, 5);
    grafika.FillRectangle(&podpowiedz, wiezaBot2X - 25, podstawaY, 50, 5);
    grafika.DrawRectangle(&kontur, wiezaBot1X - 25, podstawaY, 50, 5);
    grafika.DrawRectangle(&kontur, wiezaBot2X - 25, podstawaY, 50, 5);

    grafika.FillRectangle(&podpowiedz, kwadratX - 25, podstawaY, 50, 5);
    grafika.FillRectangle(&podpowiedz, trojkatX - 25, podstawaY, 50, 5);
    grafika.FillRectangle(&podpowiedz, koloX - 25, podstawaY, 50, 5);

    grafika.DrawRectangle(&kontur, kwadratX - 25, podstawaY, 50, 5);
    grafika.DrawRectangle(&kontur, trojkatX - 25, podstawaY, 50, 5);
    grafika.DrawRectangle(&kontur, koloX - 25, podstawaY, 50, 5);

    SolidBrush ikona(Color::DarkGray);
    grafika.FillRectangle(&ikona, wiezaBot1X - 7, podstawaY + 10, 14, 14);
    Point trojkatBota[3] = {
        Point(wiezaBot2X, podstawaY + 10),
        Point(wiezaBot2X - 7, podstawaY + 24),
        Point(wiezaBot2X + 7, podstawaY + 24)
    };
    grafika.FillPolygon(&ikona, trojkatBota, 3);
    grafika.FillEllipse(&ikona, wiezaBot2X - 7, podstawaY + 10, 14, 14);

    grafika.FillRectangle(&ikona, kwadratX - 7, podstawaY + 10, 14, 14);
    Point trojkatGracza[3] = {
        Point(trojkatX, podstawaY + 10),
        Point(trojkatX - 7, podstawaY + 24),
        Point(trojkatX + 7, podstawaY + 24)
    };
    grafika.FillPolygon(&ikona, trojkatGracza, 3);
    grafika.FillEllipse(&ikona, koloX - 7, podstawaY + 10, 14, 14);
}

void RysujDzwig(Graphics& grafika) {
    grafika.Clear(Color::White);

    LinearGradientBrush wiezaDzwigu(Rect(350, 50, 20, 300), Color(180, 180, 180), Color(100, 100, 100), LinearGradientModeVertical);
    grafika.FillRectangle(&wiezaDzwigu, 350, 50, 20, 300);

    LinearGradientBrush ramieDzwigu(Rect(50, 50, 560, 10), Color(160, 160, 160), Color(120, 120, 120), LinearGradientModeHorizontal);
    grafika.FillRectangle(&ramieDzwigu, 50, 50, 560, 10);

    int obecnyHakX = animacjaBota ? animHakX : hakX;
    int obecnyHakY = animacjaBota ? animHakY : hakY;
    
    Pen lina(Color::Black, 2);
    grafika.DrawLine(&lina, obecnyHakX, 60, obecnyHakX, obecnyHakY);

    if (czyNiesie && !animacjaBota) {
        Blok blok = niesionyBlok;
        blok.x = obecnyHakX;
        blok.y = obecnyHakY;
        RysujBlok(grafika, blok, true);
    }

    RysujPodpowiedzi(grafika);
    RysujWieze(grafika, wiezaBota1);
    RysujWieze(grafika, wiezaBota2);
    RysujWieze(grafika, wiezaKwadratow);
    RysujWieze(grafika, wiezaTrojkotow);
    RysujWieze(grafika, wiezaKol);

    if (nowyBlok.has_value() && !animacjaBota) {
        RysujBlok(grafika, nowyBlok.value(), false);
    }

    if (animacjaBota) {
        Blok animBlok = { animBlokX, animBlokY, animKsztalt, animWaga };
        RysujBlok(grafika, animBlok, true);
    }

    Font czcionkaInfo(L"Arial", 10, FontStyleBold);
    SolidBrush pedzelInfo(Color::Black);
    grafika.DrawString(L"Max udzwig: 100kg", -1, &czcionkaInfo, PointF(10, 10), &pedzelInfo);
    grafika.DrawString(L"Sterowanie: <- -> ^ v = ruch | SPACJA = podnies/opusc | R = nowy blok", -1, &czcionkaInfo, PointF(10, 30), &pedzelInfo);
}

bool CzyWZasiegu(int a, int b) {
    return abs(a - b) <= 20;
}

void DodajBlokDoWiezyBota(vector<Blok>& wieza, Ksztalt ksztalt, int pozycjaX) {
    if (wieza.size() >= 3) return;

    int waga = 50;
    Blok blok{ pozycjaX, (int)(wieza.size() * 40), ksztalt, waga };
    wieza.push_back(blok);
}

void RozpocznijAnimacjeBota(int startX, int koniecX, Ksztalt ksztalt, int waga) {
    animacjaBota = true;
    krokAnimacji = 0;
    animHakX = startX;
    animHakY = podstawaY;
    animBlokX = startX;
    animBlokY = podstawaY - 40;
    animKsztalt = ksztalt;
    animWaga = waga;
    animCelX = koniecX;
    animStartX = startX;
}

void AktualizujAnimacjeBota() {
    if (!animacjaBota) return;

    const int calkowiteKroki = 60;
    krokAnimacji++;

    if (krokAnimacji < 20) {
        float postep = (float)krokAnimacji / 20.0f;
        animHakY = podstawaY - (int)(postep * 100);
        animBlokY = podstawaY - 40 - (int)(postep * 100);
    }
    else if (krokAnimacji < 40) {
        float postep = (float)(krokAnimacji - 20) / 20.0f;
        animHakX = animStartX + (int)(postep * (animCelX - animStartX));
        animBlokX = animStartX + (int)(postep * (animCelX - animStartX));
    }
    else if (krokAnimacji < calkowiteKroki) {
        float postep = (float)(krokAnimacji - 40) / 20.0f;
        animHakY = podstawaY - 100 + (int)(postep * 100);
        animBlokY = podstawaY - 140 + (int)(postep * 100);
    }
    else {
        animacjaBota = false;
        if (bot1Dziala) {
            DodajBlokDoWiezyBota(wiezaBota1, animKsztalt, wiezaBot1X);
            krokBota1++;
            if (krokBota1 >= 3) bot1Dziala = false;
            StworzNowyBlok();
        }
        else if (bot2Dziala) {
            DodajBlokDoWiezyBota(wiezaBota2, animKsztalt, wiezaBot2X);
            krokBota2++;
            if (krokBota2 >= 3) bot2Dziala = false;
            StworzNowyBlok();
        }
        InvalidateRect(oknoGlowne, NULL, TRUE);
    }
}

LRESULT CALLBACK OknoProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        przyciskStartBot1 = CreateWindowA("BUTTON", "Start Bot 1",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            650, 10, 120, 30,
            hWnd, (HMENU)101, NULL, NULL);

        przyciskStartBot2 = CreateWindowA("BUTTON", "Start Bot 2",
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            650, 50, 120, 30,
            hWnd, (HMENU)102, NULL, NULL);

        SetTimer(hWnd, 1001, 30, NULL);
        break;

    case WM_COMMAND:
        if (LOWORD(wParam) == 101) {
            if (!bot1Dziala && !animacjaBota) {
                bot1Dziala = true;
                krokBota1 = 0;
                wiezaBota1.clear();
                if (!nowyBlok.has_value() && !czyNiesie) {
                    StworzNowyBlok();
                }
                PostMessage(przyciskStartBot1, BM_SETSTATE, FALSE, 0);
                UpdateWindow(przyciskStartBot1);
                InvalidateRect(hWnd, NULL, TRUE);
            }
        }
        else if (LOWORD(wParam) == 102) {
            if (!bot2Dziala && !animacjaBota) {
                bot2Dziala = true;
                krokBota2 = 0;
                wiezaBota2.clear();
                if (!nowyBlok.has_value() && !czyNiesie) {
                    StworzNowyBlok();
                }
                PostMessage(przyciskStartBot2, BM_SETSTATE, FALSE, 0);
                UpdateWindow(przyciskStartBot2);
                InvalidateRect(hWnd, NULL, TRUE);
            }
        }
        break;

    case WM_TIMER:
        if (wParam == 1001) {
            if (animacjaBota) {
                AktualizujAnimacjeBota();
                InvalidateRect(hWnd, NULL, TRUE);
            }
            else if (bot1Dziala && krokBota1 < 3) {
                RozpocznijAnimacjeBota(spawnX, wiezaBot1X, sekwencjaBota1[krokBota1], 50);
            }
            else if (bot2Dziala && krokBota2 < 3) {
                RozpocznijAnimacjeBota(spawnX, wiezaBot2X, sekwencjaBota2[krokBota2], 50);
            }
        }
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        Graphics grafika(hdc);
        RysujDzwig(grafika);
        EndPaint(hWnd, &ps);
        break;
    }
    case WM_KEYDOWN:
        switch (wParam) {
        case VK_LEFT: if (hakX > minX) hakX -= 10; break;
        case VK_RIGHT: if (hakX < maxX) hakX += 10; break;
        case VK_UP: if (hakY > minY) hakY -= 10; break;
        case VK_DOWN: if (hakY < maxY) hakY += 10; break;
        case 'R':
            ResetujBiezacyBlok();
            break;

        case VK_SPACE:
            if (!animacjaBota) {
                if (!czyNiesie) {
                    if (nowyBlok.has_value() && CzyWZasiegu(hakX, nowyBlok->x) && CzyWZasiegu(hakY, podstawaY - nowyBlok->y)) {
                        vector<Blok>* wieza = nullptr;
                        switch (nowyBlok->ksztalt) {
                        case KWADRAT: wieza = &wiezaKwadratow; break;
                        case TROJKAT: wieza = &wiezaTrojkotow; break;
                        case KOLO: wieza = &wiezaKol; break;
                        }

                        if (nowyBlok->waga > 100) {
                            MessageBoxA(hWnd, "Za ciezki blok", "Blad", MB_ICONERROR);
                        }
                        else if (wieza && wieza->size() >= 3) {
                            MessageBoxA(hWnd, "Brak miejsca", "Wieza pelna", MB_ICONWARNING);
                        }
                        else {
                            niesionyBlok = *nowyBlok;
                            czyNiesie = true;
                            nowyBlok.reset();
                        }
                    }
                }
                else {
                    vector<Blok>* docelowaWieza = nullptr;
                    if (niesionyBlok.ksztalt == KWADRAT && CzyWZasiegu(hakX, kwadratX)) docelowaWieza = &wiezaKwadratow;
                    else if (niesionyBlok.ksztalt == TROJKAT && CzyWZasiegu(hakX, trojkatX)) docelowaWieza = &wiezaTrojkotow;
                    else if (niesionyBlok.ksztalt == KOLO && CzyWZasiegu(hakX, koloX)) docelowaWieza = &wiezaKol;

                    if (docelowaWieza && docelowaWieza->size() < 3) {
                        niesionyBlok.x = (docelowaWieza == &wiezaKwadratow ? kwadratX :
                            (docelowaWieza == &wiezaTrojkotow ? trojkatX : koloX));
                        niesionyBlok.y = docelowaWieza->size() * 40;
                        docelowaWieza->push_back(niesionyBlok);
                        czyNiesie = false;
                        StworzNowyBlok();
                    }
                    else {
                        MessageBoxA(hWnd, "Brak miejsca", "Nie mozna upuscic bloku", MB_ICONWARNING);
                    }
                }
            }
            break;
        }
        InvalidateRect(hWnd, NULL, TRUE);
        break;

    case WM_DESTROY:
        GdiplusShutdown(tokenGDI);
        PostQuitMessage(0);
        break;

    default: return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    srand((unsigned)time(0));
    GdiplusStartupInput daneStartoweGDI;
    GdiplusStartup(&tokenGDI, &daneStartoweGDI, NULL);

    StworzNowyBlok();

    WNDCLASSA klasaOkna = {};
    klasaOkna.lpfnWndProc = OknoProc;
    klasaOkna.hInstance = hInstance;
    klasaOkna.lpszClassName = "OknoDzwigu";
    RegisterClassA(&klasaOkna);

    HWND hWnd = CreateWindowA(klasaOkna.lpszClassName, "Dzwig", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, hInstance, NULL);

    oknoGlowne = hWnd;

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    MSG wiadomosc;
    while (GetMessage(&wiadomosc, NULL, 0, 0)) {
        TranslateMessage(&wiadomosc);
        DispatchMessage(&wiadomosc);
    }

    return 0;
}
