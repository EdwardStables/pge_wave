#include "olcPixelGameEngine.h"
#include <vector>
#include <string>
#include <sstream>
#include <time.h>
#include <iostream>

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
    std::vector<uint32_t> data;
    int *height;
    int *width;

    Wave(int *height, int *width) :
        height(height),
        width(width)
    {
        data = random_wave(10);
        std::stringstream ss;
        ss << "wave_" << rand();
        name = ss.str();
    }

    void draw(olc::vi2d pos, uint32_t start_time, uint32_t end_time, uint32_t scale, olc::PixelGameEngine &pge){
        bool drawing = false;
        uint8_t val = 0; //for now assuming all data starts at 0
        uint32_t screen_x = pos.x;
        uint32_t last_d = 0;
        for (auto &d : data) {
            if (!drawing){
                if (d < start_time)
                    continue;
                else
                    drawing = true;
            }

            uint32_t new_screen_x;
            bool should_stop = false;

            if (d > end_time){
                new_screen_x = screen_x+scale*(end_time-last_d);
                should_stop = true;
            } else {
                new_screen_x = screen_x+scale*(d-last_d);
            }

            pge.DrawLine({screen_x, pos.y + *height - val * *height}, {new_screen_x, pos.y + *height - val * *height}, olc::GREEN);

            if (should_stop) break; 

            pge.DrawLine({new_screen_x, pos.y}, {new_screen_x, pos.y+*height}, olc::GREEN);

            val = val == 0 ? 1 : 0;
            screen_x = new_screen_x;
            last_d = d;
        }
    }
};

class WaveStore {
    std::vector<Wave*> waves;
    int wave_height = 10;
    int v_gap = 5;
    int wave_width = 10;
public:
    WaveStore() {
        for (int i = 0; i < 10; i++)
            waves.push_back(new Wave(&wave_height, &wave_width));
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

    NamePane(olc::vi2d pos, olc::vi2d size, float *proportion, WaveStore &ws) :
        window_pos(pos),
        window_size(size),
        proportion(proportion),
        ws(ws)
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
            pge.DrawString(get_pos() + olc::vi2d(0, 2+ws.get_v_offset(i) + 10 + 2), name);
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

    uint32_t x_px_scale = 1; //1ns
    uint32_t start_time = 0; //start at time zero

    int timeline_width = 10;

    WavePane(olc::vi2d pos, olc::vi2d size, float *proportion, WaveStore &ws) :
        window_pos(pos),
        window_size(size),
        proportion(proportion),
        ws(ws)
    {

    }

    void draw_timeline(olc::PixelGameEngine &pge){
        pge.FillRect(get_pos(), {get_size().x, timeline_width});
        uint32_t time = start_time;
        for (int i = 0; i < get_size().x; i++){
            if (time % 100 == 0){
                pge.FillRect(olc::vi2d(i, 4) + get_pos(), {2, timeline_width-4}, olc::BLACK);
                std::stringstream ss;
                ss << time;
                
                if (i + 3 +  pge.GetTextSize(ss.str()).x <= get_size().x)
                    pge.DrawString(olc::vi2d(i+3, 2) + get_pos(), ss.str(), olc::BLACK);
            }
            time += x_px_scale;
        }
    }

    void draw(olc::PixelGameEngine &pge){
        pge.DrawRect(get_pos(), get_size());
        draw_timeline(pge);


        uint32_t end_time = x_px_scale * get_size().x + start_time;
        for (auto &d : ws.get_visible_wave(0)->data)
            std::cout << d << ", ";
        std::cout << std::endl;

        for (int i = 0; i < ws.get_visible_wave_count(); i++){
            ws.get_visible_wave(i)->draw(get_pos() + olc::vi2d(0, ws.get_v_offset(i) + timeline_width + 2), start_time, end_time, x_px_scale, pge);
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
    
    struct State {
        float name_width = 0.1f;
        uint32_t get_checkpoint(){
            //TODO for more entries, do something fancier 
            return 100 * name_width;
        }
    } state;

    olc::vi2d pos = {0,0};
    olc::vi2d size = {1500, 1000};

    NamePane name_pane;
    WavePane wave_pane;

    WaveStore ws;
    bool changed = false;
    
    WaveWindow() : 
        name_pane(NamePane(pos, size, &state.name_width, ws)),
        wave_pane(WavePane(pos, size, &state.name_width, ws)) 
    {

    }

    void update(olc::PixelGameEngine &pge){
        uint32_t state_checkpoint = state.get_checkpoint();

        if (pge.GetKey(olc::LEFT).bPressed) {
            state.name_width = std::max(0.0f, state.name_width - 0.05f);
        }
        if (pge.GetKey(olc::RIGHT).bPressed) {
            state.name_width = std::min(1.0f, state.name_width + 0.05f);
        }
    
        if (state_checkpoint != state.get_checkpoint()){
            changed = true;
        }
    }

    void draw(olc::PixelGameEngine &pge){
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


