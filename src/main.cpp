#include "olcPixelGameEngine.h"
#include "vcd_parser.h"
#include "wave_store.h"
#include "pge_wave.h"

class WaveGUI : public olc::PixelGameEngine
{
public:
    WaveWindow wave_window;
    WaveGUI(VarStore &store) : wave_window(*this, store)
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
    //VarStore store;
    //srand(time(NULL));
    //WaveGUI game(store);
    //if(game.Construct(1501, 1001, 1, 1))
    //    game.Start();

    parse();

    return 0;
}
