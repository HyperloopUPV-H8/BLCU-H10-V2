#pragma once
#include "ST-LIB.hpp"

class BLCU;

class Comms{
private:
    static Comms* self; 
    static void cb_write_program();    
    static void cb_reset_all();
    static void cb_read_program();
    BLCU* board=nullptr;
    HeapOrder* write_program_order=nullptr;
    HeapOrder*  reset_all_order=nullptr;
    // static HeapOrder* get_version_order;
    HeapOrder* read_program_order;
    // static HeapOrder* erase_program_order;

public:
    Comms(BLCU* board);

    ServerSocket* tcp_socket=nullptr;

    void read_program();
    void reset_all();
    void get_version();
    void write_program();
    void erase_program();

    void send_ack();
    void send_nack();
    
    HeapOrder ack = {
        3,
    };
    
    HeapOrder nack = {
        4,
    };

};




