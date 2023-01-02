#ifndef __STATE_H__
#define __STATE_H__

#include "olcPixelGameEngine.h"
#include "wave_store.h"
#include "config.h"

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
    std::vector<int> selected_indexes = {};
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
    void select_update(e_cursor_dir dir);
    void delete_selected();
    void deselect();
    bool input(olc::Key key, bool held=false);
    bool handle_number_input(olc::Key key, int value);
};

#endif