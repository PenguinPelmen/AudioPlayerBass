#pragma once
#include <iostream>
#include <windows.h>
class ListBox
{
private:
	uint16_t current_pos;
    HWND list_handle;
public:
	ListBox(uint16_t X, uint16_t Y, uint16_t Width, uint16_t Height, int Style, HWND ParentWindow)
	{
        list_handle = CreateWindowA(
            "LISTBOX",  // Predefined class; Unicode assumed 
            "",      // Button text 
            Style,     // Styles 
            X,         // x position 
            Y,         // y position 
            Width,        // Button width
            Height,        // Button height
            ParentWindow,     // Parent window
            (HMENU)IDC_SELECT_FILE,       // No menu.
            (HINSTANCE)GetWindowLongPtr(ParentWindow, GWLP_HINSTANCE),
            NULL);
        if (list_handle == NULL) {
            MessageBox(ParentWindow, L"Ошибка инициализации BASS", L"ERROR", MB_ICONWARNING | MB_OK);
            DestroyWindow(ParentWindow);
        }
        current_pos = 0;
	}
    ~ListBox()
    {
        CloseHandle(list_handle);
    }
    void AddList(std::vector<std::string> newlist)
    {
        for (auto s : newlist)
        {
            SendMessageA(list_handle, LB_ADDSTRING, 0, (LPARAM)STRTOCHAR(s));
        }
    }

    void AddWord(std::string str)
    {
        SendMessageA(list_handle, LB_ADDSTRING, 0, (LPARAM)STRTOCHAR(str));
    }
    void EraceWord(uint16_t i)
    {
        SendMessage(list_handle, LB_DELETESTRING, 0, (LPARAM)i);
    }
    void ClearList()
    {
        SendMessage(list_handle, LB_RESETCONTENT, 0, 0);
    }

    uint16_t GetCurrentPos()
    {
        return (uint16_t)SendMessage(list_handle, LB_GETCARETINDEX, 0, 0);
    }
    HWND GetHwnd() 
    {
        return list_handle;
    }
    void SelectString(uint16_t i)
    {
        SendMessage(list_handle, LB_SETCURSEL, (WPARAM)i, 0);
    }
};