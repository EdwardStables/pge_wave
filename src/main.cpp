#include "olcPixelGameEngine.h"
#include <array>
#include <vector>
#include <string>
#include <sstream>
#include <time.h>
#include <iostream>

struct State {
    float name_width = 0.1f;
    int start_time = 0;
    int timeline_width = 10;
    float time_per_px = 1.0f; //1ns

    bool update(olc::PixelGameEngine &pge){
        set_checkpoint();        

        auto input = [&] (olc::Key key, bool held=false){
            if (held)
                return pge.GetKey(key).bHeld;
            else
                return pge.GetKey(key).bPressed;
        };

        if (input(olc::LEFT)) start_time = std::max(0, start_time - 50);
        if (input(olc::RIGHT)) start_time = start_time + 50;
        if (input(olc::N)) name_width = std::max(0.0f, name_width - 0.05f);
        if (input(olc::M)) name_width = std::min(1.0f, name_width + 0.05f);
        if (input(olc::Z) && input(olc::SHIFT, true)) time_per_px = std::min(2048.0f, time_per_px * 2); //zoom out
        if (input(olc::Z) && !input(olc::SHIFT, true)) time_per_px = std::max(0.125f, time_per_px / 2); //zoom in
    
        return get_checkpoint();
    }

private :
    bool checkpoint_valid = false;    

    float s_name_width;
    int s_start_time;
    uint32_t s_timeline_width;
    int s_time_per_px; //1ns

    void set_checkpoint(){
        s_name_width        = name_width;
        s_start_time        = start_time;
        s_timeline_width    = timeline_width;
        s_time_per_px       = time_per_px; //1ns
        checkpoint_valid = true;
    }

    bool get_checkpoint(){
        if (!checkpoint_valid) return false;

        bool same = true;
        same &= s_name_width == name_width;
        same &= s_start_time == start_time;
        same &= s_timeline_width == timeline_width;
        same &= s_time_per_px == time_per_px;
        checkpoint_valid = false;
        return same;
    }
};

//Test function
std::vector<uint32_t> random_wave(int len){
    std::vector<uint32_t> data;

    for (int i = 0; i < len; i++){
        data.push_back(rand()%1500);
    }

    std::sort(data.begin(), data.end());

    return data;
}

class Wave {
public:
    std::string name;
    std::vector<std::vector<uint32_t>> data;
    int *height;
    int width;

    Wave(int *height) :
        height(height)
    {
        std::stringstream ss;
        ss << "wave_" << rand();
        width = rand() % 7;
        if (width == 0) width = 1;
        for (int i = 0; i < width; i++){
            data.push_back(random_wave(10));
        }

        if (width > 1) ss << " [" << width-1 << ":0]";

        name = ss.str();
    }

    
    void draw(olc::vi2d pos, uint32_t start_time, uint32_t end_time, float time_per_px, olc::PixelGameEngine &pge){
        auto get_next_time = [this](int time) {
            int min_time = -1;
            for (auto d : this->data){
                for (auto t : d){
                    if (t <= time) continue;
                    if (t < min_time || min_time == -1) min_time = t; 
                    break;
                }
            }
            return min_time;
        };

        auto data_at_time = [this](int time) {
            uint32_t result = 0;
            for (int i = 0; i < this->width; i++){
                uint32_t d1b = 0;
                for (auto &d : data[i]){
                    if(d < time) d1b = !d1b;
                    else break;
                }
                result += d1b << i;
            }
            return result;
        };

        draw_template(pos, pge, get_next_time, data_at_time, start_time, end_time, time_per_px, width != 1);
    }

    void draw_template(olc::vi2d pos,
                       olc::PixelGameEngine &pge,
                       std::function<int(int)> get_next_time,
                       std::function<int(int)> data_at_time,
                       uint32_t start_time,
                       uint32_t end_time,
                       float time_per_px,
                       bool bus = false
    ){
        bool drawing = false;
        int screen_x = pos.x;
        int last_d = 0;
        
        uint32_t d = start_time;

        while (d != -1){
            int val = data_at_time(d);
            int new_screen_x;
            bool should_stop = false;

            if (d > end_time){
                new_screen_x = screen_x+float(end_time-last_d)/time_per_px;
                should_stop = true;
            } else {
                if (last_d < start_time){
                    new_screen_x = screen_x+float(d-start_time)/time_per_px;
                } else {
                    new_screen_x = screen_x+float(d-last_d)/time_per_px;
                }
            }

            //todo can abstract this to lambdas probably
            if (bus){
                pge.DrawRect({screen_x, pos.y}, {new_screen_x-screen_x, *height}, olc::GREEN);
                std::stringstream ss;
                //todo for radix, this needs to change
                ss << width << "'h" << std::setw(width) << std::hex << val;
                pge.DrawString({screen_x + 2, pos.y + 1}, ss.str());
                if (should_stop) break; 
            } else {
                pge.DrawLine({screen_x, pos.y + *height - val * *height}, {new_screen_x, pos.y + *height - val * *height}, olc::GREEN);
                if (should_stop) break; 
                pge.DrawLine({new_screen_x, pos.y}, {new_screen_x, pos.y+*height}, olc::GREEN);
            }
            screen_x = new_screen_x;
            last_d = d;
            d = get_next_time(d);
        }
    }
};

class WaveStore {
    std::vector<Wave*> waves;
    int wave_height = 10;
    int v_gap = 5;
public:
    WaveStore() {
        for (int i = 0; i < 1; i++)
            waves.push_back(new Wave(&wave_height));
    }

    int get_visible_wave_count(){
        return waves.size();
    }

    std::string get_visible_wave_name(int num){
        return waves[num]->name;
    }

    Wave* get_visible_wave(int num){
        return waves[num];
    }

    int get_v_offset(int num){
        return wave_height + num*(v_gap + wave_height);
    }
};

class NamePane {
public:
    olc::vi2d window_pos;
    olc::vi2d window_size;
    float *proportion;
    WaveStore &ws;

    State &state;

    NamePane(olc::vi2d pos, olc::vi2d size, float *proportion, WaveStore &ws, State &state) :
        window_pos(pos),
        window_size(size),
        proportion(proportion),
        ws(ws),
        state(state)
    {

    }

    void draw(olc::PixelGameEngine &pge){
        pge.DrawRect(get_pos(), get_size());
    
        for (int i = 0; i < ws.get_visible_wave_count(); i++){
            std::string name = ws.get_visible_wave_name(i);
            olc::vi2d size = pge.GetTextSize(name);
            bool cut = false;
            if (get_size().x == 0){
                return;
            }
            if (size.x > get_size().x){
                int len = std::floor(name.size()*get_size().x/size.x) - 1;
                name = name.substr(0, len) + ">";
                cut = true;
            }

            //10+2 are magic numbers for the timeline width + offset in wave window
            pge.DrawString(get_pos() + olc::vi2d(0, 2+ws.get_v_offset(i) + state.timeline_width + 2), name);
        }
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


    State &state;

    WavePane(olc::vi2d pos, olc::vi2d size, float *proportion, WaveStore &ws, State &state) :
        window_pos(pos),
        window_size(size),
        proportion(proportion),
        ws(ws),
        state(state)
    {

    }

    void draw_timeline(olc::PixelGameEngine &pge){
        //bar
        pge.FillRect(get_pos(), {get_size().x, state.timeline_width});
        int interval = 100; //what time interval to draw markers

        float time = state.start_time;
        for (int i = 0; i < get_size().x; i++){
            if ((int(time) - state.start_time) % interval == 0){
                pge.FillRect(olc::vi2d(i, 4) + get_pos(), {2, state.timeline_width-4}, olc::BLACK);
                std::stringstream ss;
                ss << int(time);
                
                if (i + 3 +  pge.GetTextSize(ss.str()).x <= get_size().x)
                    pge.DrawString(olc::vi2d(i+3, 2) + get_pos(), ss.str(), olc::BLACK);
            }
            time += state.time_per_px;
        }
    }

    void draw(olc::PixelGameEngine &pge){
        pge.DrawRect(get_pos(), get_size());
        draw_timeline(pge);


        uint32_t end_time =  get_size().x*state.time_per_px + state.start_time;

        for (int i = 0; i < ws.get_visible_wave_count(); i++){
            ws.get_visible_wave(i)->draw(
                get_pos() + olc::vi2d(0, ws.get_v_offset(i) + state.timeline_width + 2),
                state.start_time, end_time, 
                state.time_per_px, pge
            );
        }
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
    State state; 

    olc::vi2d pos = {0,0};
    olc::vi2d size = {1500, 1000};

    NamePane name_pane;
    WavePane wave_pane;

    WaveStore ws;
    bool changed = false;
    
    WaveWindow() : 
        name_pane(NamePane(pos, size, &state.name_width, ws, state)),
        wave_pane(WavePane(pos, size, &state.name_width, ws, state)) 
    {

    }

    void update(olc::PixelGameEngine &pge){
        changed = !state.update(pge);
    }

    int i = 0;
    void draw(olc::PixelGameEngine &pge){
        i++;
        name_pane.draw(pge);
        wave_pane.draw(pge);
    }

    bool has_changed(bool clear = true){
        bool v = changed;
        if (clear)
            changed = false;
        return v;
    }
};


class WaveGUI : public olc::PixelGameEngine
{
public:
    WaveWindow wave_window;
    bool firstframe = true;
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
        wave_window.update(*this);
        if (firstframe || wave_window.has_changed()){
            Clear(olc::BLACK);
            wave_window.draw(*this);
            firstframe = false;
        }
        return !GetKey(olc::Q).bPressed;
    }
};


int main()
{
    srand(time(NULL));
    WaveGUI game;
    if(game.Construct(1920, 1080, 1, 1))
        game.Start();

    return 0;
}


