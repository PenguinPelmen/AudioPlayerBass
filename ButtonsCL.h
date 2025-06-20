#pragma once

#define NONE 1
#define OnPlaySound 2
#define OnRewindR 3
#define OnRewindL 4

class Button
{
private:
    WCHAR btn_text[MAX_LOADSTRING];
    int style;
    UINT x, y;
    UINT width, height;
    HWND parent_window;

protected:
    HWND btn_hwnd;

public:

    Button(const wchar_t *BtnText, int Style, UINT X, UINT Y, UINT Width, UINT Height, uint16_t hMenu ,HWND ParentWindow)
    {
        wcscpy_s(btn_text, BtnText);
        style = Style;
        x = X;
        y = Y;
        width = Width;
        height = Height;
        parent_window = ParentWindow;
        btn_hwnd = CreateWindow(
            L"BUTTON",  // Predefined class; Unicode assumed 
            btn_text,      // Button text 
            style,     // Styles 
            x,         // x position 
            y,         // y position 
            width,        // Button width
            height,        // Button height
            parent_window,     // Parent window
            (HMENU)hMenu,       // No menu.
            (HINSTANCE)GetWindowLongPtr(parent_window, GWLP_HINSTANCE),
            NULL);      // Pointer not needed.
    }
    ~Button()
    {
        DestroyWindow(btn_hwnd);
    }
    virtual void Click() {};
    virtual void Click(Audio *ptr){};

    HWND GetHwnd() { return btn_hwnd; }
};


class ResponsibleSoundBtn : public Button
{
private:
    bool isPlay;
public:
    ResponsibleSoundBtn(const wchar_t *BtnText, int Style, UINT X, UINT Y, UINT Width, UINT Height, uint16_t hMenu, HWND ParentWindow)
        :Button(BtnText, Style, X, Y, Width, Height, hMenu = OnPlaySound, ParentWindow) {
        isPlay = true;
    }
    void Click(Audio *audio_player)
    {
        if (!isPlay)
        {
            audio_player->Play();
            isPlay = true;
            return;
        }
        audio_player->Pause();
        isPlay = false;
    }
};

class RewindBtn : public Button
{
private:
    int shift_sec;
public:
    RewindBtn(const wchar_t *BtnText, int ShiftSec, int Style, UINT X, UINT Y, UINT Width, UINT Height, uint16_t hMenu, HWND ParentWindow)
        :Button(BtnText, Style, X, Y, Width, Height, hMenu = ShiftSec > 0 ? OnRewindR : OnRewindL, ParentWindow) {
        shift_sec = ShiftSec;
    }

    void Click(Audio *audio_player)
    {
        audio_player->Rewind(shift_sec);
    }
};