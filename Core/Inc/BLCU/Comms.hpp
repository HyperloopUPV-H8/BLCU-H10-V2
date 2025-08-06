#pragma once
#include "ST-LIB.hpp"
#include "BLCU/BLCU.hpp"  

class Comms {
private:
    Comms() = delete;
public:
    static void init();
    static ServerSocket* tcp_socket; 

    inline static HeapOrder* write_program_order=nullptr; 
    inline static HeapOrder* reset_all_order=nullptr;
    //More orders to be added, for now there is only the usefull ones :)
    //inline static HeapOrder* read_program_order=nullptr;
    
    static void read_program();
    static void get_version();
    static void write_program();
    static void erase_program();

    static void send_ack();
    static void send_nack();
    
    inline static HeapOrder ack = HeapOrder(3);
    inline static HeapOrder nack = HeapOrder(4);
};




