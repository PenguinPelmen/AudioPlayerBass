#pragma once
#include <windows.h>
#include <iostream>
#include "bass.h"
#include <vector>
#include "AudioCl.h"

class GraphAplitude
{
private:
	std::vector<float> amplitudes;
	float duration;
	HBITMAP hWave_form_bitmap;
	HCHANNEL decode_hwnd;
    HSTREAM stream_play;
    HWND window_hwnd;
    RECT rect_graph;
    float current_seconds;
    HDC hBackBuffer;
    HBITMAP hBackBitmap;
public:
    GraphAplitude (HWND _win_hwnd, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
    {
        window_hwnd = _win_hwnd;
        SetRect(&rect_graph, x, y, width, height);
        duration = 0.0f;
        hWave_form_bitmap = NULL;
        decode_hwnd = NULL;
        stream_play = NULL;
        current_seconds = 0.0f;
        hBackBuffer = NULL;
        hBackBitmap = NULL;
    }
    void ExtractAmplitudes()
    {
        const DWORD windowSize = 512;
        std::vector<short> buffer(windowSize * 2); // стерео

        while (true) {
            int bytesRead = BASS_ChannelGetData(decode_hwnd, buffer.data(), windowSize * 2 * sizeof(short));
            if (bytesRead <= 0) break;

            float sum = 0.0f;
            int samples = bytesRead / sizeof(short);
            for (int i = 0; i < samples; i += 2) {
                float mono = (buffer[i] + buffer[i + 1]) / 2.0f;
                sum += mono * mono;
            }

            float rms = sqrtf(sum / (samples / 2));
            amplitudes.push_back(rms / 32768.0f); // нормализация
        }
    }

    void SetNewGraph(Audio *audio_player)
    {
        if (hWave_form_bitmap) {
            DeleteObject(hWave_form_bitmap);
            hWave_form_bitmap = NULL;
        }
        amplitudes.clear();
        decode_hwnd = audio_player->GetDecodeChannel();
        stream_play = audio_player->GetStream();
        duration = audio_player->Time();
        ExtractAmplitudes();
        RECT rect = rect_graph;
        HDC hdc = GetDC(window_hwnd);
        DrawWaveFormBitmap(hdc, rect.right, rect.bottom);
        ReleaseDC(window_hwnd, hdc);

        RenderToBackBuffer();

        InvalidateRect(window_hwnd, NULL, TRUE);
    }

    int UpdateTime()
    {
        if (BASS_ChannelIsActive(stream_play) != BASS_ACTIVE_PLAYING)
            return 0;

        QWORD pos = BASS_ChannelGetPosition(stream_play, BASS_POS_BYTE);
        current_seconds = (float)BASS_ChannelBytes2Seconds(stream_play, pos);

        RenderToBackBuffer();

        InvalidateRect(window_hwnd, NULL, FALSE);
        return 1;
    }

    void Destroy()
    {
        if (hWave_form_bitmap) DeleteObject(hWave_form_bitmap);
    }

    void Paint(Audio *audio_player)
    {
       /* PAINTSTRUCT ps;
        HDC hdc = BeginPaint(window_hwnd, &ps);
        RECT rect;
        rect = rect_graph;
        DrawWaveFormBitmap(hdc, rect.right, rect.bottom);
        HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP oldBmp = (HBITMAP)SelectObject(hdcMem, hWave_form_bitmap);
        BitBlt(hdc, 0, 0, rect.right, rect.bottom, hdcMem, 0, 0, SRCCOPY);
        SelectObject(hdcMem, oldBmp);
        DeleteDC(hdcMem);

        DrawCursor(hdc, audio_player);

        EndPaint(window_hwnd, &ps);*/
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(window_hwnd, &ps);
        if (hBackBuffer && hBackBitmap) {
            RECT rect;
            rect = rect_graph;
            BitBlt(hdc, 0, 0, rect.right, rect.bottom, hBackBuffer, 0, 0, SRCCOPY);
        }
        EndPaint(window_hwnd, &ps);
    }
    void DrawWaveFormBitmap(HDC hdcRef, int width, int height)
    {
        if (hWave_form_bitmap) DeleteObject(hWave_form_bitmap);

        HDC hdcMem = CreateCompatibleDC(hdcRef);
        hWave_form_bitmap = CreateCompatibleBitmap(hdcRef, width, height);
        HBITMAP oldBmp = (HBITMAP)SelectObject(hdcMem, hWave_form_bitmap);

        RECT rect{ 0, 0, width, height };
        FillRect(hdcMem, &rect, (HBRUSH)(COLOR_WINDOW + 1));

        if (!amplitudes.empty())
        {
            int count = amplitudes.size();
            float xScale = (float)width / count;

            MoveToEx(hdcMem, 0, height / 2, nullptr);
            for (int i = 0; i < count; ++i) {
                float amp = amplitudes[i];
                int x = (int)(i * xScale);
                int y = (int)(height / 2 - amp * (height / 2));
                LineTo(hdcMem, x, y);
            }
        }

        SelectObject(hdcMem, oldBmp);
        DeleteDC(hdcMem);
    }
    void DrawCursor(HDC hdc, Audio *audio_player)
    {
        float seconds = audio_player->Time();
        int x = (int)((seconds / duration) * rect_graph.right);

        HPEN pen = CreatePen(PS_SOLID, 2, RGB(255, 0, 0));
        HPEN old = (HPEN)SelectObject(hdc, pen);
        MoveToEx(hdc, x, 0, nullptr);
        LineTo(hdc, x, rect_graph.bottom);
        SelectObject(hdc, old);
        DeleteObject(pen);
    }
    void UpdateBackBuffer() {
        RECT rect = rect_graph;
        int width = rect.right;
        int height = rect.bottom;

        HDC hdc = GetDC(window_hwnd);
        if (hBackBuffer) DeleteDC(hBackBuffer);
        if (hBackBitmap) DeleteObject(hBackBitmap);

        hBackBuffer = CreateCompatibleDC(hdc);
        hBackBitmap = CreateCompatibleBitmap(hdc, width, height);
        SelectObject(hBackBuffer, hBackBitmap);

        ReleaseDC(window_hwnd, hdc);
    }

    void RenderToBackBuffer() {
        RECT rect;
        rect = rect_graph;
        int width = rect.right;
        int height = rect.bottom;

        FillRect(hBackBuffer, &rect, (HBRUSH)(COLOR_WINDOW + 1));

        if (hWave_form_bitmap) {
            HDC hdcWave = CreateCompatibleDC(hBackBuffer);
            HBITMAP oldBmp = (HBITMAP)SelectObject(hdcWave, hWave_form_bitmap);

            int visibleWidth = (int)((current_seconds / duration) * width);
            BitBlt(hBackBuffer, 0, 0, visibleWidth, height, hdcWave, 0, 0, SRCCOPY);

            SelectObject(hdcWave, oldBmp);
            DeleteDC(hdcWave);
        }

        // Рисуем красную линию
        int cursorX = (int)((current_seconds / duration) * width);
        HPEN redPen = CreatePen(PS_SOLID, 1, RGB(255, 0, 0));
        HPEN oldPen = (HPEN)SelectObject(hBackBuffer, redPen);
        MoveToEx(hBackBuffer, cursorX, 0, NULL);
        LineTo(hBackBuffer, cursorX, height);
        SelectObject(hBackBuffer, oldPen);
        DeleteObject(redPen);
    }
};
