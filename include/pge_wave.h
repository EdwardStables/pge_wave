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

std::vector<uint32_t> random_wave(int len);

class Wave {
public:
    std::string name;
    VarStore &data;
    int *height;
    int width;

    Wave(std::string name, int *height, VarStore &data);
    void draw(olc::vi2d pos, uint32_t start_time, uint32_t end_time, float time_per_px, olc::PixelGameEngine &pge);
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
    int end_time = 0;
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
    int get_end_time();
    Var* get_var_by_index(int num);
};

struct State {
    enum e_cursor_dir {LEFT, RIGHT, UP, DOWN};

    bool got_input;
    WaveStore &ws;

    //-- General State --//
    float name_width = 0.1f;
    int start_time = 0;
    int timeline_width = 10;
    float time_per_px = 1.0f; //1ns
    
    //-- Input State --//
    bool typing_number = false;
    int perform_count = 0;
    int active_count = 1;
    
    //-- Picker State --//
    bool picker_show = false;
    int picker_border = 50; //50px
    int picker_index = 0;

    //-- Wave Viewer State --//
    int cursor_time = 0;
    int cursor_visble_wave_index = -1;
    olc::Pixel cursor_colour = olc::WHITE;

    //-- Name Panel State --//
    bool show_values = false;

    olc::PixelGameEngine &pge;

    State(olc::PixelGameEngine &pge, WaveStore &ws);
    bool update();
private:
    void common_inputs();
    void wave_inputs();
    void name_inputs();
    void picker_inputs();
    void cursor_update(e_cursor_dir dir);
    bool input(olc::Key key, bool held=false);
    bool handle_number_input(olc::Key key, int value);
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