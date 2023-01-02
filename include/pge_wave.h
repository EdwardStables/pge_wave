#ifndef __PGE_WAVE_H__
#define __PGE_WAVE_H__

#include <array>
#include <vector>
#include <string>
#include <sstream>
#include <time.h>
#include <iostream>
#include <tuple>
#include "olcPixelGameEngine.h"
#include "wave_store.h"
#include "state.h"

class NamePane {
public:
    olc::vi2d window_pos;
    olc::vi2d window_size;
    float *proportion;
    WaveStore &ws;

    State &state;

    NamePane(olc::vi2d pos, olc::vi2d size, float *proportion, WaveStore &ws, State &state);
    void draw(olc::PixelGameEngine &pge);
    std::string get_value_text(int wave_index);
    std::string get_name_text(int wave_index);
    olc::vi2d get_pos();
    olc::vi2d get_size();
};

class WavePane {
public:
    olc::vi2d window_pos;
    olc::vi2d window_size;
    float *proportion;
    WaveStore &ws;


    State &state;

    WavePane(olc::vi2d pos, olc::vi2d size, float *proportion, WaveStore &ws, State &state);
    void draw_timeline(olc::PixelGameEngine &pge);
    void draw(olc::PixelGameEngine &pge);
    void draw_waves(olc::PixelGameEngine &pge);
    void draw_cursor(olc::PixelGameEngine &pge);
    olc::vi2d get_pos();
    olc::vi2d get_size();
};

class ValuePane {
private:
    olc::vi2d pos;
    olc::vi2d size;
    WaveStore &ws;
    State &state;

public:
    ValuePane(olc::vi2d pos, olc::vi2d size, WaveStore &ws, State &state);
    void draw(olc::PixelGameEngine &pge);
    olc::vi2d get_pos();
    olc::vi2d get_size();
};

class WavePicker {
    olc::vi2d pos;    
    olc::vi2d size;    

    State &state;
    WaveStore &ws;

public:
    WavePicker(olc::vi2d rootpos, olc::vi2d rootsize, State &state, WaveStore &ws);
    void draw(olc::PixelGameEngine &pge);
};

class WaveWindow {
public:
    State state; 

    olc::vi2d pos = {0,0};
    olc::vi2d size = {1500, 1000};

    NamePane name_pane;
    WavePane wave_pane;
    WavePicker wave_picker;

    WaveStore ws;
    olc::PixelGameEngine &pge;
    bool firstframe = true;
    
    WaveWindow(olc::PixelGameEngine &pge, VarStore &store);
    void draw();
};

#endif