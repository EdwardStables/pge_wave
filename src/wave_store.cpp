#include "wave_store.h"



std::ostream& operator<< (std::ostream &out, Var const& data){
    out << data.name << " " << data.width << " " << data.id;
    return out;
}