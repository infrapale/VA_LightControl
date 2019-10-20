#ifndef __TAHA_H__
#define __TAHA_H__

#define RUN_RECURRING 1
#define RUN_ONCE  2

typedef void (*taha_callback)(void);

class TaHa {
    public:
        TaHa();
        void run();
        void set_interval(long ival, int operation, taha_callback func );
        void stop_task( );
        void restart_task();
        void delay_task(long delay_time);
    private:
        taha_callback task;
        int operation;
        long last_time;
        long next_time;
        long interval;
        boolean running;

};

#endif
