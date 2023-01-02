#include "wave_store.h"
#include <iostream>

Var::Var(int width, std::string id, std::string name) :
    id(id),
    name(name),
    width(width)
{}

void Var::add_change(int time, int next_value){
    auto tup = std::make_tuple(time, next_value);
    value.push_back(tup);
}

int Var::val_at_time(int time) const {
    int val = std::get<0>(value[0]) == 0 ? std::get<1>(value[0]) : 0;
    for (auto &v : value){
        if (std::get<0>(v) > time){
            return val;
        } else if (std::get<0>(v) == time){
            return std::get<1>(v);
        } else {
            val = std::get<1>(v);
        }
    }
    return val;
}

int Var::get_prev_time(int time) const{
    int prev = 0;
    for (auto &v : value){
        if (std::get<0>(v) >= time) return prev;
        prev = std::get<0>(v);
    }

    return prev;
}

int Var::get_next_time(int time) const{
    for (auto &v : value){
        if (std::get<0>(v) > time) return std::get<0>(v);
    }

    return -1;
}

void VarStore::add_key(int width, std::string symbol, std::string name){
    Var v(width, symbol, name);
    vars.push_back(v);
    var_map[symbol] = vars.size()-1;
}

void VarStore::add_change(std::string key, int time, int value){
    if (var_map.find(key) != var_map.end())
        vars[var_map[key]].add_change(time, value);
    else
        std::cout << "Warning, tried to add symbol '" << key << "', which doesn't exist." << std::endl;
}    

void VarStore::parse_var(std::vector<std::string> var){
    //TODO this is parsing code and should be in the vcd file
    std::stringstream ss;
    ss << var[1];
    int width;
    ss >> width;
    std::string symbol = var[2];
    std::string name = var[3];
    std::string type = var[0];
    add_key(width, symbol, name);
}

std::ostream& operator<< (std::ostream &out, Var const& data){
    out << data.name << " " << data.width << " " << data.id;
    return out;
}

std::vector<Var> VarStore::get_vars(){
    return vars;
}

Var& VarStore::get_var_by_name(std::string name){
    for(auto &v : vars){
        if (v.name == name) return v;
    }
}

Wave::Wave(std::string name, int *height, VarStore &data) :
    height(height),
    name(name),
    width(data.get_var_by_name(name).width), //this needs a nicer solution
    data(data)
{
}

void Wave::draw(olc::vi2d pos, uint32_t start_time, uint32_t end_time, float time_per_px, olc::PixelGameEngine &pge){
    bool drawing = false;
    int screen_x = pos.x;
    int last_val = 0;
    int last_d = 0;
    bool draw_to_end_time = false;
    bool first_entry = true;
    
    uint32_t d = start_time;
    while (true){
        int val = data.get_var_by_name(name).val_at_time(d);
        int new_screen_x;
        bool should_stop = false;

        if (d > end_time || draw_to_end_time){
            new_screen_x = screen_x+float(end_time-last_d)/time_per_px;
            should_stop = true;
        } else {
            if (last_d < start_time){
                new_screen_x = screen_x+float(d-start_time)/time_per_px;
            } else {
                new_screen_x = pos.x + (d-start_time)/time_per_px;
            }
        }

        //todo can abstract this to lambdas probably
        if (!first_entry){
            if (width > 1){
                pge.DrawRect({screen_x, pos.y}, {new_screen_x-screen_x, *height}, olc::GREEN);
                std::stringstream ss;
                //todo for radix, this needs to change
                ss << width << "'h"  << std::hex << last_val;
                int width = pge.GetTextSize(ss.str()).x;
                if (width < new_screen_x - screen_x){
                    pge.DrawString({screen_x + 2, pos.y + 1}, ss.str());
                }
                if (should_stop) break; 
            } else {
                pge.DrawLine({screen_x, pos.y + *height - last_val * *height}, {new_screen_x, pos.y + *height - last_val * *height}, olc::GREEN);
                if (should_stop) break; 
                pge.DrawLine({new_screen_x, pos.y}, {new_screen_x, pos.y+*height}, olc::GREEN);
            }
        }

        first_entry = false;
        screen_x = new_screen_x;
        last_d = d;
        last_val = val;
        d = data.get_var_by_name(name).get_next_time(d);

        //When beyond the range of the variable, still need to draw to the end of the visible range
        if (d == -1){
            d = end_time;
            draw_to_end_time = true;
        }
    }
}

WaveInstance::WaveInstance(Wave* wave, int max, int min, std::string name_override) :
    wave(wave),
    max(max==-1 ? wave->width-1 : max),
    min(min),
    name_override(name_override)
{}

std::string WaveInstance::get_name(){
    return name_override == "" ? wave->name : name_override;
}

void WaveInstance::draw(olc::vi2d pos, uint32_t start_time, uint32_t end_time, float time_per_px, olc::PixelGameEngine &pge)
{
    wave->draw(pos, start_time, end_time, time_per_px, pge);
}

WaveStore::WaveStore(VarStore &store) : varstore(store) {
    for (auto &w : varstore.get_vars()){
        waves.push_back(new Wave(w.name, &wave_height, varstore));
        end_time = std::max(end_time, std::get<0>(w.value.back()));
    }
}

void WaveStore::create_instance(int num){
    wave_instances.push_back(WaveInstance(get_raw_wave(num)));
}

int WaveStore::get_raw_wave_count(){
    return waves.size();
}

Wave* WaveStore::get_raw_wave(int num){
    return waves[num];
}

int WaveStore::get_visible_wave_count(){
    return wave_instances.size();
}

std::string WaveStore::get_visible_wave_name(int num){
    return wave_instances[num].get_name();
}

WaveInstance WaveStore::get_visible_wave(int num){
    return wave_instances[num];
}

int WaveStore::get_v_offset(int num){
    return wave_height + num*(v_gap + wave_height);
}

int WaveStore::get_end_time(){
    return end_time;
}

Var* WaveStore::get_var_by_index(int num){
    if (num < 0 || num >= wave_instances.size()){
        return nullptr;
    }

    return &(varstore.get_var_by_name(wave_instances[num].wave->name));
}
