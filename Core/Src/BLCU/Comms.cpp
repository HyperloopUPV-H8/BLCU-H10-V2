#include "BLCU/Comms.hpp"
#include "BLCU/BLCU.hpp"

ServerSocket* Comms::tcp_socket = nullptr;

void Comms::init(){
    tcp_socket = new ServerSocket(BLCU::ip, BLCU::port);
    write_program_order = new HeapOrder(700, Comms::write_program, &BLCU::current_target);
    reset_all_order   = new HeapOrder(701, Comms::reset_all,   &BLCU::current_target);
    read_program_order  = new HeapOrder(702, Comms::read_program,  &BLCU::current_target);
}

void Comms::send_ack() {
    tcp_socket->send_order(ack);
}

void Comms::send_nack() {
    tcp_socket->send_order(nack);
}

void Comms::read_program(){
    if(BLCU::specific_state_machine.current_state != BLCU::SpecificStates::READY){
        send_nack();
        return;
    }
    BLCU::specific_state_machine.force_change_state(BLCU::SpecificStates::BOOTING);

    BTFTP::on(BTFTP::Mode::READ);

    send_ack();
}

void Comms::write_program(){
    if(BLCU::specific_state_machine.current_state != BLCU::SpecificStates::READY){
        send_nack();
        return;
    }
    BLCU::ready_flag = false;

    BTFTP::on(BTFTP::Mode::WRITE);

    send_ack();
}

void Comms::erase_program(){
    if(BLCU::specific_state_machine.current_state != BLCU::SpecificStates::READY){
        send_nack();
        return;
    }
    BLCU::specific_state_machine.force_change_state(BLCU::SpecificStates::BOOTING);

    if(not FDCB::erase_memory()){
        send_nack();
        BLCU::abort_booting();
        return;
    }else{
        send_ack();
    }

    BLCU::specific_state_machine.force_change_state(BLCU::SpecificStates::READY);
}

void Comms::get_version(){
    if(BLCU::specific_state_machine.current_state != BLCU::SpecificStates::READY){
        send_ack();
        return;
    }
    BLCU::specific_state_machine.force_change_state(BLCU::SpecificStates::BOOTING);

    uint8_t temporal_value = 0;
    if (not FDCB::get_version(temporal_value)){
        send_nack();
        BLCU::abort_booting();
        return;
    }

    send_ack();

    BLCU::specific_state_machine.force_change_state(BLCU::SpecificStates::READY);
}

