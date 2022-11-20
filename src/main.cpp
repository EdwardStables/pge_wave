#include "olcPixelGameEngine.h"
#include "vcd_parser.h"
#include "pge_wave.h"

class WaveGUI : public olc::PixelGameEngine
{
public:
    WaveWindow wave_window;
    WaveGUI() : wave_window(*this)
    {
        sAppName = "WaveGUI";
    }

public:

    bool OnUserCreate() override
    {
        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override
    {
        wave_window.draw();
        return !GetKey(olc::Q).bPressed;
    }
};


int main()
{
    srand(time(NULL));
    WaveGUI game;
    if(game.Construct(1501, 1001, 1, 1))
        game.Start();

    parse();

    return 0;
}


