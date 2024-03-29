/****************************************************************************
 *                                                                          *
 * File    : main.c                                                         *
 *                                                                          *
 * Purpose : Generic dialog based Win32 application.                        *
 *                                                                          *
 * History : Date      Reason                                               *
 *           17/03/07  Created                                              *
 *                                                                          *
 ****************************************************************************/

// �nderungsgeschichte
// 1.0.1.05 Im Edit-Dialog alte Endzeit als Vorgabe anzeigen    aN 27.03.2007
// 1.0.1.06 Aufruf f�r Edit-Dialog auf modal ge�ndert           aN 27.03.2007
// 1.0.1.07 Speichern und Wiederherstellen der Fensterposition  aN 10.04.2007
// 1.0.1.08 Eine einfache Syntaxpr�fung der Eingabe erg�nzt     aN 10.04.2007
// 1.0.1.09 Eine einfache Syntaxpr�fung der Eingabe erg�nzt     aN 10.04.2007
// 1.0.1.10 Bei fscanf die Anzahl gelesener Parameter gepr�ft   aN 10.04.2007
// 1.0.1.11 Farbcodes f�r die Restzeit                          aN 11.04.2007
// 1.0.1.12 Farben angepa�t und in SetColors gechoben           aN 11.04.2007
// 1.0.1.13 Minimieren auf Klick-Rechts                         aN 16.04.2007
// 1.0.1.14 Experimente mit Men�s, AktOutput                    aN 16.04.2007
// 1.0.1.15 Anzeige umschaltbar bei Minimiert                   aN 03.05.2007
// 1.0.1.16 Bei minimiert, Endzeitdialog Systemmodal            aN 04.06.2007
// 1.0.1.17 Alarmdialog                                         aN 05.06.2007
// 1.0.1.18 Alarmdialog Modal und zentriert                     aN 14.03.2012
// 1.0.1.19 Grund f�r Alarm, Eingabe und Verwaltung             aN 17.04.2012
// 1.0.1.20 Animiertes Tray-Icon                                aN 30.07.2012
// 1.0.1.23 Reduzierung des Aktualisieren der Anzeige           aN 31.07.2012
// 1.0.1.24 Main-Icon an Anzeige anpassen                       aN 31.07.2012
// 1.0.1.25 GDI-Leak in TrayIcon-Erzeugung korrigiert           aN 01.08.2012
// 1.0.1.26 GDI-Leak korrigiert 2.Versuch                       aN 01.08.2012
// 1.0.1.27 Anderes Format f�r Titelzeile                       aN 19.09.2012
// 1.0.1.28 Nochmals anderes Format f�r Titelzeile              aN 05.10.2012
// 1.0.1.29 gr��eres Zeit-Icon                                  aN 07.04.2023
// 1.0.1.30 Parameter Z(eit) und "Grund"                        aN 15.04.2023
// 1.9.0.31 Einlesen von ini und Parameter korrigiert           aN 28.06.2023
// 1.9.0.32 INI folgt EXE-Namen                                 aN 30.06.2023
// 1.9.0.33 Nur ein Edit-Dialog bei Doppelklick                 aN 01.07.2023
// 1.9.0.34 Bei Alarm Verl�ngerungsbuttons                      aN 02.07.2023
// 1.9.0.35 gemeinsame Daten in eine Struktur                   aN 08.07.2023
// 1.9.0.36 Uhren einzeln verstecken und wieder holen           aN 09.07.2023
// 1.9.0.37 bei Alarm ein vierter Button Edit                   aN 10.07.2023
// 1.9.0.38 Fenster auf TopMost schalten                        aN 22.07.2023
// 2.0.0.39 Uhren differenzieren analog, digital, beides        aN 27.07.2023
// 2.0.0.40 HPEN nach Gebrauch wieder l�schen                   aN 31.07.2023
// 2.0.0.41 Parameter S(panne) auswerten                        aN 07.08.2023
// 2.0.0.42 Edit-Dialog �berarbeitet                            aN 09.08.2023
// 2.0.0.43 Restzeit-Berechnung neu                             aN 14.08.2023
// 2.0.0.44 Farbrechnung und Farbe-Stundenzeiger neu            aN 18.08.2023
// 2.0.0.45 Refresh ohne RDW_ERASE und wieder zur�ck            aN 21.08.2023
// 2.0.0.46 Systemicon setzen mit WM_SETICON (64Bit-tauglich)   aN 05.09.2023
// 2.0.0.47 Vorl�ufige Endversion                               aN 11.09.2023
// 2.0.0.48 Reihenfolge der Zeiger der gr. Uhr �ndern           aN 28.01.2024
// 2.0.0.49 Hide und Top merken                                 aN 09.02.2024
// 2.0.0.50 Tray-Icon-Men� rep., Restore zeigt alle 3 Dialoge   aN 26.02.2024


/*
 * Either define WIN32_LEAN_AND_MEAN, or one or more of NOCRYPT,
 * NOSERVICE, NOMCX and NOIME, to decrease compile time (if you
 * don't need these defines -- see windows.h).
 */

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <winuser.h>
#include <tchar.h>
#include <stdio.h>
#include <wingdi.h>

#include "main.h"

/** #defines ***************************************************************/

#define NELEMS(a)  (sizeof(a) / sizeof((a)[0]))
#define Refresh(A) RedrawWindow(A,NULL,NULL,RDW_ERASE|RDW_INVALIDATE|RDW_ALLCHILDREN|RDW_UPDATENOW);
#define RefreshX(A) RedrawWindow(A,NULL,NULL,RDW_INVALIDATE|RDW_ALLCHILDREN|RDW_UPDATENOW);

#define BUF_SIZE            256
#define ICONSIZE            40
#define IMAGESIZE           40
#define BIGICONSIZE         96
#define BIGIMAGESIZE        96
#define STUNDE_COLOR_AM     RGB(  0,  0,255)
#define STUNDE_COLOR_PM     RGB(  0,127,191)
#define MINUTE_COLOR        RGB(  0,223,  0)
#define SEKUNDE_COLOR       RGB(  0,  0,  0)
#define WECKER_COLOR_AM     RGB(255,  0,  0)
#define WECKER_COLOR_PM     RGB(191, 64,  0)
#define MASK_COLOR          RGB(255,255,255)
#define GCL_HICON           (-14)

/** Prototypes **************************************************************/

static LRESULT CALLBACK DlgProcMain(HWND, UINT, WPARAM, LPARAM);
static LRESULT CALLBACK DlgProcEdit(HWND, UINT, WPARAM, LPARAM);
static LRESULT CALLBACK DlgProcAlarm(HWND, UINT, WPARAM, LPARAM);
void AktOutput(HWND hwndDlg);
void SetColors(HWND hwndCtl, HDC wParam);
HBRUSH SetBkfColor(COLORREF TxtColr, COLORREF BkColr, HDC hdc);
void SaveRect(void);
void CalcRestZeit(SYSTEMTIME j, SYSTEMTIME e, SYSTEMTIME *r);
void AddTime(int diff);

/** Typen *******************************************************************/
typedef struct
{
    HWND hWnd;
    HMENU hSMenu;
    int hide;
    int top;
    RECT rWndDlg;
} uhr;
/** Global variables ********************************************************/

static HANDLE ghInstance;
static COLORREF gBackgroundColor;
static COLORREF gForegroundColor;
static HICON hBackIcon;
static HICON hBigIcon;
//static HWND hMainWnd = 0;
//static int minimized = 0;
static int show_rest = 1;
static int blackwhite = 0;
static char IniName[300] = "WinUhr2.ini";

//static HMENU hSysMenu = NULL;
static HMENU hPopupMenu = NULL;
static NOTIFYICONDATA nid = {0};
static HICON hIcon;
static HICON hBIcon;

int AlarmDlg = 0;  // Flag ob der Alarmdialog eingeschaltet ist
int erreicht = 0;  // Flag f�r Zeit erreicht

SYSTEMTIME DZ = {2012, 0, 0,12,0,0,0,0};
SYSTEMTIME EZ = {2012, 3,14,17,0,0,0,0};
SYSTEMTIME RZ = { 0, 0, 0, 0,0,0,0,0};
char alarmgrund[100] = "";
char *wota[] = {"So\0nntag","Mo\0ntag","Di\0enstag","Mi\0ttwoch","Do\0nnerstag","Fr\0eitag","Sa\0mstag"};

uhr uhren[3] = {{NULL,NULL,0,0,{0,0,0,0}},
                {NULL,NULL,0,0,{0,0,0,0}},
                {NULL,NULL,0,0,{0,0,0,0}}};

//****************************************************************************
//  Get Parameter
//****************************************************************************
void GetParams(char *szCmdline)
{
    int i = 0;
    char *cp;
    char hStr[200];

    if (szCmdline == NULL)
        return;

    while (szCmdline[i] != 0)
    {
        //TODO
        switch (szCmdline[i])
        {
            case '"': // Grund
                cp = alarmgrund;
                i++;
                while ((szCmdline[i] != '"') && (szCmdline[i] != 0))
                {
                    //TODO
                    *(cp++) = szCmdline[i++];
                }
                *cp = 0;
                break;

            case 'Z': // Zeitpunkt
            case 'z':
                i++;
                if (szCmdline[i] == '=')
                    i++;
                if (szCmdline[i] == ':')
                    i++;
                cp = hStr;
                while ((szCmdline[i] != ' ') && (szCmdline[i] != '\0'))
                    *(cp++) = szCmdline[i++];
                *cp = 0;
                sscanf(hStr, "%hu:%hu:%hu", &EZ.wHour, &EZ.wMinute, &EZ.wSecond);
                break;

            case 'S': // Spanne
            case 's':
            {
                int p,z1,z2;
                i++;
                if (szCmdline[i] == '=')
                    i++;
                if (szCmdline[i] == ':')
                    i++;
                cp = hStr;
                while ((szCmdline[i] != ' ') && (szCmdline[i] != '\0'))
                    *(cp++) = szCmdline[i++];
                *cp = 0;
                p = sscanf(hStr, "%d:%d", &z1, &z2);
                switch(p)
                {
                    case 2:
                        p = z1*60+z2;
                        break;
                    case 1:
                        p = z1;
                        break;
                    default:
                        p = 0;
                        break;
                }
                GetLocalTime(&EZ);
                EZ.wSecond = 0;
                AddTime(p);
                break;
            }

            case '/': // Spanne
            case '-':
                //i++;
                break;

            default:
                //i++;
                break;
        }
        i++;
    }
}

//****************************************************************************
//  FillBitmap
//****************************************************************************
static void FillBitmap(HDC mdc, HBITMAP hBitmap, LONG x, LONG y, COLORREF Color)
{
    HBITMAP hRetBmp;
    HBRUSH hBrush;
    RECT fRect;

    hRetBmp = SelectObject(mdc, hBitmap);
    /* Pinssel und Rechteck ermitteln */
    hBrush = CreateSolidBrush(Color);
    fRect.top = fRect.left = x;
    fRect.right = fRect.bottom = y;
    /* Farbe setzen */
    FillRect(mdc, &fRect, hBrush);
    DeleteObject(hBrush);
    SelectObject(mdc, hRetBmp);
}

//****************************************************************************
//  ConvLinePoint
//****************************************************************************
static void ConvLinePoint(const int hmx, const int hmy, POINT *pt, int flag)
{
    switch (flag)
    {
        case 0: /* 0 bis 14 */
            pt->x = hmx;
            pt->y = hmy;
            break;
        case 1: /* 15 bis 29 */
            pt->x = IMAGESIZE - hmy;
            pt->y = hmx;
            break;
        case 2: /* 30 bis 44 */
            pt->x = IMAGESIZE - hmx;
            pt->y = IMAGESIZE - hmy;
            break;
        case 3: /* 45 bis 59 */
            pt->x = hmy;
            pt->y = IMAGESIZE - hmx;
            break;
    }
}

//****************************************************************************
//  ConvBigLinePoint
//****************************************************************************
static void ConvBigLinePoint(const int hmx, const int hmy, POINT *pt, int flag)
{
    switch (flag)
    {
        case 0: /* 0 bis 14 */
            pt->x = hmx;
            pt->y = hmy;
            break;
        case 1: /* 15 bis 29 */
            pt->x = BIGIMAGESIZE - hmy;
            pt->y = hmx;
            break;
        case 2: /* 30 bis 44 */
            pt->x = BIGIMAGESIZE - hmx;
            pt->y = BIGIMAGESIZE - hmy;
            break;
        case 3: /* 45 bis 59 */
            pt->x = hmy;
            pt->y = BIGIMAGESIZE - hmx;
            break;
    }
}

//****************************************************************************
//  CreateTimeIcon
//****************************************************************************
static HICON CreateTimeIcon(HWND hWnd)
{
    HDC hdc, mdc;
    HIMAGELIST IconList, mIconList, tIconList;
    HICON hIcon;
    HBITMAP hBitmap;
    HBITMAP hMaskBitmap;
    HBITMAP hRetBmp;
    HPEN hPen;

    POINT pt[2];
    SYSTEMTIME systim;

    int index, flag;
    static int gMinute = -1;
    static int gWStd = -1, gWMin = -1;

    // Tabelle f�r Stundenzeiger
    const static int hy[] = {  4,  4,  5,  7,  8, 11, 13, 16,};
    const static int hx[] = { 20, 23, 26, 29, 31, 33, 35, 36,};
    // Tabelle f�r Minutenzeiger
    const static int my[] = {  0,  0,  0,  1,  2,  3,  4,  5,  7,  8, 10, 12, 14, 16, 18,};
    const static int mx[] = { 20, 22, 24, 26, 28, 30, 32, 33, 35, 36, 37, 38, 39, 40, 40,};

    /* Zeit ermitteln und auf �nderung pr�fen */
    GetLocalTime(&systim);
    if (systim.wMinute == gMinute)
    {
        if ((gWStd == EZ.wHour) && (gWMin == EZ.wMinute))
        {
            return NULL;
        }
        else
        {
            gWStd = EZ.wHour;
            gWMin = EZ.wMinute;
        }
    }
    gMinute = systim.wMinute;

    // S T U N D E N - Z e i g e r
    hdc = GetDC(hWnd);
    mdc = CreateCompatibleDC(hdc);

    /* Bitmap f�r Vordergrund erzeugen */
    hBitmap = CreateCompatibleBitmap(hdc, ICONSIZE, ICONSIZE);
    FillBitmap(mdc, hBitmap, 0, ICONSIZE, (systim.wHour>=12)?STUNDE_COLOR_PM:STUNDE_COLOR_AM);

    /* Bitmap f�r Maske erzeugen */
    hMaskBitmap = CreateCompatibleBitmap(hdc, ICONSIZE, ICONSIZE);
    FillBitmap(mdc, hMaskBitmap, 0, ICONSIZE, MASK_COLOR);
    ReleaseDC(hWnd, hdc);
    hRetBmp = SelectObject(mdc, hMaskBitmap);

    pt[0].x = pt[0].y = IMAGESIZE / 2 + 1;
    /* Stundenzeiger zeichnen */
    index = ((systim.wHour % 12) * 5) + (systim.wMinute / 15);
    flag = index / 15;
    index %= 15;
    index = (((index * 10) / 2) + 5) / 10;
    ConvLinePoint(hx[index], hy[index], &pt[1], flag);
    hPen = CreatePen(PS_SOLID, 3, (systim.wHour>=12)?STUNDE_COLOR_PM:STUNDE_COLOR_AM);
    SelectObject(mdc, hPen);
    Polyline(mdc, pt, sizeof(pt) / sizeof(POINT));
    DeleteObject(hPen);

    /* Icon zusammen setzen */
    IconList = ImageList_Create(ICONSIZE, ICONSIZE, ILC_COLOR8 | ILC_MASK, 4, 5);
    ImageList_AddIcon(IconList, hBackIcon);

    /* GDI  */
    SelectObject(mdc, hRetBmp);
    DeleteDC(mdc);

    /* Zeit Icon zusammen setzen */
    ImageList_Add(IconList, hBitmap, hMaskBitmap);
    DeleteObject(hBitmap);
    DeleteObject(hMaskBitmap);

    // M I N U T E N - Z e i g e r

    hdc = GetDC(hWnd);
    mdc = CreateCompatibleDC(hdc);
    /* Bitmap f�r Vordergrund erzeugen */
    hBitmap = CreateCompatibleBitmap(hdc, ICONSIZE, ICONSIZE);
    FillBitmap(mdc, hBitmap, 0, ICONSIZE, MINUTE_COLOR);

    /* Bitmap f�r Maske erzeugen */
    hMaskBitmap = CreateCompatibleBitmap(hdc, ICONSIZE, ICONSIZE);
    FillBitmap(mdc, hMaskBitmap, 0, ICONSIZE, MASK_COLOR);
    ReleaseDC(hWnd, hdc);
    hRetBmp = SelectObject(mdc, hMaskBitmap);

    /* Minutenzeiger zeichnen */
    index = systim.wMinute % 15;
    flag = systim.wMinute / 15;
    ConvLinePoint(mx[index], my[index], &pt[1], flag);
    hPen = CreatePen(PS_SOLID, 2, MINUTE_COLOR);
    SelectObject(mdc, hPen);
    Polyline(mdc, pt, sizeof(pt) / sizeof(POINT));
    DeleteObject(hPen);

    /* Zeit Icon zusammen setzen */
    SelectObject(mdc, hRetBmp);
    DeleteDC(mdc);
    ImageList_Add(IconList, hBitmap, hMaskBitmap);
    DeleteObject(hBitmap);
    DeleteObject(hMaskBitmap);

    // W E C K E R - Zeiger
    hdc = GetDC(hWnd);
    mdc = CreateCompatibleDC(hdc);

    /* Bitmap f�r Vordergrund erzeugen */
    hBitmap = CreateCompatibleBitmap(hdc, ICONSIZE, ICONSIZE);
    FillBitmap(mdc, hBitmap, 0, ICONSIZE, (EZ.wHour>=12)?WECKER_COLOR_PM:WECKER_COLOR_AM);

    /* Bitmap f�r Maske erzeugen */
    hMaskBitmap = CreateCompatibleBitmap(hdc, ICONSIZE, ICONSIZE);
    FillBitmap(mdc, hMaskBitmap, 0, ICONSIZE, MASK_COLOR);
    ReleaseDC(hWnd, hdc);
    hRetBmp = SelectObject(mdc, hMaskBitmap);

    /* Stundenzeiger zeichnen */
    index = ((EZ.wHour % 12) * 5) + (EZ.wMinute / 15);
    flag = index / 15;
    index %= 15;
    index = (((index * 10) / 2) + 5) / 10;
    ConvLinePoint(hx[index], hy[index], &pt[1], flag);
    hPen = CreatePen(PS_SOLID, 2, (EZ.wHour>=12)?WECKER_COLOR_PM:WECKER_COLOR_AM);
    SelectObject(mdc, hPen);
    Polyline(mdc, pt, sizeof(pt) / sizeof(POINT));
    DeleteObject(hPen);

    /* Maske setzen */
    FillBitmap(mdc, hMaskBitmap, IMAGESIZE / 2, IMAGESIZE / 2 + 1, MASK_COLOR);

    /* Wecker Icon zusammen setzen */
    SelectObject(mdc, hRetBmp);
    DeleteDC(mdc);
    ImageList_Add(IconList, hBitmap, hMaskBitmap);
    DeleteObject(hBitmap);
    DeleteObject(hMaskBitmap);

    // "Mergen" der Icons
    mIconList = ImageList_Merge(IconList, 0, IconList, 1, 0, 0);  // Originalicon + Stundenzeiger
    tIconList = ImageList_Merge(mIconList, 0, IconList, 2, 0, 0);  // + Minutenzeiger
    ImageList_Destroy(mIconList);
    mIconList = ImageList_Merge(tIconList, 0, IconList, 3, 0, 0);  // Wecker
    ImageList_Destroy(tIconList);

    hIcon = ImageList_GetIcon(mIconList, 0, ILD_NORMAL);
    ImageList_Destroy(mIconList);
    ImageList_Destroy(IconList);

    return hIcon;
}

//****************************************************************************
//  CreateBigTimeIcon
//****************************************************************************
static HICON CreateBigTimeIcon(HWND hWnd)
{
    HDC hdc, mdc;
    HIMAGELIST IconList, mIconList, tIconList;
    HICON hIcon;
    HBITMAP hBitmap;
    HBITMAP hMaskBitmap;
    HBITMAP hRetBmp;
    HPEN hPen;

    POINT pt[2];
    SYSTEMTIME systim;

    int index, flag;
    static int gWStd = -1, gWMin = -1, gSekunde = -1;

    // Tabelle f�r Stundenzeiger
    const static int hy[] = { 10, 10, 11, 12, 13, 15, 17, 20, 23, 26, 29, 33, 36, 40, 44,};
    const static int hx[] = { 48, 52, 56, 60, 63, 67, 70, 73, 76, 79, 81, 83, 84, 85, 86,};
    // Tabelle f�r Minutenzeiger
    const static int my[] = {  0,  0,  1,  2,  4,  6,  9, 12, 16, 20, 24, 28, 33, 38, 43,};
    const static int mx[] = { 48, 53, 58, 63, 68, 72, 76, 80, 84, 87, 90, 92, 94, 95, 96,};

    /* Zeit ermitteln und auf �nderung pr�fen */
    GetLocalTime(&systim);
    if (systim.wSecond == gSekunde)
    {
        if ((gWStd == EZ.wHour) && (gWMin == EZ.wMinute))
        {
            return NULL;
        }
        else
        {
            gWStd = EZ.wHour;
            gWMin = EZ.wMinute;
        }
    }
    gSekunde = systim.wSecond;

    pt[0].x = pt[0].y = BIGIMAGESIZE / 2 +1;

    // W E C K E R - Zeiger
    hdc = GetDC(hWnd);
    mdc = CreateCompatibleDC(hdc);

    /* Bitmap f�r Vordergrund erzeugen */
    hBitmap = CreateCompatibleBitmap(hdc, BIGICONSIZE, BIGICONSIZE);
    FillBitmap(mdc, hBitmap, 0, BIGICONSIZE, (EZ.wHour>=12)?WECKER_COLOR_PM:WECKER_COLOR_AM);

    /* Bitmap f�r Maske erzeugen */
    hMaskBitmap = CreateCompatibleBitmap(hdc, BIGICONSIZE, BIGICONSIZE);
    FillBitmap(mdc, hMaskBitmap, 0, BIGICONSIZE, MASK_COLOR);
    ReleaseDC(hWnd, hdc);
    hRetBmp = SelectObject(mdc, hMaskBitmap);

    /* Stundenzeiger zeichnen */
    index = ((EZ.wHour % 12) * 5) + (EZ.wMinute / 15);
    flag = index / 15;
    index %= 15;
    //index = (((index * 10) / 2) + 5) / 10;
    ConvBigLinePoint(hx[index], hy[index], &pt[1], flag);
    hPen = CreatePen(PS_SOLID, 4, (EZ.wHour>=12)?WECKER_COLOR_PM:WECKER_COLOR_AM);
    SelectObject(mdc, hPen);
    Polyline(mdc, pt, sizeof(pt) / sizeof(POINT));
    DeleteObject(hPen);

    /* Icon zusammen setzen */
    IconList = ImageList_Create(BIGICONSIZE, BIGICONSIZE, ILC_COLOR8 | ILC_MASK, 4, 5);
    ImageList_AddIcon(IconList, hBigIcon);

    /* GDI  */
    SelectObject(mdc, hRetBmp);
    DeleteDC(mdc);

    /* Zeit Icon zusammen setzen */
    ImageList_Add(IconList, hBitmap, hMaskBitmap);
    DeleteObject(hBitmap);
    DeleteObject(hMaskBitmap);

    //hIcon = ImageList_GetIcon(mIconList, 0, ILD_NORMAL);
    //ImageList_Destroy(mIconList);
    //ImageList_Destroy(IconList);

    // S T U N D E N - Z e i g e r
    hdc = GetDC(hWnd);
    mdc = CreateCompatibleDC(hdc);

    /* Bitmap f�r Vordergrund erzeugen */
    hBitmap = CreateCompatibleBitmap(hdc, BIGICONSIZE, BIGICONSIZE);
    FillBitmap(mdc, hBitmap, 0, BIGICONSIZE, (systim.wHour>=12)?STUNDE_COLOR_PM:STUNDE_COLOR_AM);

    /* Bitmap f�r Maske erzeugen */
    hMaskBitmap = CreateCompatibleBitmap(hdc, BIGICONSIZE, BIGICONSIZE);
    FillBitmap(mdc, hMaskBitmap, 0, BIGICONSIZE, MASK_COLOR);
    ReleaseDC(hWnd, hdc);
    hRetBmp = SelectObject(mdc, hMaskBitmap);


    /* Stundenzeiger zeichnen */
    index = ((systim.wHour % 12) * 5) + (systim.wMinute / 15);
    flag = index / 15;
    index %= 15;
    //index = (((index * 10) / 2) + 5) / 10;
    ConvBigLinePoint(hx[index], hy[index], &pt[1], flag);

    hPen = CreatePen(PS_SOLID, 3, (systim.wHour>=12)?STUNDE_COLOR_PM:STUNDE_COLOR_AM);
    SelectObject(mdc, hPen);
    Polyline(mdc, pt, sizeof(pt) / sizeof(POINT));
    DeleteObject(hPen);

    /* GDI  */
    SelectObject(mdc, hRetBmp);
    DeleteDC(mdc);

    /* Zeit Icon zusammen setzen */
    ImageList_Add(IconList, hBitmap, hMaskBitmap);
    DeleteObject(hBitmap);
    DeleteObject(hMaskBitmap);

    // M I N U T E N - Z e i g e r

    hdc = GetDC(hWnd);
    mdc = CreateCompatibleDC(hdc);

    /* Bitmap f�r Vordergrund erzeugen */
    hBitmap = CreateCompatibleBitmap(hdc, BIGICONSIZE, BIGICONSIZE);
    FillBitmap(mdc, hBitmap, 0, BIGICONSIZE, MINUTE_COLOR);

    /* Bitmap f�r Maske erzeugen */
    hMaskBitmap = CreateCompatibleBitmap(hdc, BIGICONSIZE, BIGICONSIZE);
    FillBitmap(mdc, hMaskBitmap, 0, BIGICONSIZE, MASK_COLOR);
    ReleaseDC(hWnd, hdc);
    hRetBmp = SelectObject(mdc, hMaskBitmap);

    /* Minutenzeiger zeichnen */
    index = systim.wMinute % 15;
    flag = systim.wMinute / 15;
    ConvBigLinePoint(mx[index], my[index], &pt[1], flag);

    hPen = CreatePen(PS_SOLID, 2, MINUTE_COLOR);
    SelectObject(mdc, hPen);
    Polyline(mdc, pt, sizeof(pt) / sizeof(POINT));
    DeleteObject(hPen);

    /* Zeit Icon zusammen setzen */
    SelectObject(mdc, hRetBmp);
    DeleteDC(mdc);
    ImageList_Add(IconList, hBitmap, hMaskBitmap);
    DeleteObject(hBitmap);
    DeleteObject(hMaskBitmap);

    // S E K U N D E N - Z e i g e r
    hdc = GetDC(hWnd);
    mdc = CreateCompatibleDC(hdc);

    /* Bitmap f�r Vordergrund erzeugen */
    hBitmap = CreateCompatibleBitmap(hdc, BIGICONSIZE, BIGICONSIZE);
    FillBitmap(mdc, hBitmap, 0, BIGICONSIZE, SEKUNDE_COLOR);

    /* Bitmap f�r Maske erzeugen */
    hMaskBitmap = CreateCompatibleBitmap(hdc, BIGICONSIZE, BIGICONSIZE);
    FillBitmap(mdc, hMaskBitmap, 0, BIGICONSIZE, MASK_COLOR);
    ReleaseDC(hWnd, hdc);
    hRetBmp = SelectObject(mdc, hMaskBitmap);

    /* Sekundenzeiger zeichnen */
    index = systim.wSecond % 15;
    flag = systim.wSecond / 15;
    ConvBigLinePoint(mx[index], my[index], &pt[1], flag);
    //char hStr[200];
    //sprintf(hStr,"index:%2d flag:%d mx/y:%2d,%2d pt:%2d,%2d\n",index,flag,mx[index], my[index],pt[1].x,pt[1].y);
    //OutputDebugString(hStr);
    hPen = CreatePen(PS_SOLID, 1, SEKUNDE_COLOR);
    SelectObject(mdc, hPen);
    Polyline(mdc, pt, sizeof(pt) / sizeof(POINT));
    DeleteObject(hPen);

    /* Maske setzen */
    FillBitmap(mdc, hMaskBitmap, BIGIMAGESIZE / 2, BIGIMAGESIZE / 2, MASK_COLOR);

    /* Sekunden Icon zusammen setzen */
    SelectObject(mdc, hRetBmp);
    DeleteDC(mdc);
    ImageList_Add(IconList, hBitmap, hMaskBitmap);
    DeleteObject(hBitmap);
    DeleteObject(hMaskBitmap);

    // "Mergen" der Icons
    mIconList = ImageList_Merge(IconList, 0, IconList, 1, 0, 0);  // Originalicon + Stundenzeiger
    tIconList = ImageList_Merge(mIconList, 0, IconList, 2, 0, 0);  // + Minutenzeiger
    ImageList_Destroy(mIconList);
    mIconList = ImageList_Merge(tIconList, 0, IconList, 3, 0, 0);  // Wecker
    ImageList_Destroy(tIconList);
    tIconList = ImageList_Merge(mIconList, 0, IconList, 4, 0, 0);  // + Minutenzeiger
    ImageList_Destroy(mIconList);

    hIcon = ImageList_GetIcon(tIconList, 0, ILD_NORMAL);
    ImageList_Destroy(tIconList);
    ImageList_Destroy(IconList);

    return hIcon;
}

//****************************************************************************
// Setzen der Farben                                                        **
//****************************************************************************
void AktOutput(HWND hwndDlg)
{
    SYSTEMTIME ST;
    LPSYSTEMTIME lpST = &ST;
    char hStr[50];
    static int tag = -1;
    static int ez = -1;

    GetLocalTime(lpST);
    sprintf(hStr, "%02d:%02d:%02d", lpST->wHour, lpST->wMinute, lpST->wSecond);
    SetDlgItemText(hwndDlg, IDD_UHRZEIT, hStr);

    CalcRestZeit(ST, EZ, &RZ);

    if (ST.wDay != tag)
    {
        sprintf(hStr, "%2s: %02d.%02d.%04d", wota[ST.wDayOfWeek], ST.wDay, ST.wMonth, ST.wYear);
        SendMessage(uhren[0].hWnd, WM_SETTEXT, 0, (size_t)hStr);
        SendMessage(uhren[1].hWnd, WM_SETTEXT, 0, (size_t)hStr);
        SendMessage(uhren[2].hWnd, WM_SETTEXT, 0, (size_t)hStr);
        tag = ST.wDay;
    }

    if (ez != (((EZ.wHour << 6) + EZ.wMinute) << 6) + EZ.wSecond)
    {
        sprintf(hStr, "%02d:%02d:%02d .", EZ.wHour, EZ.wMinute, EZ.wSecond);
        //SetDlgItemText(hwndDlg, IDD_ENDZEIT, hStr);
        SetDlgItemText(uhren[0].hWnd, IDD_ENDZEIT, hStr);
        SetDlgItemText(uhren[1].hWnd, IDD_ENDZEIT, hStr);
        SetDlgItemText(uhren[2].hWnd, IDD_ENDZEIT, hStr);
        ez = (((EZ.wHour << 6) + EZ.wMinute) << 6) + EZ.wSecond;
    }
    sprintf(hStr, "%02d:%02d:%02d .", RZ.wHour, RZ.wMinute, RZ.wSecond);
    SetDlgItemText(hwndDlg, IDD_RESTZEIT, hStr);

    if (RZ.wHour == 0 && RZ.wMinute == 0 && RZ.wSecond == 0)
    {
        if (AlarmDlg == 0)
        {
            DialogBox(NULL, MAKEINTRESOURCE(DLG_ALARM), hwndDlg, (DLGPROC)DlgProcAlarm);
        }
    }
}

/****************************************************************************
 *
 *     FUNCTION: SetColor
 *
 *     PURPOSE:  Set the colors used to paint controls in OnCtlColor....
 *
 *     PARAMS:   COLORREF TxtColr - Desired text color
 *               COLORREF BkColr - Desired back color
 *               HDC hdc - Handle of a device context
 *
 *     RETURNS:  HBRUSH - A reusable brush object
 *
 * History:
 *                August '07 - Created
 *
\****************************************************************************/

HBRUSH SetBkfColor(COLORREF TxtColr, COLORREF BkColr, HDC hdc)
{
    static HBRUSH ReUsableBrush;

    DeleteObject(ReUsableBrush);
    ReUsableBrush = CreateSolidBrush(BkColr);
    SetTextColor(hdc, TxtColr);
    SetBkColor(hdc, BkColr);
    return ReUsableBrush;
}

//****************************************************************************
// Setzen der Farben                                                        **
//****************************************************************************
void SetColors(HWND hwndCtl, HDC wParam)
{
    static COLORREF old_back = 0;
    int id;
    int delta;
    int abstand = 24*60;
    int tx_r,tx_g,tx_b;
    int bg_r,bg_g,bg_b;

    id = GetWindowLong(hwndCtl, GWL_ID);

    if (id == IDD_RESTZEIT)
    {
        delta = (RZ.wHour * 60 + RZ.wMinute);
        // abstand = (DZ.wHour * 60 + DZ.wMinute);
        if (0 == abstand)
            abstand = 60;  // Korrektur bei Abstand 0

        if (erreicht)
        {
            // gr�n
            bg_r = (127 * (abstand - delta)) / abstand;
            bg_g = ( 15 * (abstand - delta)) / abstand + 112;
            bg_b = 0;
        }
        else
        {
            // rot
            // bg_r = ( 15 * (abstand - delta)) / abstand + 112;
            bg_r = (127 * (          delta)) / abstand;
            bg_g = 0;
            bg_b = (127 * (abstand - delta)) / abstand;
        }
        gBackgroundColor = RGB(bg_r, bg_g, bg_b);

        // Schriftfarbe Schwarz/Wei� oder bunt :)
        if (blackwhite)
        {
            if ((bg_r + bg_g + bg_b) >= 384)
            {
                tx_r = tx_g = tx_b = 0;
            }
            else
            {
                tx_r = tx_g = tx_b = 255;
            }
        }
        else
        {
            tx_r = 127 + bg_r % 128;
            tx_g = 127 + bg_g % 128;
            tx_b = 127 + bg_b % 128;
        }

        gForegroundColor = RGB(tx_r, tx_g, tx_b);
        if (gBackgroundColor != old_back)
        {
            old_back = gBackgroundColor;
            SetTextColor(wParam, gBackgroundColor);
            SetBkColor(wParam, gForegroundColor);
            Refresh(uhren[0].hWnd);
            Refresh(uhren[1].hWnd);
            Refresh(uhren[2].hWnd);
        }
    }

    SetBkMode(wParam, OPAQUE);
}

//****************************************************************************
// Speichern des aktuellen Fensters und Endzeit                             **
//****************************************************************************
void SaveRect(void)
{
    FILE * f;
    char hStr[150];

    f = fopen(IniName, "w");
    if (f != NULL)
    {
        sprintf(hStr, "%02d:%02d:%02d\n", EZ.wHour, EZ.wMinute, EZ.wSecond);
        fwrite(hStr, 1, strlen(hStr), f);

        sprintf(hStr, "%s\n", alarmgrund);
        fwrite(hStr, 1, strlen(hStr), f);

        // Positionen speichern
        sprintf(hStr, "%ld,%ld,%ld,%ld, %d,%d\n",
        uhren[0].rWndDlg.left, uhren[0].rWndDlg.top,
        uhren[0].rWndDlg.right, uhren[0].rWndDlg.bottom,
        uhren[0].hide, uhren[0].top);
        fwrite(hStr, 1, strlen(hStr), f);
        sprintf(hStr, "%ld,%ld,%ld,%ld, %d,%d\n",
        uhren[1].rWndDlg.left, uhren[1].rWndDlg.top,
        uhren[1].rWndDlg.right, uhren[1].rWndDlg.bottom,
        uhren[1].hide, uhren[1].top);
        fwrite(hStr, 1, strlen(hStr), f);
        sprintf(hStr, "%ld,%ld,%ld,%ld, %d,%d\n",
        uhren[2].rWndDlg.left, uhren[2].rWndDlg.top,
        uhren[2].rWndDlg.right, uhren[2].rWndDlg.bottom,
        uhren[2].hide, uhren[2].top);
        fwrite(hStr, 1, strlen(hStr), f);

        fclose(f);
    }
}

//****************************************************************************
// Berechnen der Restzeit aus der Jetzt- und Endzeit                        **
//****************************************************************************
void CalcRestZeit(SYSTEMTIME J, SYSTEMTIME E, SYSTEMTIME *rz)
{
    unsigned int iJ, iE, iR;

    memset(rz, 0, sizeof(*rz));

    iJ = (J.wHour * 60 + J.wMinute) * 60 + J.wSecond;
    iE = (E.wHour * 60 + E.wMinute) * 60 + E.wSecond;

    if(erreicht)
    {
        if(iJ > iE)
        {
            iR = iJ - iE;
        }
        else
        {
            iR = iJ + 24 * 3600 - iE;
        }
    }
    else
    {
        // nicht erreicht
        if(iJ > iE)
        {
            iR = iE + 24 * 3600 - iJ;
        }
        else
        {
            iR = iE - iJ;
        }
    }

    rz->wSecond = iR % 60;
    rz->wMinute = (iR / 60) % 60;
    rz->wHour = (unsigned short)(iR / 3600) % 24;
}
/****************************************************************************
 *                                                                          *
 * Function: WinMain                                                        *
 *                                                                          *
 * Purpose : Initialize the application.  Register a window class,          *
 *           create and display the main window and enter the               *
 *           message loop.                                                  *
 *                                                                          *
 * History : Date      Reason                                               *
 *           00/00/00  Created                                              *
 *                                                                          *
 ****************************************************************************/

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    INITCOMMONCONTROLSEX icc;
    WNDCLASSEX wcx;
    MSG Msg;
    int wHour, wMinute, wSecond;
    char hStr[200];
    RECT r;
    FILE * f;

    ghInstance = hInstance;
    // Namen f�r ini ermitteln
    GetModuleFileName(NULL, IniName, sizeof(IniName));
    strcpy(&IniName[strlen(IniName) - 3], "ini");

    /* Initialize common controls. Also needed for MANIFEST's */
    /*
     * TODO: set the ICC_???_CLASSES that you need.
     */
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_WIN95_CLASSES /*|ICC_COOL_CLASSES|ICC_DATE_CLASSES|ICC_PAGESCROLLER_CLASS|ICC_USEREX_CLASSES*/ ;
    InitCommonControlsEx(&icc);

    /* Load Rich Edit control support */
    /*
     * TODO: uncomment one of the lines below, if you are using a Rich Edit control.
     */
    // LoadLibrary(_T("riched32.dll"));  // Rich Edit v1.0
    // LoadLibrary(_T("riched20.dll"));  // Rich Edit v2.0, v3.0

    // Ini-Datei lesen und Werte setzen
    f = fopen(IniName, "r");

    if (f != NULL)
    {
        // Endzeit einlesen
        fgets(hStr, 50, f);
        if (3 == sscanf(hStr, "%d:%d:%d", &wHour, &wMinute, &wSecond))
        {
            EZ.wHour = wHour % 24;
            EZ.wMinute = wMinute % 60;
            EZ.wSecond = wSecond % 60;
        }

        // Grund f�r Alarm lesen
        fgets(alarmgrund, 99, f);
        for (unsigned i = 0; i < strlen(alarmgrund); i++)
        {
            if ('\n' == alarmgrund[i])
            {
                alarmgrund[i] = 0;
                break;
            }
        }

        // Rechteck einlesen
        for (int i = 0; i < 3; i++)
        {
            fgets(hStr, 50, f);
            if (4 <= sscanf(hStr, "%ld,%ld,%ld,%ld, %d,%d",
                            &r.left, &r.top, &r.right, &r.bottom,
                            &uhren[i].hide, &uhren[i].top))
            {
                // RECT hr;

                // GetWindowRect(hWndDlg[i], &hr);
                // hr.right  -= hr.left;
                // hr.bottom -= hr.top;
                // MoveWindow(hWndDlg[i], r.left, r.top, hr.right, hr.bottom, TRUE);
                uhren[i].rWndDlg = r;
            }
        }

        // Datei schlie�en
        fclose(f);
    }

    GetParams(lpszCmdLine);

    /* Get system dialog information */
    wcx.cbSize = sizeof(wcx);
    if (!GetClassInfoEx(NULL, MAKEINTRESOURCE(32770), &wcx))
        return 0;

        /* Add our own stuff */
    wcx.hInstance = hInstance;
    wcx.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDR_ICO_MAIN));
    wcx.lpszClassName = _T("WinUhr2Class");

    // Tray-Icons laden
    hBackIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDR_ICO_TRAY3), IMAGE_ICON, ICONSIZE, ICONSIZE, 0);
    hBigIcon  = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDR_ICO_TRAY4), IMAGE_ICON, BIGICONSIZE, BIGICONSIZE, 0);

    if (!RegisterClassEx(&wcx))
        return 0;

        /* The user interface is a modal dialog box */

        //hWindow  = CreateWindow("myWinUhr2","WinUhr2",0,100,100,1,1,NULL,NULL,hInstance,NULL);
    uhren[0].hWnd = CreateDialog(hInstance, MAKEINTRESOURCE(DLG_MAIN_X), NULL, (DLGPROC)DlgProcMain);
    uhren[1].hWnd = CreateDialog(hInstance, MAKEINTRESOURCE(DLG_MAIN_Y), NULL, (DLGPROC)DlgProcMain);
    uhren[2].hWnd = CreateDialog(hInstance, MAKEINTRESOURCE(DLG_MAIN_Z), NULL, (DLGPROC)DlgProcMain);
    Refresh(uhren[0].hWnd);
    Refresh(uhren[1].hWnd);
    Refresh(uhren[2].hWnd);
    //DialogBox(hInstance, MAKEINTRESOURCE(DLG_MAIN), NULL, (DLGPROC)DlgProcMain);

    while (GetMessage(&Msg, NULL, 0, 0) > 0)
    {
#if 1
        //if(!IsDialogMessage(hwndDlg, &Msg))
        //{
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
        //}
#else
        if (!IsDialogMessage(hWnd[0], &Msg))
        {
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
        }
        if (!IsDialogMessage(hWnd[1], &Msg))
        {
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
        }
        if (!IsDialogMessage(hWnd[2], &Msg))
        {
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
        }
#endif
    }

    return TRUE;
}

/****************************************************************************
 *                                                                          *
 * Function: MainDlgProc                                                    *
 *                                                                          *
 * Purpose : Process messages for the Main dialog.                          *
 *                                                                          *
 * History : Date      Reason                                               *
 *           00/00/00  Created                                              *
 *                                                                          *
 ****************************************************************************/

static LRESULT CALLBACK DlgProcMain(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    char hStr[100];
    SYSTEMTIME Jetzt;
    POINT pt;
    static int done = 0;
    static int sU = 0;
    int selUhr = -1;
    int i;

    memset(hStr, 0, 100);

    for (i = 0;i < 3;i++)
    {
        if (uhren[i].hWnd == hwndDlg)
        {
            selUhr = i;
            break;
        }
    }

    switch (uMsg)
    {
        case WM_INITDIALOG:
            // Timer starten    
            SetTimer(hwndDlg, TIMER_UHR , 250, NULL);
            SetTimer(hwndDlg, TIMER_UHRA, 500, NULL);

            // Popup-Men� erzeugen
            if (NULL == hPopupMenu)
            {
                hPopupMenu = CreatePopupMenu();
                AppendMenu(hPopupMenu, MF_STRING, IDM_EDIT, "&Eingabe Endzeit");
                AppendMenu(hPopupMenu, MF_STRING | MF_CHECKED, IDM_RESTZEIT, "&Restzeit bei Minimiert");
                AppendMenu(hPopupMenu, MF_SEPARATOR, 0, 0);
                //AppendMenu(hPopupMenu, MF_STRING            ,IDM_MINI       , "&Minimieren");
                AppendMenu(hPopupMenu, MF_STRING, IDM_HIDEX, "&Verstecken B");
                AppendMenu(hPopupMenu, MF_STRING, IDM_HIDEY, "&Verstecken D");
                AppendMenu(hPopupMenu, MF_STRING, IDM_HIDEZ, "&Verstecken A");
                AppendMenu(hPopupMenu, MF_SEPARATOR, 0, 0);
                AppendMenu(hPopupMenu, MF_STRING, IDM_TOPX, "&TopMost B");
                AppendMenu(hPopupMenu, MF_STRING, IDM_TOPY, "&TopMost D");
                AppendMenu(hPopupMenu, MF_STRING, IDM_TOPZ, "&TopMost A");
                AppendMenu(hPopupMenu, MF_SEPARATOR, 0, 0);
                AppendMenu(hPopupMenu, MF_STRING, IDM_RESTORE, "&Wiederherstellen");
                AppendMenu(hPopupMenu, MF_SEPARATOR, 0, 0);
                AppendMenu(hPopupMenu, MF_STRING, IDM_EXIT, "&Ende");
            }

            // System-Men� erzeugen
            if (selUhr == -1)
            {
                selUhr = sU++;
            }
            uhren[selUhr].hSMenu = GetSystemMenu(hwndDlg, FALSE);
            AppendMenu(uhren[selUhr].hSMenu, MF_SEPARATOR, 0, 0);
            AppendMenu(uhren[selUhr].hSMenu, MF_STRING, IDM_EDIT, "&Eingabe Endzeit");
            //AppendMenu(uhren[selUhr].hSMenu, MF_STRING | MF_CHECKED, IDM_RESTZEIT, "&Restzeit bei Minimiert");
            AppendMenu(uhren[selUhr].hSMenu, MF_SEPARATOR, 0, 0);
            AppendMenu(uhren[selUhr].hSMenu, MF_STRING, IDM_TOP, "&TopMost");
            AppendMenu(uhren[selUhr].hSMenu, MF_STRING, IDM_HIDE, "&Verstecken");

            SetColors(hwndDlg, (HDC)wParam);
            SetBkfColor(gForegroundColor, gBackgroundColor, (HDC)wParam);

            // Tray-Icon erzeugen
            if (nid.cbSize == 0)
            {
                nid.cbSize = sizeof(NOTIFYICONDATA);  //Most API Structs require this
                nid.hWnd = hwndDlg;
                nid.uID = IDR_ICO_TRAY3;
                nid.uFlags = NIF_ICON + NIF_MESSAGE + NIF_TIP;  //Flags to set requires fields
                nid.uCallbackMessage = WM_SHELLNOTIFY;  // Message ID sent when the pointer enters Tray icon area
                nid.hIcon = LoadIcon(ghInstance, MAKEINTRESOURCE(IDR_ICO_MAIN));  //Load Icon for tray
                lstrcpy(nid.szTip, "WinUhr2");  //Tray Icon Tool Tip
                Shell_NotifyIcon(NIM_ADD, &nid);  //Show the Icon
            }

            // SendDlgItemMessage(hwndDlg,IDI_ACLOCK,STM_SETICON,(WPARAM)nid.hIcon,(LPARAM)0);

            return TRUE;

        case WM_SHELLNOTIFY:
            //Called when pointer entry tray icon area
            if (wParam == IDR_ICO_TRAY3)
            {
                //Show PopUp menu if right button down
                if (lParam == WM_RBUTTONDOWN)
                {
                    GetCursorPos(&pt);
                    SetForegroundWindow(hwndDlg);
                    TrackPopupMenu(hPopupMenu, TPM_RIGHTALIGN, pt.x, pt.y, 0, hwndDlg, 0);
                    PostMessage(hwndDlg, WM_NULL, 0, 0);
                    return TRUE;
                }
                else if (WM_LBUTTONDBLCLK == lParam)
                {
                    // Restore Window
                    SendMessage(hwndDlg, WM_COMMAND, IDM_RESTORE, 0);
                }
            }
            break;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDM_EXIT:
                    //SendMessage(hwndDlg, WM_CLOSE, 0, 0);
                    SendMessage(uhren[0].hWnd, WM_CLOSE, 0, 0);
                    SendMessage(uhren[1].hWnd, WM_CLOSE, 0, 0);
                    SendMessage(uhren[2].hWnd, WM_CLOSE, 0, 0);
                    exit(0);
                    return TRUE;

                case IDM_RESTORE:
                    //minimized = 0;
                    uhren[0].hide = 0;
                    uhren[1].hide = 0;
                    uhren[2].hide = 0;
                    CheckMenuItem(hPopupMenu, IDM_HIDEX, MF_UNCHECKED);
                    CheckMenuItem(hPopupMenu, IDM_HIDEY, MF_UNCHECKED);
                    CheckMenuItem(hPopupMenu, IDM_HIDEZ, MF_UNCHECKED);
                    ShowWindow(uhren[0].hWnd, SW_RESTORE);
                    ShowWindow(uhren[1].hWnd, SW_RESTORE);
                    ShowWindow(uhren[2].hWnd, SW_RESTORE);
                    break;

                case IDM_RESTZEIT:
                // hSysMenu = GetSystemMenu(hwndDlg, FALSE);
                    show_rest = !show_rest;
                    if (show_rest)
                    {
                        CheckMenuItem(uhren[selUhr].hSMenu, IDM_RESTZEIT, MF_CHECKED);
                        CheckMenuItem(hPopupMenu, IDM_RESTZEIT, MF_CHECKED);
                    }
                    else
                    {
                        CheckMenuItem(uhren[selUhr].hSMenu, IDM_RESTZEIT, MF_UNCHECKED);
                        CheckMenuItem(hPopupMenu, IDM_RESTZEIT, MF_UNCHECKED);
                    }
                    return TRUE;

                case IDM_EDIT:
                    DialogBox(ghInstance, MAKEINTRESOURCE(DLG_EDIT), hwndDlg, (DLGPROC)
                    DlgProcEdit);
                    SaveRect();
                    return TRUE;

                case IDM_HIDEX:
                {
                    uhren[0].hide = !uhren[0].hide;
                    CheckMenuItem(hPopupMenu, IDM_HIDEX, uhren[0].hide?MF_CHECKED:MF_UNCHECKED);
                    ShowWindow(uhren[0].hWnd, uhren[0].hide?SW_HIDE:SW_SHOW);
                    ShowWindow(uhren[0].hWnd, uhren[0].hide?SW_HIDE:SW_RESTORE);
                    // Restore Window
                    SendMessage(uhren[0].hWnd, WM_COMMAND, IDM_RESTORE, 0);
                }
                    return TRUE;
                    break;

                case IDM_HIDEY:
                {
                    uhren[1].hide = !uhren[1].hide;
                    CheckMenuItem(hPopupMenu, IDM_HIDEY, uhren[1].hide?MF_CHECKED:MF_UNCHECKED);
                    ShowWindow(uhren[1].hWnd, uhren[1].hide?SW_HIDE:SW_RESTORE);
                    // Restore Window
                    SendMessage(uhren[1].hWnd, WM_COMMAND, IDM_RESTORE, 0);
                }
                    return TRUE;
                    break;

                case IDM_HIDEZ:
                {
                    uhren[2].hide = !uhren[2].hide;
                    CheckMenuItem(hPopupMenu, IDM_HIDEZ, uhren[2].hide?MF_CHECKED:MF_UNCHECKED);
                    ShowWindow(uhren[2].hWnd, uhren[2].hide?SW_HIDE:SW_MAXIMIZE);
                    // Restore Window
                    SendMessage(uhren[2].hWnd, WM_COMMAND, IDM_RESTORE, 0);
                }
                    return TRUE;
                    break;
                case IDM_TOPX:
                {
                    uhren[0].top = !uhren[0].top;
                    CheckMenuItem(hPopupMenu, IDM_TOPX, uhren[0].top?MF_CHECKED:MF_UNCHECKED);
                    CheckMenuItem(uhren[0].hSMenu, IDM_TOP, uhren[0].top?MF_CHECKED:MF_UNCHECKED);
                    SetWindowPos(uhren[0].hWnd, uhren[0].top?HWND_TOPMOST:HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                }
                    return TRUE;

                case IDM_TOPY:
                {
                    uhren[1].top = !uhren[1].top;
                    CheckMenuItem(hPopupMenu, IDM_TOPY, uhren[1].top?MF_CHECKED:MF_UNCHECKED);
                    CheckMenuItem(uhren[1].hSMenu, IDM_TOP, uhren[1].top?MF_CHECKED:MF_UNCHECKED);
                    SetWindowPos(uhren[1].hWnd, uhren[1].top?HWND_TOPMOST:HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                }
                    return TRUE;

                case IDM_TOPZ:
                {
                    uhren[2].top = !uhren[2].top;
                    CheckMenuItem(hPopupMenu, IDM_TOPZ, uhren[2].top?MF_CHECKED:MF_UNCHECKED);
                    CheckMenuItem(uhren[2].hSMenu, IDM_TOP, uhren[2].top?MF_CHECKED:MF_UNCHECKED);
                    SetWindowPos(uhren[2].hWnd, uhren[2].top?HWND_TOPMOST:HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                }
                    return TRUE;

            }
            break;

        case WM_SYSCOMMAND:
            switch (wParam)
            {
                case SC_RESTORE:
                    ShowWindow(hwndDlg, SW_RESTORE);
                    break;

                case IDM_RESTZEIT:
                // hSysMenu = GetSystemMenu(hwndDlg, FALSE);
                    show_rest = !show_rest;
                    if (show_rest)
                    {
                        CheckMenuItem(uhren[selUhr].hSMenu, IDM_RESTZEIT, MF_CHECKED);
                    }
                    else
                    {
                        CheckMenuItem(uhren[selUhr].hSMenu, IDM_RESTZEIT, MF_UNCHECKED);
                    }
                    return TRUE;

                case IDM_EXIT:
                    SendMessage(hwndDlg, WM_CLOSE, 0, 0);
                    return TRUE;

                case IDM_EDIT:
                    DialogBox(ghInstance, MAKEINTRESOURCE(DLG_EDIT), hwndDlg, (DLGPROC)DlgProcEdit);
                    return TRUE;

                case IDM_HIDE:
                    uhren[selUhr].hide = !uhren[selUhr].hide;
                    CheckMenuItem(hPopupMenu, IDM_HIDEX + selUhr, uhren[selUhr].hide?MF_CHECKED:MF_UNCHECKED);
                    ShowWindow(uhren[selUhr].hWnd, !uhren[selUhr].hide);
                    return TRUE;

                case IDM_TOP:
                    uhren[selUhr].top = !uhren[selUhr].top;
                    CheckMenuItem(uhren[selUhr].hSMenu, IDM_TOP, uhren[selUhr].top?MF_CHECKED:MF_UNCHECKED);
                    CheckMenuItem(hPopupMenu, IDM_TOPX + selUhr, uhren[selUhr].top?MF_CHECKED:MF_UNCHECKED);
                    SetWindowPos(hwndDlg, uhren[selUhr].top?HWND_TOPMOST:HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
                    return TRUE;
            }
            break;

        case WM_TIMER:
            // Analoguhr
            if (TIMER_UHRA == wParam)
            {
               static int xx = 0;
                if (uhren[2].hide == 0)
                {
                    static HICON hBTempIcon = NULL;
                    hBTempIcon = CreateBigTimeIcon(hwndDlg);
                    if (NULL != hBTempIcon)
                    {
                        if (NULL != hBIcon)
                        {
                            DestroyIcon(hBIcon);
                        }
                        hBIcon = hBTempIcon;
                        //SendDlgItemMessage(uhren[2].hWnd, IDR_ICO_MAIN, STM_SETICON, (WPARAM)hBTempIcon, (LPARAM)0);
                        SendDlgItemMessage(uhren[2].hWnd, IDI_BCLOCK, STM_SETICON, (WPARAM)hBIcon, (LPARAM)0);
                        //SetClassLong(uhren[2].hWnd, GCL_HICON, (LONG)hBTempIcon);
                    }
                }
                if(xx<3)
                {
                    //TODO
                    ShowWindow(hwndDlg, uhren[selUhr].hide?SW_HIDE:SW_SHOW);
                    Refresh(hwndDlg);
                    xx++;
                }
                return TRUE;
            }
            AktOutput(hwndDlg);
            // Tray-Tip aktualisieren
            GetLocalTime(&Jetzt);
            sprintf(nid.szTip, "%2s: %02d.%02d.%04d\nJetzt: %02d:%02d:%02d\nRest: %02d:%02d:%02d\nEnde: %02d:%02d:%02d",
                    wota[Jetzt.wDayOfWeek], Jetzt.wDay, Jetzt.wMonth, Jetzt.wYear /*%100*/ ,
                    Jetzt.wHour, Jetzt.wMinute, Jetzt.wSecond,
                    RZ.wHour, RZ.wMinute, RZ.wSecond,
                    EZ.wHour, EZ.wMinute, EZ.wSecond);
            nid.hIcon = hIcon;

            // Icons setzen

            static HICON hTempIcon;
            hTempIcon = CreateTimeIcon(hwndDlg);
            if (NULL != hTempIcon)
            {
                if (NULL != hIcon)
                {
                    DestroyIcon(hIcon);
                }
                hIcon = hTempIcon;
                nid.hIcon = hIcon;

                SendDlgItemMessage(uhren[0].hWnd, IDR_ICO_MAIN, STM_SETICON, (WPARAM)hIcon, (LPARAM)0);
                SendDlgItemMessage(uhren[0].hWnd, IDI_ACLOCK, STM_SETICON, (WPARAM)hIcon, (LPARAM)0);
                //SetClassLong(uhren[0].hWnd, GCL_HICON, (size_t)hIcon);
                SendDlgItemMessage(uhren[1].hWnd, IDR_ICO_MAIN, STM_SETICON, (WPARAM)hIcon, (LPARAM)0);
                //SendDlgItemMessage(uhren[1].hWnd, IDI_ACLOCK, STM_SETICON, (WPARAM)hIcon, (LPARAM)0);
                //SetClassLong(uhren[1].hWnd, GCL_HICON, (size_t)hIcon);
                SendDlgItemMessage(uhren[2].hWnd, IDR_ICO_MAIN, STM_SETICON, (WPARAM)hIcon, (LPARAM)0);
                SendMessage(uhren[0].hWnd,WM_SETICON, ICON_SMALL,(LPARAM)hIcon);
                SendMessage(uhren[0].hWnd,WM_SETICON, ICON_BIG  ,(LPARAM)hIcon);
                SendMessage(uhren[1].hWnd,WM_SETICON, ICON_SMALL,(LPARAM)hIcon);
                SendMessage(uhren[1].hWnd,WM_SETICON, ICON_BIG  ,(LPARAM)hIcon);
                SendMessage(uhren[2].hWnd,WM_SETICON, ICON_SMALL,(LPARAM)hIcon);
                SendMessage(uhren[2].hWnd,WM_SETICON, ICON_BIG  ,(LPARAM)hIcon);
            }

            // Tray aktuallisieren
            Shell_NotifyIcon(NIM_MODIFY, &nid);

            if (!done)
            {
                if ((uhren[0].hWnd != 0) &&
                    (uhren[1].hWnd != 0) &&
                    (uhren[2].hWnd != 0))
                {
                    RECT hr;
                    for (i = 0;i < 3; i++)
                    {
                        GetWindowRect(uhren[i].hWnd, &hr);
                        hr.right -= hr.left;
                        hr.bottom -= hr.top;
                        MoveWindow(uhren[i].hWnd, uhren[i].rWndDlg.left, uhren[i].rWndDlg.top, hr.right, hr.bottom, TRUE);
                        char hStr[200];
                        sprintf(hStr,"index:%2d pos x/y:%2ld,%2ld\n",i,uhren[i].rWndDlg.left,uhren[i].rWndDlg.top);
                        OutputDebugString(hStr);
                    }
                }
                done = !done;
            }
            return TRUE;

        case WM_CTLCOLORSTATIC:
            SetColors((HWND)lParam, (HDC) wParam);
            SendMessage(hwndDlg, WM_CTLCOLORDLG, wParam, lParam);
            return (size_t)GetSysColorBrush(COLOR_3DFACE);
            break;

        case WM_CTLCOLORDLG:
            return SetDlgMsgResult((HWND)lParam, uMsg, SetBkfColor(gForegroundColor, gBackgroundColor, (HDC) wParam));
            break;

        case WM_LBUTTONDBLCLK:
            DialogBox(ghInstance, MAKEINTRESOURCE(DLG_EDIT), hwndDlg, (DLGPROC)DlgProcEdit);
            return TRUE;
            break;

        case WM_RBUTTONUP:
        //SendMessage(hwndDlg, WM_SYSCOMMAND, IDM_MINI , 0);
            for (i = 0; i < 3; i++)
            {
                if (hwndDlg == uhren[i].hWnd)
                {
                    uhren[i].hide = !uhren[i].hide;
                    CheckMenuItem(hPopupMenu, IDM_HIDEX + i, uhren[i].hide?MF_CHECKED:MF_UNCHECKED);
                    ShowWindow(uhren[i].hWnd, !uhren[i].hide);
                }
            }
            return TRUE;
            break;

        case WM_MOVE:
        case WM_WINDOWPOSCHANGED:
            for(int i=0; i<3 ;i++)
            {
                if (uhren[i].hide == 0)
                {
                    if (done)
                    {
                        GetWindowRect(uhren[i].hWnd, &uhren[i].rWndDlg);
                    }
                }
            }
            SaveRect();
            return TRUE;
            break;

        case WM_CLOSE:
            KillTimer(hwndDlg, TIMER_UHR);
            KillTimer(hwndDlg, TIMER_UHRA);
            SaveRect();
            nid.cbSize = sizeof(NOTIFYICONDATA);
            nid.hWnd = hwndDlg;
            nid.uID = IDR_ICO_TRAY3;
            nid.uFlags = NIF_ICON + NIF_MESSAGE + NIF_TIP;
            Shell_NotifyIcon(NIM_DELETE, &nid);
            uhren[selUhr].hWnd = NULL;
            EndDialog(hwndDlg, 0);
            exit(0);
            return TRUE;
    }

    return FALSE;
}

static LRESULT CALLBACK DlgProcEdit(HWND hwndEDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    char hStr[100];
    int items;
    int h,m,s;

    switch (uMsg)
    {
        case WM_INITDIALOG:
            sprintf(hStr, "%02d:%02d:%02d", EZ.wHour, EZ.wMinute, EZ.wSecond);
            SetDlgItemText(hwndEDlg, IDD_EDIT_ZEIT, hStr);
            SetDlgItemText(hwndEDlg, IDD_EDIT_GRUND, alarmgrund);
            return TRUE;

        case WM_SIZE:
            /*
             * TODO: Add code to process resizing, when needed.
             */
            return TRUE;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDOK:
                    GetDlgItemText(hwndEDlg, IDD_EDIT_ZEIT, hStr, 99);
                    items=sscanf(hStr, "%u:%u:%u", &h, &m, &s);
                    // eine Einfache Syntaxpr�fung der Eingabe  ** aN 09.08.2023
                    switch(items)
                    {
                        case 3:
                            EZ.wHour   = h % 24;
                            EZ.wMinute = m % 60;
                            EZ.wSecond = s % 60;
                            erreicht = 0;
                            break;
                        case 2:
                            EZ.wHour   = h % 24;
                            EZ.wMinute = m % 60;
                            EZ.wSecond = 0;
                            erreicht = 0;
                            break;
                        case 1:
                            GetLocalTime(&EZ);
                            EZ.wSecond = 0;
                            AddTime(h);
                            erreicht = 0;
                            break;
                        default:
                            break;
                    }

                    

                    GetDlgItemText(hwndEDlg, IDD_EDIT_GRUND, alarmgrund, 99);

                    EndDialog(hwndEDlg, TRUE);
                    return TRUE;

                case IDCANCEL:
                    EndDialog(hwndEDlg, FALSE);
                    return TRUE;
            }
            break;

        case WM_CLOSE:
            EndDialog(hwndEDlg, 0);
            return TRUE;

            /*
             * TODO: Add more messages, when needed.
             */
    }

    return FALSE;
}

// zur Endzeit EZ 'diff'-Minuten addieren
void AddTime(int diff)
{
    EZ.wMinute += (short)diff;
    if (EZ.wMinute >= 60)
    {
        EZ.wHour += EZ.wMinute/60;
        EZ.wMinute %= 60;
        if (EZ.wHour >= 24)
        {
            EZ.wHour %= 24;
        }
    }
    erreicht = 0;
}

static LRESULT CALLBACK DlgProcAlarm(HWND hwndADlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    char hStr[120];

    switch (uMsg)
    {
        case WM_INITDIALOG:
            AlarmDlg = 1;
            erreicht = 1;
            sprintf(hStr, "%02d:%02d:%02d   %s", EZ.wHour, EZ.wMinute, EZ.wSecond, alarmgrund);
            SetDlgItemText(hwndADlg, IDD_ALARM_TEXT, hStr);
            return TRUE;

        case WM_COMMAND:
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDD_5MIN:
                    AddTime(5);
                    break;
                case IDD_15MIN:
                    AddTime(15);
                    break;
                case IDD_30MIN:
                    AddTime(30);
                    break;
                case IDD_60MIN:
                    AddTime(60);
                    break;
                case IDD_EDIT:
                    DialogBox(ghInstance, MAKEINTRESOURCE(DLG_EDIT), hwndADlg, (DLGPROC)DlgProcEdit);
                    break;
            }
            AlarmDlg = 0;
            EndDialog(hwndADlg, 0);
            return TRUE;

        case WM_RBUTTONDBLCLK:
        case WM_RBUTTONUP:
            DialogBox(ghInstance, MAKEINTRESOURCE(DLG_EDIT), hwndADlg, (DLGPROC)DlgProcEdit);
            AlarmDlg = 0;
            EndDialog(hwndADlg, 0);
            return TRUE;

        case WM_LBUTTONDBLCLK:
        case WM_LBUTTONUP:
        case WM_CLOSE:
            AlarmDlg = 0;
            EndDialog(hwndADlg, 0);
            return TRUE;
    }

    return FALSE;
}
