ifndef __EHA_H__
#define __EHA_H__

#define EHA_MAX_EVENT_HANDLES  4
#define EHA_MAX_BUFFS          8
#define EHA_DATA_LEN           64

typedef void (*eha_callback)(void);

class EHa {
    public:
        EHa();
        void add(char *data);
        char *get_event(uint8_t last_event);
        //void subscribe(char *event, taha_callback func );
        //void unsubscribe(char *event, taha_callback func );
    private:
        char eBuff[EHA_MAX_BUFFS][EHA_DATA_LEN];
        byte eMatrix[EHA_MAX_EVENT_HANDLES];
};

#endif
