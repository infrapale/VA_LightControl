#ifndef __RELAYS_H__
#define __RELAYS_H__
#define ZONE_LEN 4
struct relays_struct = {
   char zone[ZONE_LEN];
   char sub_index;
    
}

relays_struct all_relays = {
    {"MH1",'1'},
    {"MH1",'2'},
    {"MH2",'1'},
    {"MH2",'2'},
    {"ET_",'1'},
    {"WC_",'1'},
    {"WC_",'2'},
    {"WC_",'G'},
    {"K__",'1'}
}
#endif
