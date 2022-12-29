#include "pge_wave.h"

//Test function
std::vector<uint32_t> random_wave(int len){
    std::vector<uint32_t> data;

    for (int i = 0; i < len; i++){
        data.push_back(rand()%1500);
    }

    std::sort(data.begin(), data.end());

    return data;
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
    int last_d = 0;
    bool draw_to_end_time = false;
    
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
        if (width > 1){
            pge.DrawRect({screen_x, pos.y}, {new_screen_x-screen_x, *height}, olc::GREEN);
            std::stringstream ss;
            //todo for radix, this needs to change
            ss << width << "'h"  << std::hex << val;
            int width = pge.GetTextSize(ss.str()).x;
            if (width < new_screen_x - screen_x){
                pge.DrawString({screen_x + 2, pos.y + 1}, ss.str());
            }
            if (should_stop) break; 
        } else {
            pge.DrawLine({screen_x, pos.y + *height - val * *height}, {new_screen_x, pos.y + *height - val * *height}, olc::GREEN);
            if (should_stop) break; 
            pge.DrawLine({new_screen_x, pos.y}, {new_screen_x, pos.y+*height}, olc::GREEN);
        }
        screen_x = new_screen_x;
        last_d = d;
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

State::State(olc::PixelGameEngine &pge, WaveStore &ws) : 
    pge(pge), 
    ws(ws)
{}

bool State::update(){
    got_input = false;

    common_inputs();
    wave_inputs();
    picker_inputs();

    return !got_input;
}

void State::common_inputs(){
    if (input(olc::SPACE)) picker_show = !picker_show;
}

void State::cursor_update(e_cursor_dir dir){
    int wave_count = ws.get_visible_wave_count();

    //may be a nullptr if the index is invalid
    Var* selected_instance = ws.get_var_by_index(cursor_visble_wave_index);
    int next_cursor_time;
    
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

void State::wave_inputs(){
    if (picker_show) return;        

    if (input(olc::LEFT)) cursor_update(LEFT);
    if (input(olc::RIGHT)) cursor_update(RIGHT);
    if (input(olc::UP)) cursor_update(UP);
    if (input(olc::DOWN)) cursor_update(DOWN);
    if (input(olc::N)) name_width = std::max(0.0f, name_width - 0.05f);
    if (input(olc::M)) name_width = std::min(1.0f, name_width + 0.05f);
    if (input(olc::Z) && input(olc::SHIFT, true)) time_per_px = std::min(2048.0f, time_per_px * 2); //zoom out
    if (input(olc::Z) && !input(olc::SHIFT, true)) time_per_px = std::max(0.125f, time_per_px / 2); //zoom in
}

void State::picker_inputs(){
    if (!picker_show) return;

    if (input(olc::UP)) picker_index = std::max(0, picker_index-1);
    if (input(olc::DOWN)) picker_index = std::min(ws.get_raw_wave_count()-1, picker_index+1);
    if (input(olc::ENTER)) ws.create_instance(picker_index);
}


bool State::input(olc::Key key, bool held){
    bool res = false;
    if (held)
        res = pge.GetKey(key).bHeld;
    else
        res = pge.GetKey(key).bPressed;
    this->got_input |= res;
    return res;
};

NamePane::NamePane(olc::vi2d pos, olc::vi2d size, float *proportion, WaveStore &ws, State &state) :
    window_pos(pos),
    window_size(size),
    proportion(proportion),
    ws(ws),
    state(state)
{

}

void NamePane::draw(olc::PixelGameEngine &pge){
    pge.DrawRect(get_pos(), get_size());
    int max_name_width = get_size().x - 8;

    for (int i = 0; i < ws.get_visible_wave_count(); i++){
        std::string name = ws.get_visible_wave_name(i);
        olc::vi2d size = pge.GetTextSize(name);
        if (max_name_width <= 0){
            return;
        }
        if (size.x > max_name_width){
            int len = std::floor(name.size()*max_name_width/size.x) - 1;
            name = name.substr(0, len) + ">";
        }

        //2 is a magic number for v offset at top of wave window
        int y_offset = 2+ws.get_v_offset(i) + state.timeline_width + 2;
        pge.DrawString(get_pos() + olc::vi2d(0, y_offset), name);

        if (i == state.cursor_visble_wave_index){
            pge.DrawString(get_pos() + olc::vi2d(max_name_width, y_offset), "*");
        }
    }
}

olc::vi2d NamePane::get_pos(){
    return window_pos;
}

olc::vi2d NamePane::get_size(){
    return window_size - olc::vi2d(window_size.x*(1.0f-*proportion), 0);
}

WavePane::WavePane(olc::vi2d pos, olc::vi2d size, float *proportion, WaveStore &ws, State &state) :
    window_pos(pos),
    window_size(size),
    proportion(proportion),
    ws(ws),
    state(state)
{

}

void WavePane::draw_timeline(olc::PixelGameEngine &pge){
    //bar
    pge.FillRect(get_pos(), {get_size().x, state.timeline_width});
    
    int time_width = get_size().x * state.time_per_px;
    int interval = time_width/10; //what time interval to draw markers

    float time = state.start_time;
    int last_time = -1;
    for (int i = 0; i < get_size().x; i++){
        if ((int(time) - state.start_time) % interval == 0 && int(time) != last_time){
            pge.FillRect(olc::vi2d(i, 4) + get_pos(), {2, state.timeline_width-4}, olc::BLACK);
            std::stringstream ss;
            ss << int(time);
            
            if (i + 3 +  pge.GetTextSize(ss.str()).x <= get_size().x){
                pge.DrawString(olc::vi2d(i+3, 2) + get_pos(), ss.str(), olc::BLACK);
            }
            last_time = int(time);
        }
        time += state.time_per_px;
    }
}

void WavePane::draw_waves(olc::PixelGameEngine &pge){
    uint32_t end_time =  std::min((float)ws.get_end_time(), get_size().x*state.time_per_px + state.start_time);

    for (int i = 0; i < ws.get_visible_wave_count(); i++){
        ws.get_visible_wave(i).draw(
            get_pos() + olc::vi2d(0, ws.get_v_offset(i) + state.timeline_width + 2),
            state.start_time, end_time, 
            state.time_per_px, pge
        );
    }
}

void WavePane::draw_cursor(olc::PixelGameEngine &pge){
    int end_time =  get_size().x*state.time_per_px + state.start_time;
    if (state.cursor_time < state.start_time || state.cursor_time > end_time) return;

    int x = get_pos().x + ((state.cursor_time - state.start_time)/state.time_per_px );
    pge.DrawLine({x, get_pos().y+state.timeline_width}, {x, get_pos().y+get_size().y}, state.cursor_colour);
}

void WavePane::draw(olc::PixelGameEngine &pge){
    draw_timeline(pge);
    draw_waves(pge);
    draw_cursor(pge);
    pge.DrawRect(get_pos(), get_size());
}

olc::vi2d WavePane::get_pos(){
    return window_pos + olc::vi2d(window_size.x* *proportion, 0);
}

olc::vi2d WavePane::get_size(){
    return window_size - olc::vi2d(window_size.x* *proportion, 0);
}

WavePicker::WavePicker(olc::vi2d rootpos, olc::vi2d rootsize, State &state, WaveStore &ws) :
    state(state),
    ws(ws),
    pos(rootpos + state.picker_border*olc::vi2d(1,1)),
    size(rootsize - 2*state.picker_border*olc::vi2d(1,1))
{}

void WavePicker::draw(olc::PixelGameEngine &pge){
    if (!state.picker_show) return;
    pge.FillRect(pos, size, olc::BLACK);
    pge.DrawRect(pos, size, olc::WHITE);

    olc::vi2d drawpos = pos + olc::vi2d(3,3);
    std::string selector = " > ";
    olc::vi2d selector_offs = {pge.GetTextSize(selector).x, 0};
    for (int i = 0; i < ws.get_raw_wave_count(); i++){
        if (i == state.picker_index) 
            pge.DrawString(drawpos, selector);
        pge.DrawString(drawpos + selector_offs, ws.get_raw_wave(i)->name);
        drawpos += {0, pge.GetTextSize(ws.get_raw_wave(i)->name).y};
    }
}

WaveWindow::WaveWindow(olc::PixelGameEngine &pge, VarStore &store) : 
    pge(pge),
    state(pge, ws),
    name_pane(NamePane(pos, size, &state.name_width, ws, state)),
    wave_pane(WavePane(pos, size, &state.name_width, ws, state)),
    wave_picker(WavePicker(pos, size, state, ws)),
    ws(store)
{

}

void WaveWindow::draw(){
    if (!firstframe && state.update()) return;
    pge.Clear(olc::BLACK);
    name_pane.draw(pge);
    wave_pane.draw(pge);
    wave_picker.draw(pge);
    firstframe=false;
}