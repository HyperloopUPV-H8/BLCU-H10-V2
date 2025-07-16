#include "BLCU/Comms.hpp"
#include "BLCU/BLCU.hpp" 


Comms* Comms::self = nullptr;

Comms::Comms(BLCU* board): board(board){
    self = this;
    tcp_socket = new ServerSocket(board->ip, board->port);
    write_program_order = new HeapOrder(700, cb_write_program, &board->current_target);
    reset_all_order   = new HeapOrder(701, cb_reset_all);
    read_program_order = new HeapOrder(702, cb_read_program, &board->current_target);

}

void Comms::cb_write_program() { self->write_program(); }
void Comms::cb_reset_all()     { self->reset_all(); }
void Comms::cb_read_program() { self->read_program(); }
void Comms::send_ack() {
    tcp_socket->send_order(ack);
}

void Comms::send_nack() {
    tcp_socket->send_order(nack);
}

void Comms::read_program(){
    if(board->specific_state_machine.current_state != board->SpecificStates::READY){
        send_nack();
        return;
    }
    board->specific_state_machine.force_change_state(board->SpecificStates::BOOTING);

    BTFTP::on(BTFTP::Mode::READ);

    send_ack();
}

void Comms::write_program(){
    if(board->specific_state_machine.current_state != board->SpecificStates::READY){
        send_nack();
        return;
    }
    board->booting_flag = false;

    BTFTP::on(BTFTP::Mode::WRITE);

    send_ack();
}

void Comms::erase_program(){
    if(board->specific_state_machine.current_state != board->SpecificStates::READY){
        send_nack();
        return;
    }
    board->specific_state_machine.force_change_state(board->SpecificStates::BOOTING);

    if(not FDCB::erase_memory()){
        send_nack();
        board->abort_booting();
        return;
    }else{
        send_ack();
    }

    board->specific_state_machine.force_change_state(board->SpecificStates::READY);
}

void Comms::get_version(){
    if(board->specific_state_machine.current_state != board->SpecificStates::READY){
        send_ack();
        return;
    }
    board->specific_state_machine.force_change_state(board->SpecificStates::BOOTING);

    uint8_t temporal_value = 0;
    if (not FDCB::get_version(temporal_value)){
        send_nack();
        board->abort_booting();
        return;
    }

    send_ack();

    board->specific_state_machine.force_change_state(board->SpecificStates::READY);
}

void Comms::reset_all() {
    board->reset_all();
}

