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

// Änderungsgeschichte
// 1.0.1.05 Im Edit-Dialog alte Endzeit als Vorgabe anzeigen    aN 27.03.2007
// 1.0.1.06 Aufruf für Edit-Dialog auf modal geändert           aN 27.03.2007
// 1.0.1.07 Speichern und Wiederherstellen der Fensterposition  aN 10.04.2007
// 1.0.1.08 Eine einfache Syntaxprüfung der Eingabe ergänzt     aN 10.04.2007
// 1.0.1.09 Eine einfache Syntaxprüfung der Eingabe ergänzt     aN 10.04.2007
// 1.0.1.10 Bei fscanf die Anzahl gelesener Parameter grprüft   aN 10.04.2007
// 1.0.1.11 Farbcodes für die Restzeit                          aN 11.04.2007
// 1.0.1.12 Farben angepaßt und in SetColors gechoben           aN 11.04.2007
// 1.0.1.13 Minimieren auf Klick-Rechts                         aN 16.04.2007
// 1.0.1.14 Experimente mit Menüs, AktOutput                    aN 16.04.2007
// 1.0.1.15 Anzeige umschaltbar bei Minimiert                   aN 03.05.2007
// 1.0.1.16 Bei minimiert, Endzeitdialog Systemmodal            aN 04.06.2007
// 1.0.1.17 Alarmdialog                                         aN 05.06.2007
// 1.0.1.18 Alarmdialog Modal und zentriert                     aN 14.03.2012
// 1.0.1.19 Grund für Alarm, Eingabe und Verwaltung             aN 17.04.2012
// 1.0.1.20 Animiertes Tray-Icon                                aN 30.07.2012
// 1.0.1.23 Reduzierung des Aktualisieren der Anzeige           aN 31.07.2012
// 1.0.1.24 Main-Icon an Anzeige anpassen                       aN 31.07.2012
// 1.0.1.25 GDI-Leak in TrayIcon-Erzeugung korrigiert           aN 01.08.2012
// 1.0.1.26 GDI-Leak korrigiert 2.Versuch                       aN 01.08.2012
// 1.0.1.27 Anderes Format für Titelzeile                       aN 19.09.2012
// 1.0.1.28 Nochmals anderes Format für Titelzeile              aN 05.10.2012
// 1.0.1.29 größeres Zeit-Icon                                  aN 07.04.2023
// 1.0.1.30 Parameter Z(eit) und "Grund"                        aN 15.04.2023
// 1.9.0.31 Einlesen von ini und Parameter korrigiert           aN 28.06.2023
// 1.9.0.32 INI folgt EXE-Namen                                 aN 30.06.2023
// 1.9.0.33 Nur ein Edit-Dialog bei Doppelklick                 aN 01.07.2023
// 1.9.0.34 Bei Alarm Verlängerungsbuttons                      aN 02.07.2023
// 1.9.0.35 gemeinsame Daten in eine Struktur                   aN 08.07.2023
// 1.9.0.36 Uhren einzeln verstecken und wieder holen           aN 09.07.2023
// 1.9.0.37 bei Alarm ein vierter Button Edit                   aN 10.07.2023

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
#define Refresh(A) RedrawWindow(A,NULL,NULL,RDW_ERASE|RDW_INVALIDATE|/*RDW_ALLCHILDREN|*/RDW_UPDATENOW);

#define BUF_SIZE            256
#define ICONSIZE            40 // 16
#define IMAGESIZE           38 // 14
#define STUNDE_COLOR        RGB(0,0,191)
#define MINUTE_COLOR        RGB(0,191,0)
#define WECKER_COLOR        RGB(255,0,0)
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
void CalcRestZeit(SYSTEMTIME j, SYSTEMTIME e,SYSTEMTIME *r);

/** Typen *******************************************************************/
typedef struct
{
    HWND hWndDlg;
    int hide;
    RECT rWndDlg;
} uhr;
/** Global variables ********************************************************/

static HANDLE ghInstance;
static COLORREF gBackgroundColor;
static COLORREF gForegroundColor;
static HICON    hBackIcon;
static HWND     hMainWnd = 0;
static int minimized = 0;
static int show_rest = 1;
static int hide = 1;
static int blackwhite = 0;
static char IniName[300] = "myWinUhr2.ini";

static HMENU hSysMenu = NULL;
static HMENU hPopupMenu = NULL;
static NOTIFYICONDATA nid = {0};
static HICON hIcon;

int AlarmDlg = 0;                       // Flag ob der Alarmdialog eingeschaltet ist

SYSTEMTIME DZ={2012, 0, 0,12,0,0,0,0};
SYSTEMTIME EZ={2012, 3,14,17,0,0,0,0};
SYSTEMTIME RZ={   0, 0, 0, 0,0,0,0,0};
char alarmgrund[100] = "";
char *wota[]= {"So\0nntag","Mo\0ntag","Di\0enstag","Mi\0ttwoch","Do\0nnerstag","Fr\0eitag","Sa\0mstag"};

uhr uhren[3]   = {{NULL,0,{0,0,0,0}},
                  {NULL,0,{0,0,0,0}},
                  {NULL,0,{0,0,0,0}}};

//****************************************************************************
//  Get Parameter
//****************************************************************************
void GetParams(char *szCmdline)
{
    int i = 0;
    char *cp;
    char hStr[200];

    if (szCmdline == NULL) return;

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
                if(szCmdline[i]=='=') i++;
                if(szCmdline[i]==':') i++;
                cp = hStr;
                while ((szCmdline[i] != ' ') && (szCmdline[i] != '\0'))
                    *(cp++) = szCmdline[i++];
                *cp = 0;
                sscanf(hStr, "%hu:%hu:%hu", &EZ.wHour, &EZ.wMinute, &EZ.wSecond);
                break;

            case 'S': // Spanne
            case 's':
                break;

            default:
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
    switch(flag)
    {
    case 0:     /* 0 bis 14 */
        pt->x = hmx;
        pt->y = hmy;
        break;
    case 1:     /* 15 bis 29 */
        pt->x = IMAGESIZE - hmy;
        pt->y = hmx;
        break;
    case 2:     /* 30 bis 44 */
        pt->x = IMAGESIZE - hmx;
        pt->y = IMAGESIZE - hmy;
        break;
    case 3:     /* 45 bis 59 */
        pt->x = hmy;
        pt->y = IMAGESIZE - hmx;
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

    POINT pt[2];
    SYSTEMTIME systim;

    int index, flag;
    static int gMinute = -1;
    static int gWStd = -1, gWMin = -1;
#if 0
    /* Tabelle für Stundenzeiger */
    const static int hx[] = {7,  8,  9, 10, 11, 11, 12, 12};
    const static int hy[] = {2,  2,  2,  3,  3,  4,  5,  6};
    /* Tabelle für Minutenzeiger */
    const static int mx[] = {7,  8,  9, 10, 10, 11, 12, 12, 12, 13, 13, 13, 14, 14, 14};
    const static int my[] = {0,  0,  0,  0,  1,  1,  1,  2,  2,  2,  3,  4,  4,  5,  6};
#else
    /* Tabelle für Stundenzeiger */
    const static int hy[] = {  4,  5,  6,  8, 10, 13, 16, 18,};
    const static int hx[] = { 22, 25, 27, 29, 31, 33, 34, 34,};
    /* Tabelle für Minutenzeiger */
    const static int my[] = {  0,  0,  1,  2,  3,  4,  5,  6,  8,  9, 11, 13, 15, 17, 19,};
    const static int mx[] = { 21, 23, 25, 27, 28, 30, 32, 33, 34, 35, 36, 37, 38, 38, 38,};
#endif
    /* Zeit ermitteln und auf Änderung prüfen */
    GetLocalTime(&systim);
    if(systim.wMinute == gMinute)
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

    /* Bitmap für Vordergrund erzeugen */
    hBitmap = CreateCompatibleBitmap(hdc, ICONSIZE, ICONSIZE);
    FillBitmap(mdc, hBitmap, 0, ICONSIZE, STUNDE_COLOR);

    /* Bitmap für Maske erzeugen */
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
    SelectObject(mdc, CreatePen(PS_SOLID, 3, STUNDE_COLOR));
    Polyline(mdc, pt, sizeof(pt) / sizeof(POINT));

    /* Icon zusammen setzen */
    IconList = ImageList_Create(ICONSIZE, ICONSIZE, ILC_COLOR | ILC_MASK, 4, 5);
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
    /* Bitmap für Vordergrund erzeugen */
    hBitmap = CreateCompatibleBitmap(hdc, ICONSIZE, ICONSIZE);
    FillBitmap(mdc, hBitmap, 0, ICONSIZE, MINUTE_COLOR);

    /* Bitmap für Maske erzeugen */
    hMaskBitmap = CreateCompatibleBitmap(hdc, ICONSIZE, ICONSIZE);
    FillBitmap(mdc, hMaskBitmap, 0, ICONSIZE, MASK_COLOR);
    ReleaseDC(hWnd, hdc);
    hRetBmp = SelectObject(mdc, hMaskBitmap);

    /* Minutenzeiger zeichnen */
    index = systim.wMinute % 15;
    flag = systim.wMinute / 15;
    ConvLinePoint(mx[index], my[index], &pt[1], flag);
    SelectObject(mdc, CreatePen(PS_SOLID, 2, MINUTE_COLOR));
    Polyline(mdc, pt, sizeof(pt) / sizeof(POINT));

    /* Zeit Icon zusammen setzen */
    SelectObject(mdc, hRetBmp);
    DeleteDC(mdc);
    ImageList_Add(IconList, hBitmap, hMaskBitmap);
    DeleteObject(hBitmap);
    DeleteObject(hMaskBitmap);

    // W E C K E R - Zeiger
    hdc = GetDC(hWnd);
    mdc = CreateCompatibleDC(hdc);

    /* Bitmap für Vordergrund erzeugen */
    hBitmap = CreateCompatibleBitmap(hdc, ICONSIZE, ICONSIZE);
    FillBitmap(mdc, hBitmap, 0, ICONSIZE, WECKER_COLOR);

    /* Bitmap für Maske erzeugen */
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
    SelectObject(mdc, CreatePen(PS_SOLID, 2, WECKER_COLOR));
    Polyline(mdc, pt, sizeof(pt) / sizeof(POINT));

    /* Maske setzen */
    FillBitmap(mdc, hMaskBitmap, IMAGESIZE / 2, IMAGESIZE / 2 + 1, MASK_COLOR);

    /* Wecker Icon zusammen setzen */
    SelectObject(mdc, hRetBmp);
    DeleteDC(mdc);
    ImageList_Add(IconList, hBitmap, hMaskBitmap);
    DeleteObject(hBitmap);
    DeleteObject(hMaskBitmap);

    // "Mergen" der Icons
    mIconList = ImageList_Merge( IconList, 0, IconList, 1, 0, 0);   // Originalicon + Stundenzeiger
    tIconList = ImageList_Merge(mIconList, 0, IconList, 2, 0, 0);   // + Minutenzeiger
    ImageList_Destroy(mIconList);
    mIconList = ImageList_Merge(tIconList, 0, IconList, 3, 0, 0);   // Wecker
    ImageList_Destroy(tIconList);

    hIcon = ImageList_GetIcon(mIconList, 0, ILD_NORMAL);
    ImageList_Destroy(mIconList);
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

    if (minimized)
    {
        if (show_rest)
        {
            sprintf(hStr, "%02d:%02d:%02d .", RZ.wHour, RZ.wMinute, RZ.wSecond);
            tag = -1;
        }
        //SendMessage(hwndDlg,  WM_SETTEXT, 0, (long int)hStr);
        SendMessage(uhren[0].hWndDlg, WM_SETTEXT, 0, (long int)hStr);
        SendMessage(uhren[1].hWndDlg, WM_SETTEXT, 0, (long int)hStr);
        SendMessage(uhren[2].hWndDlg, WM_SETTEXT, 0, (long int)hStr);
    }
    else
    {
        if (ST.wDay != tag)
        {
            sprintf(hStr,"X %2s: %02d.%02d.%04d",wota[ST.wDayOfWeek], ST.wDay, ST.wMonth, ST.wYear);
            //SendMessage(hwndDlg , WM_SETTEXT, 0, (long int) hStr);
            SendMessage(uhren[0].hWndDlg, WM_SETTEXT, 0, (long int)hStr);
            hStr[0] = 'Y';
            SendMessage(uhren[1].hWndDlg, WM_SETTEXT, 0, (long int)hStr);
            hStr[0] = 'Z';
            SendMessage(uhren[2].hWndDlg, WM_SETTEXT, 0, (long int)hStr);
            tag = ST.wDay;
        }
    }

    if (ez != (((EZ.wHour<<6)+EZ.wMinute)<<6)+EZ.wSecond)
    {
        sprintf(hStr, "%02d:%02d:%02d .", EZ.wHour, EZ.wMinute, EZ.wSecond);
        //SetDlgItemText(hwndDlg, IDD_ENDZEIT, hStr);
        SetDlgItemText(uhren[0].hWndDlg, IDD_ENDZEIT, hStr);
        SetDlgItemText(uhren[1].hWndDlg, IDD_ENDZEIT, hStr);
        SetDlgItemText(uhren[2].hWndDlg, IDD_ENDZEIT, hStr);
        ez = (((EZ.wHour<<6)+EZ.wMinute)<<6)+EZ.wSecond;
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
    int abstand;
    int tx_r,tx_g,tx_b;
    int bg_r,bg_g,bg_b;

    id = GetWindowLong(hwndCtl, GWL_ID);

    if (id == IDD_RESTZEIT)
    {
        delta   = (RZ.wHour*60 + RZ.wMinute);
        abstand = (DZ.wHour*60 + DZ.wMinute);
        if (0 == abstand) abstand = 60;         // Korrektur bei Abstand 0

        if (RZ.wDay > 0)
        {
            // grün
            bg_r = (240*(abstand-delta))/abstand;
            bg_g = ( 31*(abstand-delta))/abstand+224;
            bg_b = 0;
        }
        else
        {
            // rot
            bg_r = ( 31*(abstand-delta))/abstand+224;
            bg_g = (240*(abstand-delta))/abstand;
            bg_b = 0;
        }
        gBackgroundColor = RGB(bg_r,bg_g,bg_b);

        // Schriftfarbe Schwarz/Weiß oder bunt :)
        if (blackwhite)
        {
            if ((bg_r+bg_g+bg_b)>=384)
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
            tx_r = (128 + bg_r)%256;
            tx_g = (128 + bg_g)%256;
            tx_b = (128 + bg_b)%256;
        }

        gForegroundColor = RGB(tx_r,tx_g,tx_b);
        if (gBackgroundColor != old_back)
        {
            old_back = gBackgroundColor;
            //Refresh(hMainWnd);
            SetTextColor(wParam, gBackgroundColor);
            SetBkColor(wParam, gForegroundColor);
            Refresh(uhren[0].hWndDlg);
            Refresh(uhren[1].hWndDlg);
            Refresh(uhren[2].hWndDlg);
        }
    }

    SetBkMode(wParam,OPAQUE);
}

//****************************************************************************
// Speichern des aktuellen Fensters und Endzeit                             **
//****************************************************************************
void SaveRect(void)
{
    FILE *f;
    char hStr[50];

    f = fopen(IniName, "w");
    if (f != NULL)
    {
        sprintf(hStr, "%02d:%02d:%02d\n", EZ.wHour, EZ.wMinute, EZ.wSecond);
        fwrite(hStr, 1, strlen(hStr), f);

        sprintf(hStr, "%s\n", alarmgrund);
        fwrite(hStr, 1, strlen(hStr), f);

        sprintf(hStr, "%d,%d,%d,%d\n",
            uhren[0].rWndDlg.left,  uhren[0].rWndDlg.top,
            uhren[0].rWndDlg.right, uhren[0].rWndDlg.bottom);
        fwrite(hStr, 1, strlen(hStr), f);
        sprintf(hStr, "%d,%d,%d,%d\n",
            uhren[1].rWndDlg.left,  uhren[1].rWndDlg.top,
            uhren[1].rWndDlg.right, uhren[1].rWndDlg.bottom);
        fwrite(hStr, 1, strlen(hStr), f);
        sprintf(hStr, "%d,%d,%d,%d\n",
            uhren[2].rWndDlg.left,  uhren[2].rWndDlg.top,
            uhren[2].rWndDlg.right, uhren[2].rWndDlg.bottom);
        fwrite(hStr, 1, strlen(hStr), f);

        fclose(f);
    }
}

//****************************************************************************
// Berechnen der Restzeit aus der Jetzt- und Endzeit                        **
//****************************************************************************
void CalcRestZeit(SYSTEMTIME J, SYSTEMTIME E,SYSTEMTIME *rz)
{
    unsigned int iJ, iE, iR;

    memset(rz, 0, sizeof(*rz));

    iJ = (J.wHour*60 + J.wMinute)*60 + J.wSecond;
    iE = (E.wHour*60 + E.wMinute)*60 + E.wSecond;

    if (iJ > iE)
    {
        iR = iJ - iE;
        rz->wDay = 1;
    }
    else
    {
        iR = iE - iJ;
        rz->wDay = 0;
    }

    rz->wSecond = iR % 60;
    rz->wMinute = (iR / 60) % 60;
    rz->wHour   = (unsigned short)(iR / 3600);
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
    char hStr[100];
    RECT r;
    FILE *f;

    ghInstance = hInstance;
    // Namen für ini ermitteln
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
        fgets(hStr,50,f);
        if (3 == sscanf(hStr,"%ld:%ld:%ld", &wHour, &wMinute, &wSecond))
        {
            EZ.wHour   = wHour % 24;
            EZ.wMinute = wMinute % 60;
            EZ.wSecond = wSecond % 60;
        }

        // Grund für Alarm lesen
        fgets(alarmgrund,99,f);
        for(unsigned i=0; i<strlen(alarmgrund); i++)
        {
            if ('\n' == alarmgrund[i])
            {
                alarmgrund[i] = 0;
                break;
            }
        }

        // Rechteck einlesen
        for(int i=0; i<3; i++)
        {
            fgets(hStr,50,f);
            if (4 == sscanf(hStr, "%ld,%ld,%ld,%ld", &r.left, &r.top, &r.right, &r.bottom))
            {
                // RECT hr;
                    
                // GetWindowRect(hWndDlg[i], &hr);
                // hr.right  -= hr.left;
                // hr.bottom -= hr.top;
                // MoveWindow(hWndDlg[i], r.left, r.top, hr.right, hr.bottom, TRUE);
                uhren[i].rWndDlg = r;
            }
        }

        // Datei schließen
        fclose(f);
    }

    GetParams(lpszCmdLine);

    /* Get system dialog information */
    wcx.cbSize = sizeof(wcx);
    if(!GetClassInfoEx(NULL, MAKEINTRESOURCE(32770), &wcx))
        return 0;

        /* Add our own stuff */
    wcx.hInstance = hInstance;
    wcx.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDR_ICO_MAIN));
    wcx.lpszClassName = _T("WinUhrClass");

    // Tray-Icon laden
    hBackIcon = (HICON)LoadImage(hInstance, MAKEINTRESOURCE(IDR_ICO_TRAY3), IMAGE_ICON, ICONSIZE, ICONSIZE, 0);

    if(!RegisterClassEx(&wcx))
        return 0;

        /* The user interface is a modal dialog box */

        //hWindow  = CreateWindow("myWinUhr2","WinUhr2",0,100,100,1,1,NULL,NULL,hInstance,NULL);
    uhren[0].hWndDlg = CreateDialog(hInstance, MAKEINTRESOURCE(DLG_MAIN), NULL, (DLGPROC)DlgProcMain);
    uhren[1].hWndDlg = CreateDialog(hInstance, MAKEINTRESOURCE(DLG_MAIN), NULL, (DLGPROC)DlgProcMain);
    uhren[2].hWndDlg = CreateDialog(hInstance, MAKEINTRESOURCE(DLG_MAIN), NULL, (DLGPROC)DlgProcMain);
    //DialogBox(hInstance, MAKEINTRESOURCE(DLG_MAIN), NULL, (DLGPROC)DlgProcMain);

    while(GetMessage(&Msg, NULL, 0, 0) > 0)
    {
#if 1
        //if(!IsDialogMessage(hwndDlg, &Msg))
        //{
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
        //}
#else
        if(!IsDialogMessage(hWndDlg[0], &Msg))
        {
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
        }
        if(!IsDialogMessage(hWndDlg[1], &Msg))
        {
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
        }
        if(!IsDialogMessage(hWndDlg[2], &Msg))
        {
            TranslateMessage(&Msg);
            DispatchMessage(&Msg);
        }
#endif
    }

    if(uhren[0].hWndDlg == NULL && uhren[1].hWndDlg == NULL && uhren[2].hWndDlg == NULL)
    {
        SaveRect();
        exit(0);
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
    int i;

    memset(hStr, 0, 100);

    if (0 == hMainWnd)
    {
        hMainWnd = hwndDlg;
    }

    switch (uMsg)
    {
    case WM_INITDIALOG:
        // Timer starten
        SetTimer(hwndDlg, TIMER_UHR, 250, NULL);

        // Popup-Menü erzeugen
        if (NULL == hPopupMenu)
        {
            hPopupMenu = CreatePopupMenu();
            AppendMenu(hPopupMenu, MF_STRING            ,IDM_EDIT       , "&Eingabe Endzeit");
            AppendMenu(hPopupMenu, MF_STRING|MF_CHECKED ,IDM_RESTZEIT   , "&Restzeit bei Minimiert");
            AppendMenu(hPopupMenu, MF_SEPARATOR         ,0              , 0);
            AppendMenu(hPopupMenu, MF_STRING            ,IDM_MINI       , "&Minimieren");
            AppendMenu(hPopupMenu, MF_STRING            ,IDM_HIDEX      , "&Verstecken X");
            AppendMenu(hPopupMenu, MF_STRING            ,IDM_HIDEY      , "&Verstecken Y");
            AppendMenu(hPopupMenu, MF_STRING            ,IDM_HIDEZ      , "&Verstecken Z");
            AppendMenu(hPopupMenu, MF_STRING            ,IDM_RESTORE    , "&Wiederherstellen");
            AppendMenu(hPopupMenu, MF_SEPARATOR         ,0              , 0);
            AppendMenu(hPopupMenu, MF_STRING            ,IDM_EXIT       , "&Ende");
        }

        // System-Menü erzeugen
        hSysMenu = GetSystemMenu(hwndDlg, FALSE);
        AppendMenu(hSysMenu, MF_SEPARATOR           , 0             , 0);
        AppendMenu(hSysMenu, MF_STRING              , IDM_EDIT      , "&Eingabe Endzeit");
        AppendMenu(hSysMenu, MF_STRING | MF_CHECKED , IDM_RESTZEIT  , "&Restzeit bei Minimiert");
        AppendMenu(hSysMenu, MF_SEPARATOR           , 0             , 0);
        AppendMenu(hSysMenu, MF_STRING              , IDM_MINI      , "&Minimieren");
        AppendMenu(hSysMenu, MF_STRING              , IDM_HIDE      , "&Verstecken");

        SetColors(hwndDlg, (HDC)wParam);
        SetBkfColor(gForegroundColor, gBackgroundColor, (HDC)wParam);

        // Tray-Icon erzeugen
        if (nid.cbSize == 0)
        {
            nid.cbSize = sizeof(NOTIFYICONDATA); //Most API Structs require this
            nid.hWnd   = hwndDlg;
            nid.uID    = IDR_ICO_TRAY3;
            nid.uFlags = NIF_ICON+NIF_MESSAGE+NIF_TIP; //Flags to set requires fields
            nid.uCallbackMessage=WM_SHELLNOTIFY; // Message ID sent when the pointer enters Tray icon area
            nid.hIcon  = LoadIcon(ghInstance, MAKEINTRESOURCE(IDR_ICO_MAIN)); //Load Icon for tray
            lstrcpy(nid.szTip,"myWinUhr2"); //Tray Icon Tool Tip
            Shell_NotifyIcon(NIM_ADD,&nid);//Show the Icon
        }

        // SendDlgItemMessage(hwndDlg,IDI_ACLOCK,STM_SETICON,(WPARAM)nid.hIcon,(LPARAM)0);
    
        return TRUE;

    case WM_SHELLNOTIFY:
        //Called when pointer entry tray icon area
        if (wParam==IDR_ICO_TRAY3 )
        {
            //Show PopUp menu if right button down
            if (lParam == WM_RBUTTONDOWN)
            {
                GetCursorPos(&pt );
                SetForegroundWindow(hwndDlg);
                TrackPopupMenu(hPopupMenu,TPM_RIGHTALIGN,pt.x,pt.y,0,hwndDlg,0 );
                PostMessage(hwndDlg,WM_NULL,0,0 );
                return TRUE;
            }
            else if (WM_LBUTTONDBLCLK == lParam)
            {
                // Restore Window
                SendMessage(hwndDlg,WM_COMMAND,IDM_RESTORE,0);
            }
        }
        break;

    case WM_COMMAND:
        switch (GET_WM_COMMAND_ID(wParam, lParam))
        {
        case IDM_EXIT:
            //SendMessage(hwndDlg, WM_CLOSE, 0, 0);
            SendMessage(uhren[0].hWndDlg, WM_CLOSE, 0, 0);
            SendMessage(uhren[1].hWndDlg, WM_CLOSE, 0, 0);
            SendMessage(uhren[2].hWndDlg, WM_CLOSE, 0, 0);
            exit(0);
            return TRUE;

        case IDM_RESTORE:
            minimized = 0;
            ShowWindow(hwndDlg,SW_RESTORE);
            break;
        case IDM_MINI:
            minimized = 1;
            SaveRect();
            if (hide == 1)
            {
                ShowWindow(hwndDlg, SW_HIDE);
            }
            else
            {
                SendMessage(hwndDlg, WM_SYSCOMMAND, SC_MINIMIZE, 0);
            }
            return TRUE;

        case IDM_RESTZEIT:
            // hSysMenu = GetSystemMenu(hwndDlg, FALSE);
            show_rest = !show_rest;
            if (show_rest)
            {
                CheckMenuItem(hSysMenu  , IDM_RESTZEIT, MF_CHECKED);
                CheckMenuItem(hPopupMenu, IDM_RESTZEIT, MF_CHECKED);
            }
            else
            {
                CheckMenuItem(hSysMenu  , IDM_RESTZEIT, MF_UNCHECKED);
                CheckMenuItem(hPopupMenu, IDM_RESTZEIT, MF_UNCHECKED);
            }
            return TRUE;

        case IDM_EDIT:
            if (minimized == 0)
            {
                DialogBox(ghInstance, MAKEINTRESOURCE(DLG_EDIT), hwndDlg, (DLGPROC)DlgProcEdit);
            }
            else
            {
                DialogBox(ghInstance, MAKEINTRESOURCE(DLG_EDIT), NULL, (DLGPROC)DlgProcEdit);
            }
            SaveRect();
            return TRUE;
        case IDM_HIDEX:
            {
                uhren[0].hide = !uhren[0].hide;
                CheckMenuItem(hPopupMenu,IDM_HIDEX,/*uhren[0].hide?MF_CHECKED:*/MF_UNCHECKED);
                ShowWindow(uhren[0].hWndDlg,/*uhren[0].hide?SW_HIDE:*/SW_SHOW);
                ShowWindow(uhren[0].hWndDlg,/*uhren[0].hide?SW_HIDE:*/SW_RESTORE);
                // Restore Window
                SendMessage(uhren[0].hWndDlg,WM_COMMAND,IDM_RESTORE,0);
            }
            return TRUE;
            break;

        case IDM_HIDEY:
            {
                uhren[1].hide = !uhren[1].hide;
                CheckMenuItem(hPopupMenu,IDM_HIDEY,uhren[1].hide?MF_CHECKED:MF_UNCHECKED);
                ShowWindow(uhren[1].hWndDlg,/*uhren[1].hide?SW_HIDE:*/SW_RESTORE);
                // Restore Window
                SendMessage(uhren[1].hWndDlg,WM_COMMAND,IDM_RESTORE,0);
            }
            return TRUE;
            break;

        case IDM_HIDEZ:
            {
                uhren[2].hide = !uhren[2].hide;
                CheckMenuItem(hPopupMenu,IDM_HIDEZ,uhren[2].hide?MF_CHECKED:MF_UNCHECKED);
                ShowWindow(uhren[2].hWndDlg,/*uhren[2].hide?SW_HIDE:*/SW_MAXIMIZE);
                // Restore Window
                SendMessage(uhren[2].hWndDlg,WM_COMMAND,IDM_RESTORE,0);
            }
            return TRUE;
            break;

        }
        break;

    case WM_SYSCOMMAND:
        switch (wParam)
        {
        case SC_MINIMIZE:
            minimized = 1;
            break;
        case SC_MAXIMIZE:
        case SC_RESTORE:
            minimized = 0;
            ShowWindow(hwndDlg,SW_RESTORE);
            break;
        case IDM_MINI:
            SaveRect();
            minimized = 1;
            SaveRect();
            if (hide == 1)
            {
                ShowWindow(hwndDlg, SW_HIDE);
            }
            else
            {
                SendMessage(hwndDlg, WM_SYSCOMMAND, SC_MINIMIZE, 0);
            }
            return TRUE;
        case IDM_RESTZEIT:
            // hSysMenu = GetSystemMenu(hwndDlg, FALSE);
            show_rest = !show_rest;
            if (show_rest)
            {
                CheckMenuItem(hSysMenu, IDM_RESTZEIT, MF_CHECKED);
            }
            else
            {
                CheckMenuItem(hSysMenu, IDM_RESTZEIT, MF_UNCHECKED);
            }

            return TRUE;
        case IDM_EXIT:
            SendMessage(hwndDlg, WM_CLOSE, 0, 0);
            return TRUE;
        case IDM_EDIT:
            if (minimized == 0)
            {
                DialogBox(ghInstance, MAKEINTRESOURCE(DLG_EDIT), hwndDlg, (DLGPROC)DlgProcEdit);
            }
            else
            {
                DialogBox(ghInstance, MAKEINTRESOURCE(DLG_EDIT), NULL, (DLGPROC)DlgProcEdit);
            }
            return TRUE;

        case IDM_HIDE:
            for(i=0; i<3; i++)
            {
                if (hwndDlg == uhren[i].hWndDlg)
                {
                    uhren[i].hide = !uhren[i].hide;
                    CheckMenuItem(hPopupMenu,IDM_HIDEX+i,uhren[i].hide?MF_CHECKED:MF_UNCHECKED);
                    ShowWindow(uhren[i].hWndDlg,!uhren[i].hide);
                }
            }
            break;
        case IDM_HIDEX:
            {
                uhren[0].hide = !uhren[0].hide;
                CheckMenuItem(hPopupMenu,IDM_HIDEX,uhren[0].hide?MF_CHECKED:MF_UNCHECKED);
                ShowWindow(uhren[0].hWndDlg,/*uhren[0].hide?SW_SHOW:*/SW_SHOW);
            }
            return TRUE;
            break;

        case IDM_HIDEY:
            {
                uhren[1].hide = !uhren[1].hide;
                CheckMenuItem(hPopupMenu,IDM_HIDEY,uhren[1].hide?MF_CHECKED:MF_UNCHECKED);
                ShowWindow(uhren[1].hWndDlg,/*uhren[1].hide?SW_SHOW:*/SW_SHOW);
            }
            return TRUE;
            break;

        case IDM_HIDEZ:
            {
                uhren[2].hide = !uhren[2].hide;
                CheckMenuItem(hPopupMenu,IDM_HIDEZ,uhren[2].hide?MF_CHECKED:MF_UNCHECKED);
                ShowWindow(uhren[2].hWndDlg,/*uhren[2].hide?SW_HIDE:*/SW_SHOW);
            }
            return TRUE;
            break;

        }
        break;

    case WM_TIMER:
        AktOutput(hwndDlg);
        // Tray-Tip aktualisieren
        GetLocalTime(&Jetzt);
        sprintf(nid.szTip, "%2s: %02d.%02d.%04d\nJetzt: %02d:%02d:%02d\nRest: %02d:%02d:%02d\nEnde: %02d:%02d:%02d",
            wota[Jetzt.wDayOfWeek],Jetzt.wDay, Jetzt.wMonth, Jetzt.wYear/*%100*/,
            Jetzt.wHour, Jetzt.wMinute, Jetzt.wSecond,
            RZ.wHour, RZ.wMinute, RZ.wSecond,
            EZ.wHour, EZ.wMinute, EZ.wSecond);

        // TrayIcon setzen
        HICON hTempIcon;
        hTempIcon = CreateTimeIcon(hwndDlg);
        if (NULL != hTempIcon)
        {
            if (NULL != hIcon)
            {
                DestroyIcon(hIcon);
            }
            hIcon = hTempIcon;
            nid.hIcon = hIcon;

            //SendDlgItemMessage(hwndDlg, IDR_ICO_MAIN , STM_SETICON, (WPARAM)hIcon, (LPARAM)0);
            //SendDlgItemMessage(hwndDlg, IDI_ACLOCK   , STM_SETICON, (WPARAM)hIcon, (LPARAM)0);
            //SetClassLong(hwndDlg, GCL_HICON, (LONG)hIcon);
            SendDlgItemMessage(uhren[0].hWndDlg, IDR_ICO_MAIN , STM_SETICON, (WPARAM)hIcon, (LPARAM)0);
            SendDlgItemMessage(uhren[0].hWndDlg, IDI_ACLOCK   , STM_SETICON, (WPARAM)hIcon, (LPARAM)0);
            SetClassLong(uhren[0].hWndDlg, GCL_HICON, (LONG)hIcon);
            SendDlgItemMessage(uhren[1].hWndDlg, IDR_ICO_MAIN , STM_SETICON, (WPARAM)hIcon, (LPARAM)0);
            SendDlgItemMessage(uhren[1].hWndDlg, IDI_ACLOCK   , STM_SETICON, (WPARAM)hIcon, (LPARAM)0);
            SetClassLong(uhren[1].hWndDlg, GCL_HICON, (LONG)hIcon);
            SendDlgItemMessage(uhren[2].hWndDlg, IDR_ICO_MAIN , STM_SETICON, (WPARAM)hIcon, (LPARAM)0);
            SendDlgItemMessage(uhren[2].hWndDlg, IDI_ACLOCK   , STM_SETICON, (WPARAM)hIcon, (LPARAM)0);
            SetClassLong(uhren[2].hWndDlg, GCL_HICON, (LONG)hIcon);
        }

        // Tray aktuallisieren
        Shell_NotifyIcon(NIM_MODIFY, &nid);

        if (!done)
        {
            if ((uhren[0].hWndDlg != 0) &&
                (uhren[1].hWndDlg != 0) &&
                (uhren[2].hWndDlg != 0))
            {
                RECT hr;
                for(int i=0;i<3; i++)
                {
                    GetWindowRect(uhren[i].hWndDlg, &hr);
                    hr.right  -= hr.left;
                    hr.bottom -= hr.top;
                    MoveWindow(uhren[i].hWndDlg, uhren[i].rWndDlg.left, uhren[i].rWndDlg.top, hr.right, hr.bottom, TRUE);
                }
            }
            done = !done;
        }
        return TRUE;

    case WM_CTLCOLORSTATIC:
        SetColors((HWND)lParam,(HDC) wParam);
        SendMessage(hwndDlg,WM_CTLCOLORDLG,wParam,lParam);
        return (int)GetSysColorBrush(COLOR_3DFACE);
        break;

    case WM_CTLCOLORDLG:
        return SetDlgMsgResult((HWND)lParam,uMsg, SetBkfColor(gForegroundColor,gBackgroundColor,(HDC) wParam));
        break;

    case WM_LBUTTONDBLCLK:
        DialogBox(ghInstance, MAKEINTRESOURCE(DLG_EDIT), hwndDlg, (DLGPROC)DlgProcEdit);
        break;

    case WM_RBUTTONUP:
        //SendMessage(hwndDlg, WM_SYSCOMMAND, IDM_MINI , 0);
        for(i=0; i<3; i++)
        {
            if (hwndDlg == uhren[i].hWndDlg)
            {
                uhren[i].hide = !uhren[i].hide;
                CheckMenuItem(hPopupMenu,IDM_HIDEX+i,uhren[i].hide?MF_CHECKED:MF_UNCHECKED);
                ShowWindow(uhren[i].hWndDlg,!uhren[i].hide);
            }
        }
        break;

    case WM_WINDOWPOSCHANGED:
         for(int i=0; i<3; i++)
         {
            if (hwndDlg == uhren[i].hWndDlg)
            {
                GetWindowRect(hwndDlg, &(uhren[i].rWndDlg));
                break;
             }
         }
         break;

    case WM_CLOSE:
        KillTimer(hwndDlg, TIMER_UHR);
        SaveRect();
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = hwndDlg;
        nid.uID=IDR_ICO_TRAY3;
        nid.uFlags=NIF_ICON+NIF_MESSAGE+NIF_TIP;
        Shell_NotifyIcon(NIM_DELETE, &nid);
        for(int i=0;i<3;i++){if (uhren[i].hWndDlg==hwndDlg){uhren[i].hWndDlg=NULL;break;}};
        EndDialog(hwndDlg, 0);
        return TRUE;
    }

    return FALSE;
}

static LRESULT CALLBACK DlgProcEdit(HWND hwndEDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    char hStr[100];

    switch (uMsg)
    {
    case WM_INITDIALOG:
        sprintf(hStr, "%02d:%02d:%02d", EZ.wHour, EZ.wMinute, EZ.wSecond);
        SetDlgItemText(hwndEDlg,IDD_EDIT_ZEIT, hStr);
        SetDlgItemText(hwndEDlg,IDD_EDIT_GRUND, alarmgrund);
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
            sscanf(hStr, "%hu:%hu:%hu", &EZ.wHour, &EZ.wMinute, &EZ.wSecond);

            // eine Einfache Syntaxprüfung der Eingabe  ** aN 10.04.2007
            EZ.wHour   %= 24;
            EZ.wMinute %= 60;
            EZ.wSecond %= 60;

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

void AddTime(int diff)
{
    EZ.wMinute += (short)diff;
    if (EZ.wMinute >= 60)
    {
        EZ.wMinute -= 60; 
        EZ.wHour++;
        if (EZ.wHour >= 24)
        {
            EZ.wHour %= 24;
        }
    }
}

static LRESULT CALLBACK DlgProcAlarm(HWND hwndADlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    char hStr[120];

    switch (uMsg)
    {
    case WM_INITDIALOG:
        AlarmDlg = 1;
        sprintf(hStr,"%02d:%02d:%02d   %s", EZ.wHour, EZ.wMinute, EZ.wSecond, alarmgrund);
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
        case IDD_EDIT:
            DialogBox(ghInstance, MAKEINTRESOURCE(DLG_EDIT), NULL, (DLGPROC)DlgProcEdit);
            break;
        }
            
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDBLCLK:
    case WM_LBUTTONUP:
    case WM_RBUTTONUP:
    case WM_CLOSE:
        AlarmDlg = 0;
        EndDialog(hwndADlg, 0);
        return TRUE;
    }

    return FALSE;
}


