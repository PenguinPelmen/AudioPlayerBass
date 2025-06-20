#pragma once
#include <windows.h>

class ScrollBar
{
private:
	int max, min;
	int current_value;
	HWND hScroll;
	int d_line, d_page;
public:
	ScrollBar(HWND win, int x, int y, int width, int hight, int Min = 0, int Max = 100, int Current_value = 25, int dLine = 1, int dPage = 5, int flag = SBS_VERT)
	{
		max = Max;
		min = Min;
		d_line = dLine;
		d_page = dPage;
		current_value = Current_value;
		hScroll = CreateWindow(L"SCROLLBAR", NULL, WS_VISIBLE | WS_CHILD | flag,
			x, y, width, hight, win, (HMENU)200, NULL, NULL);
		SetScrollRange(hScroll, SB_CTL, min, max, TRUE);
		SetScrollPos(hScroll, SB_CTL, current_value, TRUE);
	}
	~ScrollBar()
	{
		CloseHandle(hScroll);
	}
	void Scrolling(WPARAM wParam)
	{
		switch (LOWORD(wParam))
		{
		case SB_LINEUP: current_value -= d_line; break;
		case SB_LINEDOWN: current_value += d_line; break;
		case SB_PAGEUP: current_value -= d_page; break;
		case SB_PAGEDOWN: current_value += d_page; break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			current_value = HIWORD(wParam);
			break;
		}
		current_value = max(min, min(current_value, max));
		SetScrollPos(hScroll, SB_CTL, current_value, TRUE);
	}

	int CurrentPos()
	{
		return current_value;
	}
};