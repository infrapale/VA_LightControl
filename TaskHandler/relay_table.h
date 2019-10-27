#define NUMBER_RELAY_ENTRIES 16

 struct relay_table_struct {
     char key;
     char type;
     char zone[4];
     char sub_index;
     char operation;
};


relay_table_struct key_function_table[NUMBER_RELAY_ENTRIES] = {
    {'0','R',"ALL",'G','0'},
    {'1','R',"MH1",'1','T'},
    {'2','R',"MH2",'1','T'},
    {'3','R',"ET_",'1','T'},
    {'4','R',"MH1",'2','T'},
    {'5','R',"MH2",'2','T'},
    {'6','R',"K__",'1','T'},
    {'7','R',"K__",'2','T'},
    {'8','R',"TUP",'1','T'},
    {'9','R',"TUP",'2','T'},
    {'*','R',"ALL",'1','1'},
    {'#','R',"PAR",'1','T'},
    {'A','R',"KHH",'1','T'},
    {'B','R',"PSH",'1','T'},
    {'C','R',"SAU",'1','T'},
    {'D','R',"ULK",'1','T'}
};

/*            
            case '0': send_radio('R',"ALL",'G','0'); break;
            case '1': send_radio('R',"MH1",'1','T'); break;
            '2','R',"MH2",'1','T'
            '3','R',"ET_",'1','T'
            '4','R',"MH1",'2','T'
            '5','R',"MH2",'2','T'
            '6','R',"K__",'1','T'
            '7','R',"K__",'2','T'
            '8','R',"TUP",'1','T'
            '9','R',"TUP",'2','T'
            '*','R',"ALL",'1','1'
            '#','R',"PAR",'1','T'
            'A','R',"KHH",'1','T'
            'B','R',"PSH",'1','T'
            'C','R',"SAU",'1','T'
            'D','R',"ULK",'1','T'
  */
