#include "BLCU/Comms.hpp"
#include "BLCU/BLCU.hpp" 



Comms::Comms(BLCU* board): board(board){
    tcp_socket = new ServerSocket(board->ip, board->port,1000,500,10);
    write_program_order = new HeapOrder{
        700,
        write_program,
        &board->current_target,
    };
    
    reset_all_order = new HeapOrder{
        704,
        reset_all
    };

}

void Comms::read_program(){
    if(board->specific_state_machine.current_state != board->SpecificStates::READY){
        tcp_socket->send_order(nack);
        return;
    }
    board->specific_state_machine.force_change_state(board->SpecificStates::BOOTING);

    BTFTP::on(BTFTP::Mode::READ);

    tcp_socket->send_order(ack);
}

void Comms::write_program(){
    if(board->specific_state_machine.current_state != board->SpecificStates::READY){
        tcp_socket->send_order(nack);
        return;
    }
    board->specific_state_machine.force_change_state(board->SpecificStates::BOOTING);

    BTFTP::on(BTFTP::Mode::WRITE);

    tcp_socket->send_order(ack);
}

void Comms::erase_program(){
    if(board->specific_state_machine.current_state != board->SpecificStates::READY){
        tcp_socket->send_order(nack);
        return;
    }
    board->specific_state_machine.force_change_state(board->SpecificStates::BOOTING);

    if(not FDCB::erase_memory()){
        tcp_socket->send_order(nack);
        board->abort_booting();
        return;
    }else{
        tcp_socket->send_order(ack);
    }

    board->specific_state_machine.force_change_state(board->SpecificStates::READY);
}

void Comms::get_version(){
    if(board->specific_state_machine.current_state != board->SpecificStates::READY){
        tcp_socket->send_order(nack);
        return;
    }
    board->specific_state_machine.force_change_state(board->SpecificStates::BOOTING);

    uint8_t temporal_value = 0;
    if (not FDCB::get_version(temporal_value)){
        tcp_socket->send_order(nack);
        board->abort_booting();
        return;
    }

    tcp_socket->send_order(ack);

    board->specific_state_machine.force_change_state(board->SpecificStates::READY);
}

