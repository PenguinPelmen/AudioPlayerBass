
#define MAX_LOADSTRING 128
#include "AudioPlayer.h"
#include "bass.h"
#include "framework.h"
#include <iostream>
#include <warning.h>
#include <windows.h>
#include "AudioCL.h"
#include "ButtonsCL.h"
#include "ScrollBar.h"
#include <shlobj_core.h>
#include <vector>
#include "ListBox.h"
#include "GraphAmplitude..h"

#pragma comment(lib, "Winmm.lib")


// Глобальные переменные:
char filename[MAX_LOADSTRING];
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна
HINSTANCE hInst;                                // текущий экземпляр
Button *audio_btn;
Audio *audio_player;
RewindBtn *rewind_btn_right;
RewindBtn *rewind_btn_left;
ScrollBar *volumScroll;
ListBox *list_box;
OPENFILENAMEA ofn{};
BROWSEINFO browse_folder_info{};
std::vector<std::string> ListMp3 = std::vector<std::string>(0);
uint16_t index_list;
RECT size_rec;
GraphAplitude *graph_aplitude;

ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
void SetOpenFileParams(HWND);
void SetOpenFolderParams(HWND);
void Init(HWND);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    // Инициализация глобальных строк
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_AUDIOPLAYER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_AUDIOPLAYER));
    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            if (audio_player != NULL)
            {
                if (audio_player->IfChannelEnd() && !ListMp3.empty() && index_list < ListMp3.size())
                {
                    OutputDebugStringA("\nNEW FILE\n");
                    list_box->SelectString(index_list);
                    audio_player->NewFileName(ListMp3[index_list]);
                    audio_player->Volum(100 - volumScroll->CurrentPos());
                    graph_aplitude->SetNewGraph(audio_player);
                    index_list++;
                }
            }
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

//
//  ФУНКЦИЯ: MyRegisterClass()
//
//  ЦЕЛЬ: Регистрирует класс окна.
//

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_AUDIOPLAYER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_AUDIOPLAYER);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));


    return RegisterClassExW(&wcex);
}

//
//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//
//   ЦЕЛЬ: Сохраняет маркер экземпляра и создает главное окно
//
//   КОММЕНТАРИИ:
//
//        В этой функции маркер экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится главное окно программы.
//

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
   if (!hWnd)
   {
      return FALSE;
   } 
   Init(hWnd);
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);
   return TRUE;
}

//
//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ЦЕЛЬ: Обрабатывает сообщения в главном окне.
//
//  WM_COMMAND  - обработать меню приложения
//  WM_PAINT    - Отрисовка главного окна
//  WM_DESTROY  - отправить сообщение о выходе и вернуться
//

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_PAINT:
    {
        if (graph_aplitude && audio_player)
            graph_aplitude->Paint(audio_player);
        break;
    }
    case WM_TIMER:
    {
        if (graph_aplitude)
            graph_aplitude->UpdateTime();
    }
    break;
    case WM_COMMAND:
    {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
                case IDM_EXIT:
                    DestroyWindow(hWnd);
                    break;
                case IDC_SELECT_FILE:
                    OutputDebugStringA("SELECT AUDIO\n");
                    if(HIWORD(wParam) == LBN_SELCHANGE)
                    {
                        index_list = list_box->GetCurrentPos();
                        audio_player->NewFileName(ListMp3[index_list]);
                        audio_player->Volum(100 - volumScroll->CurrentPos());
                        graph_aplitude->SetNewGraph(audio_player);
                        index_list++;
                    }
                    break;
                case IDM_OPEN_FILE:
                    ListMp3.clear();
                    list_box->ClearList();
                    if (GetOpenFileNameA(&ofn))
                    {
                        audio_player->NewFileName(filename);
                        audio_player->Volum(100 - volumScroll->CurrentPos());
                        graph_aplitude->SetNewGraph(audio_player);
                    }
                    break;
                case IDM_OPEN_FOLDER:
                    LPITEMIDLIST pidl;
                    if ((pidl = SHBrowseForFolder(&browse_folder_info)) != NULL)
                    {
                        ListMp3.clear();
                        list_box->ClearList();
                        audio_player->ChannelFree();
                        char file_name[MAX_LOADSTRING * 8] = {0};
                        char folder_name[MAX_LOADSTRING * 7] = {0};
                        WIN32_FIND_DATAA wfd;
                        SHGetPathFromIDListA(pidl, folder_name);
                        strncpy_s(file_name, folder_name, MAX_LOADSTRING * 7);
                        strncat_s(file_name, "\\*.mp3", MAX_LOADSTRING-1);
                        HANDLE const hFile = FindFirstFileA(file_name, &wfd);
                        if (INVALID_HANDLE_VALUE != hFile)
                        {
                            do
                            {
                                if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                                {
                                    ListMp3.push_back((std::string)folder_name + "\\" + wfd.cFileName);
                                    list_box->AddWord((std::string)wfd.cFileName);
                                }
                            } while (FindNextFileA(hFile, &wfd));
                            index_list = 0;
                        }
                    }
                    break;
                case OnPlaySound:
                    audio_btn->Click(audio_player);
                    break;
                case OnRewindR:
                    rewind_btn_right->Click(audio_player);
                    break;
                case OnRewindL:
                    rewind_btn_left->Click(audio_player);
                    break;
                default:
                    return DefWindowProc(hWnd, message, wParam, lParam);
            }
    }
    break;
    case WM_VSCROLL:
    {
        volumScroll->Scrolling(wParam);
        audio_player->Volum(100 - volumScroll->CurrentPos());
    }
        break;
    case WM_DESTROY:
    {
        graph_aplitude->Destroy();
        KillTimer(hWnd, 1);
        PostQuitMessage(0);
    }
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void SetOpenFileParams(HWND hwnd)
{
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFile = filename;
    ofn.nMaxFile = sizeof(filename);
    ofn.lpstrFilter = "mp3\0*.mp3\0wav\0*.wav\0";
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
}

void SetOpenFolderParams(HWND hwnd)
{
    ZeroMemory(&browse_folder_info, sizeof(browse_folder_info));
    browse_folder_info.ulFlags = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT;
    browse_folder_info.hwndOwner = hwnd;
    browse_folder_info.lpszTitle = L"Choose playlist";
}

void Init(HWND hWnd)
{
    GetClientRect(hWnd, &size_rec);
    audio_btn = new ResponsibleSoundBtn(L"Play", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_CENTER, size_rec.right / 2 - 250, size_rec.bottom * 3 / 4, 100, 100, NONE, hWnd);
    rewind_btn_right = new RewindBtn(L"->", 10, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_CENTER, size_rec.right / 2 - 150, size_rec.bottom * 3 / 4, 100, 100, NONE, hWnd);
    rewind_btn_left = new RewindBtn(L"<-", -10, WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON | BS_CENTER, size_rec.right / 2 - 350, size_rec.bottom * 3 / 4, 100, 100, NONE, hWnd);
    audio_player = new Audio(hWnd);
    volumScroll = new ScrollBar(hWnd, size_rec.right / 2 - 400, size_rec.bottom * 4 / 6, 30, 150, 0, 100, 75);
    list_box = new ListBox(size_rec.right * 3 / 5, size_rec.bottom * 1 / 100, size_rec.right * 2 / 5, size_rec.bottom * 99 / 100, WS_CHILD | WS_VISIBLE | LBS_STANDARD | LBS_WANTKEYBOARDINPUT, hWnd);
    graph_aplitude = new GraphAplitude(hWnd, 0, 0, size_rec.right * 3 / 5, size_rec.bottom * 4 / 6);
    graph_aplitude->UpdateBackBuffer();
    graph_aplitude->RenderToBackBuffer();
    SetTimer(hWnd, 1, 30, NULL);
    SetOpenFileParams(hWnd);
    SetOpenFolderParams(hWnd);
}