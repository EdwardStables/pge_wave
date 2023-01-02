#include "olcPixelGameEngine.h"

//button press macros
#define PRESS(key) !input(olc::SHIFT, true) && input(olc::key)
#define HOLD(key) !input(olc::SHIFT, true) && input(olc::key, true) 
#define SHIFT_PRESS(key) input(olc::SHIFT, true) && input(olc::key)
#define UNBOUND false //using this may lead to features not working
#define NUM_PRESS(key, value) !input(olc::SHIFT, true) && handle_number_input(olc::key, value) 

//key bindings
#define TOGGLE_VALUE PRESS(V)
#define MOVE_LEFT PRESS(H)
#define MOVE_DOWN PRESS(J)
#define MOVE_UP PRESS(K)
#define MOVE_RIGHT PRESS(L)
#define DELETE PRESS(X)
#define EXPAND_WAVE_WINDOW PRESS(LEFT)
#define REDUCE_WAVE_WINDOW PRESS(RIGHT)
#define ZOOM_IN SHIFT_PRESS(Z)
#define ZOOM_OUT PRESS(Z)
#define CONFIRM PRESS(ENTER)
#define TOGGLE_PICKER PRESS(SPACE)

//These belong here for consistency and keeping the input handler clean
//Do not rebind 
#define NUM_0 NUM_PRESS(K0, 0)
#define NUM_1 NUM_PRESS(K1, 1)
#define NUM_2 NUM_PRESS(K2, 2)
#define NUM_3 NUM_PRESS(K3, 3)
#define NUM_4 NUM_PRESS(K4, 4)
#define NUM_5 NUM_PRESS(K5, 5)
#define NUM_6 NUM_PRESS(K6, 6)
#define NUM_7 NUM_PRESS(K7, 7)
#define NUM_8 NUM_PRESS(K8, 8)
#define NUM_9 NUM_PRESS(K9, 9)