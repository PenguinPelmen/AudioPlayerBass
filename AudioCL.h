#pragma once
#include <cstring>
#include <iostream>

class Audio
{
private:
    HSTREAM stream_audio;
    HWND hwnd_win;
    std::string file_name;
    QWORD len_audio;
    float time;
    float bitrate;
    HCHANNEL hchannel;
public:
    Audio(std::string filename, HWND hwnd)
    {
        stream_audio = NULL;
        hchannel = NULL;
        file_name = filename;
        if (!BASS_Init(-1, 44100, 0, 0, NULL)) {
            MessageBox(hwnd, L"Ошибка инициализации BASS", L"ERROR", MB_ICONWARNING | MB_OK);
            BASS_Free();
            DestroyWindow(hwnd);
        }
        NewAudio();
        hwnd_win = hwnd;
    }
    Audio(HWND hwnd)
    {
        stream_audio = NULL;
        hchannel = NULL;
        if (!BASS_Init(-1, 44100, 0, 0, NULL)) {
            MessageBox(hwnd, L"Ошибка инициализации BASS", L"ERROR", MB_ICONWARNING | MB_OK);
            BASS_Free();
            DestroyWindow(hwnd);
        }
        len_audio = 0;
        time = 1;
        bitrate = 0;
        hwnd_win = hwnd;
    }
    ~Audio()
    {
        BASS_StreamFree(stream_audio);
        BASS_Free();
    }
    void NewAudio()
    {
        BASS_ChannelStop(stream_audio);
        BASS_StreamFree(stream_audio);
        hchannel = BASS_StreamCreateFile(FALSE, STRTOCHAR(file_name), 0, 0, BASS_STREAM_DECODE);
        stream_audio = BASS_StreamCreateFile(FALSE, STRTOCHAR(file_name), 0, 0, 0);
        if (!stream_audio || !hchannel) {
            MessageBox(hwnd_win, L"Ошибка загрузки файла", L"ERROR", MB_ICONWARNING | MB_OK);
            BASS_StreamFree(stream_audio);
        }
        len_audio = BASS_ChannelGetLength(stream_audio, BASS_POS_BYTE);
        time = BASS_ChannelBytes2Seconds(stream_audio, len_audio);
        bitrate = (len_audio / (125 * time)) + 1;// Kbit/s
        BASS_ChannelPlay(stream_audio, FALSE);
    }
    void NewFileName(std::string filename)
    {
        file_name = filename;
        NewAudio();
    }
    void Play()
    {
        BASS_ChannelPlay(stream_audio, FALSE);
    }
    void Pause()
    {
        BASS_ChannelPause(stream_audio);
    }
    void Rewind(int shift_seconds)
    {
        QWORD pos = BASS_ChannelGetPosition(stream_audio, BASS_POS_BYTE);
        BASS_ChannelSetPosition(stream_audio, pos + (int)bitrate * shift_seconds * 125, BASS_POS_BYTE);
    }
    void Volum(WORD value)
    {
        BASS_ChannelSetAttribute(stream_audio, BASS_ATTRIB_VOL, (float)value/100); //принимает 0,## значение
    }
    void ChannelFree()
    {
        BASS_ChannelStop(stream_audio);
        BASS_StreamFree(stream_audio);
    }
    byte IfChannelEnd() 
    { 
        if (BASS_ChannelGetPosition(stream_audio, BASS_POS_BYTE) >= BASS_ChannelGetLength(stream_audio, BASS_POS_BYTE) && stream_audio != -1)
        {
            BASS_ChannelStop(stream_audio);
            BASS_StreamFree(stream_audio);
            return 1;
        }
        return 0;
    }
    QWORD CurrentPos()
    {
        return BASS_ChannelGetPosition(stream_audio, BASS_POS_BYTE);
    }
    HSTREAM GetStream()
    {
        return stream_audio;
    }
    HCHANNEL GetDecodeChannel()
    {
        return hchannel;
    }
    float Time()
    {
        return time;
    }
};