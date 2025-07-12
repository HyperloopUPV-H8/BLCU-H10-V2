#include "BLCU/TCP.hpp"

TCP::write_program_order = new HeapOrder{
    700,
    BLCU::write_program,
    &BLCU::orders_data.target,
};

TCP::reset_all_order = new HeapOrder{
    704,
    BLCU::reset_all,
    &BLCU::orders_data.target,
};

TCP::board = nullptr;
TCP::tcp_socket = nullptr;

TCP::TCP(BLCU* board){
    this->board = board;
    tcp_socket = new ServerSocket(BLCU->ip, BLCU->port,1000,500,10);

}