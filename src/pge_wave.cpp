#include "pge_wave.h"

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
    int max_text_width = get_size().x - 8;

    if (max_text_width <= 0){
        return;
    }

    for (int i = 0; i < ws.get_visible_wave_count(); i++){
        //Value to draw
        std::string text = state.show_values ? get_value_text(i) : get_name_text(i);

        //If the value is too wide, chop it down and add a '>' at the end
        olc::vi2d size = pge.GetTextSize(text);
        if (size.x > max_text_width){
            int len = std::floor(text.size()*max_text_width/size.x) - 1;
            text = text.substr(0, len) + ">";
        }

        //2 is a magic number for v offset at top of wave window
        int y_offset = 2+ws.get_v_offset(i) + state.timeline_width + 2;
        pge.DrawString(get_pos() + olc::vi2d(0, y_offset), text);

        //Draw a '*' to indicate the selected signal
        if (i == state.cursor_visble_wave_index){
            pge.DrawString(get_pos() + olc::vi2d(max_text_width, y_offset), "*");
        }
    }
}

std::string NamePane::get_value_text(int wave_index){
    int width = ws.get_visible_wave(wave_index).wave->width;
    int value = ws.get_var_by_index(wave_index)->val_at_time(state.cursor_time);

    //TODO: store radix value in waveinstance and apply that here
    if (width > 1){
        std::stringstream ss;
        ss << width << "'h" << std::hex << value;
        return ss.str();
    } else {
        return value == 1 ? "1" : "0";
    }
}

std::string NamePane::get_name_text(int wave_index){
    std::string name = ws.get_visible_wave_name(wave_index);
    return name;
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

ValuePane::ValuePane(olc::vi2d rootpos, olc::vi2d rootsize, WaveStore &ws, State &state) :
    state(state),
    ws(ws),
    pos(rootpos + state.picker_border*olc::vi2d(1,1)),
    size(rootsize - 2*state.picker_border*olc::vi2d(1,1))
{}

void ValuePane::draw(olc::PixelGameEngine &pge){
    pge.DrawRect(get_pos(), get_size());
}

olc::vi2d ValuePane::get_pos(){
    return pos;
}

olc::vi2d ValuePane::get_size(){
    return size;
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

    olc::vi2d drawpos = pos + olc::vi2d(3,5);
    std::string selector = " > ";
    olc::vi2d selector_offs = {pge.GetTextSize(selector).x, 0};
    for (int i = 0; i < ws.get_raw_wave_count(); i++){
        if (i == state.picker_index) 
            pge.DrawString(drawpos, selector);

        std::stringstream ss;
        ss << ws.get_raw_wave(i)->width << " " << ws.get_raw_wave(i)->name;

        pge.DrawString(drawpos + selector_offs, ss.str());
        drawpos += {0, pge.GetTextSize(ss.str()).y + 2};
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