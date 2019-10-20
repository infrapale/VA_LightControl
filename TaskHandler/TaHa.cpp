#include <Arduino.h>
#include "TaHa.h"


TaHa::TaHa(){
    running = false;
    last_time = millis();
};

void TaHa::run(){
    if (running) {
        if(millis() > next_time){
            switch(operation){
            case RUN_RECURRING:
                last_time = millis();
                next_time = last_time + interval;
                break;
            case RUN_ONCE:
                running= false;
                break;   
            }
            (*task)();   //run given task         
        }
    }

}

void TaHa::set_interval(long ival, int operation, taha_callback func ){
    this->operation = operation;
    this->interval = ival;
    this->task = func;
    this->last_time = millis();
    this->next_time = this->last_time + this->interval;
    this->running = true;
}
void TaHa::stop_task( ){
    running = false;
}

void TaHa::restart_task(){
    running = true;
}

void TaHa::delay_task(long delay_time){
    next_time += delay_time;
}
