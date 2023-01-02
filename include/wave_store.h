#ifndef __WAVE_STORE_H__
#define __WAVE_STORE_H__

#include <vector>
#include <string>
#include <tuple>
#include <unordered_map>
#include <sstream>
#include "olcPixelGameEngine.h"

struct Var{
    std::string id;
    std::string name;
    int width;
    int initial;
    std::vector<std::tuple<int,int>> value;
    
    //TODO: apply a mask here to indicate times bits toggle
    std::vector<std::vector<int>> x_mask;

    Var(int width, std::string id, std::string name);
    void add_change(int time, int next_value);
    int val_at_time(int time) const;
    int get_next_time(int time) const;
    int get_prev_time(int time) const;
};

std::ostream& operator<< (std::ostream &out, Var const& data);

struct VarStore{
    std::unordered_map<std::string,int> var_map;
    std::vector<Var> vars;
    std::vector<Var> get_vars();
    Var& get_var_by_name(std::string name);
    void add_key(int width, std::string symbol, std::string name);
    void add_change(std::string key, int time, int value);
    void parse_var(std::vector<std::string> var);
};

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
#endif