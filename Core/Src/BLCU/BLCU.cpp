#include "BLCU/BLCU.hpp"


BLCU::BLCU(){
//-------STATE MACHINE------//
    setup_state_machine();
	setup_specific_state_machine();
	general_state_machine.add_state_machine(specific_state_machine, GeneralStates::OPERATIONAL);
	Time::register_low_precision_alarm(1, [&](){
		general_state_machine.check_transitions();
	});
	
//-------FDCAN------//
	fdcan = FDCAN::inscribe<
			CANBitRatesSpeed::CAN_1_Mbit,    
			CANFormat::CAN_FDCAN_FORMAT,                
			CANIdentifier::CAN_29_BIT_IDENTIFIER,      
			CANMode::CAN_MODE_NORMAL              
		>(FDCAN::fdcan1);

//-------RESETS------//
	resets[VCU] = DigitalOutput(PA12);
	resets[HVSCU] = DigitalOutput(PG3);
	resets[BCU] = DigitalOutput(PD11);
	resets[BMSL] = DigitalOutput(PD9);
	resets[LCU] = DigitalOutput(PB12);
	resets[PCU] = DigitalOutput(PG1);

//-------BOOTS------//
	boots[VCU] = DigitalOutput(PA11);
	boots[HVSCU] = DigitalOutput(PG2);
	boots[BCU] = DigitalOutput(PD10);
	boots[BMSL] = DigitalOutput(PD8);
	boots[LCU] = DigitalOutput(PE15);
	boots[PCU] = DigitalOutput(PE7);

	

//-------LEDS------//
	LED_OPERATIONAL = DigitalOutput(PG8);
	LED_FAULT = DigitalOutput(PG7);
	LED_CAN = DigitalOutput(PG6);
	LED_FLASH = DigitalOutput(PG5);
	LED_SLEEP = DigitalOutput(PG4);

}

void BLCU::init(){
	STLIB::start();//TODO

	for (auto& [target, reset_pin]: BLCU::resets){
		reset_pin.turn_on();
    }

	for (auto& [target, boot_pin]: BLCU::boots){
		boot_pin.turn_off();
    }
}

void BLCU::setup_state_machine(){
        general_state_machine = {GeneralStates::INITIAL};
		general_state_machine.add_state(GeneralStates::OPERATIONAL);
		general_state_machine.add_state(GeneralStates::FAULT);

		ProtectionManager::link_state_machine(general_state_machine, GeneralStates::FAULT);
		ProtectionManager::set_id(Boards::ID::BLCU_VEHICLE);

		//Enter actions
		general_state_machine.add_enter_action([&](){
			Time::set_timeout(max_tcp_connection_timeout, [&](){
				if(not (tcp_socket->state == ServerSocket::ServerState::ACCEPTED )){
							tcp_timeout = true;
				}
			});
		}, GeneralStates::INITIAL);

		general_state_machine.add_enter_action([&](){
			LED_FAULT.turn_on();
		}, GeneralStates::FAULT);

		general_state_machine.add_enter_action([&](){
			LED_OPERATIONAL.turn_on();
		}, GeneralStates::OPERATIONAL);
		BLCU::
		//Exit actions
		general_state_machine.add_exit_action([&](){
			LED_OPERATIONAL.turn_off();
		}, GeneralStates::INITIAL);

		general_state_machine.add_exit_action([&](){
			LED_FAULT.turn_off();
		}, GeneralStates::FAULT);

		general_state_machine.add_exit_action([&](){
			LED_OPERATIONAL.turn_off();
		}, GeneralStates::OPERATIONAL);

		//Transitions
		general_state_machine.add_transition(GeneralStates::INITIAL, GeneralStates::OPERATIONAL, [&](){
			return tcp_socket->state == ServerSocket::ServerState::ACCEPTED;
		});
		general_state_machine.add_transition(GeneralStates::INITIAL, GeneralStates::FAULT, [&](){
			if(tcp_timeout) ErrorHandler("TCP connections could not be established in time and timed out");
			return tcp_timeout;
		});
		general_state_machine.add_transition(GeneralStates::OPERATIONAL, GeneralStates::FAULT, [&](){
			if(tcp_socket->state!= ServerSocket::ServerState::ACCEPTED){
				ErrorHandler("TCP connections fell");
				return true;
			}
			return false;
		});

		//Cyclic actions
		general_state_machine.add_low_precision_cyclic_action([&](){LED_OPERATIONAL.toggle();}, (chrono::milliseconds)150, INITIAL);

		// general_state_machine.add_low_precision_cyclic_action(ProtectionManager::check_protections, (chrono::milliseconds)1 , OPERATIONAL);
}

void BLCU::setup_specific_state_machine(){
    specific_state_machine = StateMachine(SpecificStates::READY);
    specific_state_machine.add_state(SpecificStates::BOOTING);


    //Enter actions
    specific_state_machine.add_enter_action([&](){
        // boots[current_target].turn_off();
        // resets[current_target].turn_off();
		stop_booting();
        turn_on_all_boards();
        LED_OPERATIONAL.turn_on();

    }, SpecificStates::READY);

    specific_state_machine.add_enter_action([&](){
        turn_off_all_boards();
        send_to_bootmode();
    }, SpecificStates::BOOTING);

    //Exit actions
    specific_state_machine.add_exit_action([&](){
        LED_OPERATIONAL.turn_off();
    }, SpecificStates::READY);

    specific_state_machine.add_exit_action([&](){
        LED_CAN.turn_off();
    }, SpecificStates::BOOTING);

    //Cyclic actions
    general_state_machine.add_low_precision_cyclic_action([&](){LED_CAN.toggle();}, (chrono::milliseconds)75, SpecificStates::BOOTING);

}

void BLCU::finish_write_read_order(bool error_ok){
	BTFTP::off();

		if(not error_ok){
			BLCU::tcp_socket->send_order(nack);
			__abort_booting();
			return;
		}else{
			BLCU::tcp_socket->send_order(ack);
		}

		BLCU::specific_state_machine.force_change_state(SpecificStates::READY);
}