#include "state.h"

State::State(olc::PixelGameEngine &pge, WaveStore &ws) : 
    pge(pge), 
    ws(ws)
{}

bool State::update(){
    got_input = false;

    common_inputs();
    name_inputs();
    wave_inputs();
    picker_inputs();

    return !got_input;
}

void State::cursor_update(e_cursor_dir dir){
    int wave_count = ws.get_visible_wave_count();

    //may be a nullptr if the index is invalid
    Var* selected_instance = ws.get_var_by_index(cursor_visble_wave_index);
    int next_cursor_time;
    
    for (int i = 0; i < perform_count; i++)
        switch (dir)
        {
        case LEFT:
            if (selected_instance == nullptr) break;
            cursor_time = selected_instance->get_prev_time(cursor_time);
            break;
        case RIGHT:
            if (selected_instance == nullptr) break;
            next_cursor_time = selected_instance->get_next_time(cursor_time);
            if (next_cursor_time != -1) cursor_time = next_cursor_time;
            break;
        case UP:
            if (cursor_visble_wave_index <= 0) 
                //go to the last element if already 0 or -1. If no waves, sets to -1 (should stay in the same state)
                cursor_visble_wave_index = wave_count - 1; 
            else 
                //just subtract 1
                cursor_visble_wave_index--;
            break;
        default: //DOWN
            if (wave_count == 0)
                //if no waves, become -1
                cursor_visble_wave_index = -1;
            if (cursor_visble_wave_index == -1 || cursor_visble_wave_index == wave_count-1) 
                //go to the first element when waves on screen and either -1 or at last element
                cursor_visble_wave_index = 0;
            else 
                //just add 1
                cursor_visble_wave_index++;
            break;
        }
}

void State::common_inputs(){
    if (TOGGLE_PICKER) picker_show = !picker_show;

    //input macro calls handle_number_input which also performs the required action
    if (NUM_0);
    if (NUM_1);
    if (NUM_2);
    if (NUM_3);
    if (NUM_4);
    if (NUM_5);
    if (NUM_6);
    if (NUM_7);
    if (NUM_8);
    if (NUM_9);
}

void State::name_inputs(){
    if (picker_show) return;

    if (TOGGLE_VALUE) show_values = !show_values;
}

void State::wave_inputs(){
    if (picker_show) return;        

    if (MOVE_LEFT) cursor_update(LEFT);
    if (MOVE_RIGHT) cursor_update(RIGHT);
    if (MOVE_UP) cursor_update(UP);
    if (MOVE_DOWN) cursor_update(DOWN);
    if (EXPAND_WAVE_WINDOW) name_width = std::max(0.0f, name_width - 0.05f);
    if (REDUCE_WAVE_WINDOW) name_width = std::min(1.0f, name_width + 0.05f);
    if (ZOOM_OUT) time_per_px = std::min(2048.0f, time_per_px * 2); //zoom out
    if (ZOOM_IN) time_per_px = std::max(0.125f, time_per_px / 2); //zoom in
}

void State::picker_inputs(){
    if (!picker_show) return;

    if (MOVE_UP) picker_index = std::max(0, picker_index-1);
    if (MOVE_DOWN) picker_index = std::min(ws.get_raw_wave_count()-1, picker_index+1);
    if (CONFIRM) ws.create_instance(picker_index);
}

//Special case of input handling, does both key detection and adding in the value to the total
bool State::handle_number_input(olc::Key key, int value){
    if(pge.GetKey(key).bPressed){
        if (!typing_number && value == 0) return true;

        if (!typing_number){
            active_count = value;
            typing_number = true;
        } else {
            active_count *= 10;
            active_count += value;
        }
    }
    return true;
}

bool State::input(olc::Key key, bool held){
    bool res = false;
    perform_count = 0;
    if (held)
        res = pge.GetKey(key).bHeld;
    else
        res = pge.GetKey(key).bPressed;
    this->got_input |= res;
    
    if (res){
        perform_count = active_count;
        typing_number = false;
        active_count = 1;
        return true;
    } else {
        return false;
    }
};
