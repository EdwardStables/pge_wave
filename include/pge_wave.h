#ifndef __PGE_WAVE_H__
#define __PGE_WAVE_H__

#include <array>
#include <vector>
#include <string>
#include <sstream>
#include <time.h>
#include <iostream>
#include "olcPixelGameEngine.h"
#include "wave_store.h"

std::vector<uint32_t> random_wave(int len);

class Wave {
public:
    std::string name;
    std::vector<std::vector<uint32_t>> data;
    int *height;
    int width;

    Wave(int *height);
    void draw(olc::vi2d pos, uint32_t start_time, uint32_t end_time, float time_per_px, olc::PixelGameEngine &pge);
    void draw_template(
        olc::vi2d pos,
        olc::PixelGameEngine &pge,
        std::function<int(int)> get_next_time,
        std::function<int(int)> data_at_time,
        uint32_t start_time,
        uint32_t end_time,
        float time_per_px,
        bool bus=false
    );
};

class WaveInstance {
public:
    Wave* wave; 
    std::string name_override;
    int max;
    int min;

    WaveInstance(Wave* wave, int max=-1, int min=0, std::string name_override="");
    std::string get_name();
    void draw(olc::vi2d pos, uint32_t start_time, uint32_t end_time, float time_per_px, olc::PixelGameEngine &pge);
};

class WaveStore {
    VarStore &varstore;
    std::vector<Wave*> waves;
    std::vector<WaveInstance> wave_instances;
    int wave_height = 10;
    int v_gap = 5;
public:
    WaveStore(VarStore &store);
    void create_instance(int num);
    int get_raw_wave_count();
    Wave* get_raw_wave(int num);
    int get_visible_wave_count();
    std::string get_visible_wave_name(int num);
    WaveInstance get_visible_wave(int num);
    int get_v_offset(int num);
};

struct State {
    bool got_input;
    WaveStore &ws;

    //-- General State --//
    float name_width = 0.1f;
    int start_time = 0;
    int timeline_width = 10;
    float time_per_px = 1.0f; //1ns
    
    //-- Picker specific variables --//
    bool picker_show = false;
    int picker_border = 50; //50px
    int picker_index = 0;

    olc::PixelGameEngine &pge;

    State(olc::PixelGameEngine &pge, WaveStore &ws);
    bool update();
private:
    void common_inputs();
    void wave_inputs();
    void picker_inputs();
    bool input(olc::Key key, bool held=false);
};

class NamePane {
public:
    olc::vi2d window_pos;
    olc::vi2d window_size;
    float *proportion;
    WaveStore &ws;

    State &state;

    NamePane(olc::vi2d pos, olc::vi2d size, float *proportion, WaveStore &ws, State &state);
    void draw(olc::PixelGameEngine &pge);
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