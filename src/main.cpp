#include "olcPixelGameEngine.h"
#include <vector>
#include <string>

class Wave {
public:
    std::string name = "testwave";
    std::vector<uint8_t> data;
    int height = 10;
    int width = 10;
    olc::vi2d pos = {0, 200};

    Wave(){
        data = {0, 1, 0, 1, 0, 1, 0, 1};
    }

    void draw(olc::PixelGameEngine &pge){
        int x = pos.x;
        for (auto &d : data){
            int y = pos.y + height - (d*height);
            pge.DrawLine({x,y}, {x+width, y}, olc::GREEN);
            x += width;
        }
    }
};

class WaveStore {
public:
    WaveStore() {

    }
};

class NamePane {
public:
    olc::vi2d window_pos;
    olc::vi2d window_size;
    float *proportion;
    WaveStore &ws;

    NamePane(olc::vi2d pos, olc::vi2d size, float *proportion, WaveStore &ws) :
        window_pos(pos),
        window_size(size),
        proportion(proportion),
        ws(ws)
    {

    }

    void draw(olc::PixelGameEngine &pge){
        pge.DrawRect(get_pos(), get_size());
    }

    olc::vi2d get_pos(){
        return window_pos;
    }

    olc::vi2d get_size(){
        return window_size - olc::vi2d(window_size.x*(1.0f-*proportion), 0);
    }
};

class WavePane {
public:
    olc::vi2d window_pos;
    olc::vi2d window_size;
    float *proportion;
    WaveStore &ws;

    WavePane(olc::vi2d pos, olc::vi2d size, float *proportion, WaveStore &ws) :
        window_pos(pos),
        window_size(size),
        proportion(proportion),
        ws(ws)
    {

    }

    void draw(olc::PixelGameEngine &pge){
        pge.DrawRect(get_pos(), get_size());
    }

    olc::vi2d get_pos(){
        return window_pos + olc::vi2d(window_size.x* *proportion, 0);
    }

    olc::vi2d get_size(){
        return window_size - olc::vi2d(window_size.x* *proportion, 0);
    }
};

class WaveWindow {
public:
    olc::vi2d pos = {0,0};
    olc::vi2d size = {1500, 1000};
    float name_width = 0.1f;

    NamePane name_pane;
    WavePane wave_pane;

    WaveStore ws;
    
    WaveWindow() : 
        name_pane(NamePane(pos, size, &name_width, ws)),
        wave_pane(WavePane(pos, size, &name_width, ws)) 
    {

    }

    void update(olc::PixelGameEngine &pge){
        if (pge.GetKey(olc::LEFT).bPressed) {
            name_width = std::max(0.0f, name_width - 0.05f);
        }
        if (pge.GetKey(olc::RIGHT).bPressed) {
            name_width = std::min(1.0f, name_width + 0.05f);
        }
    }

    void draw(olc::PixelGameEngine &pge){
        name_pane.draw(pge);
        wave_pane.draw(pge);
    }
};


class WaveGUI : public olc::PixelGameEngine
{
public:
    WaveWindow wave_window;
    WaveGUI()
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
        Clear(olc::BLACK);
        wave_window.update(*this);
        wave_window.draw(*this);
        return !GetKey(olc::Q).bPressed;
    }
};


int main()
{
    WaveGUI game;
    if(game.Construct(1920, 1080, 1, 1))
        game.Start();

    return 0;
}


