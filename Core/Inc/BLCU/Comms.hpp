#pragma once
#include "ST-LIB.hpp"

class BLCU;

class Comms{
private:
    BLCU* board=nullptr;
    HeapOrder* write_program_order=nullptr;
    HeapOrder*  reset_all_order=nullptr;
    // static HeapOrder* get_version_order;
    // static HeapOrder* read_program_order;
    // static HeapOrder* erase_program_order;

public:
    Comms(BLCU* board);

    ServerSocket* tcp_socket=nullptr;

    void read_program();
    void reset_all();
    void get_version();
    void write_program();
    void erase_program();
    
    HeapOrder ack = {
        3,
    };
    
    HeapOrder nack = {
        4,
    };

};




