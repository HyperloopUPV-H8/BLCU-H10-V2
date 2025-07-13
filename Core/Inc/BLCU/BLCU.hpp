/**
 * BLCU.h
 *
 *  Created on: Feb 3, 2023
 *      Author: Pablo
 */

#pragma once

#include "ST-LIB.hpp"
#include "FDCBootloader/FDCBootloader.hpp"
#include "FDCBootloader/BootloaderTFTP.hpp"
#include "Comms.hpp"

#define BLCU_ID         ((uint8_t)1)

    enum Target{
		NOTARGET,
		VCU,			//		    1
		HVSCU,			//			2
		BCU,			//			3
		BMSL,			//			4
		LCU,			//			5
		PCU,			//          6
		
	};
    class BLCU{
        private:
    
        static constexpr uint16_t max_tcp_connection_timeout = 30000;
    
        unordered_map<Target, DigitalOutput> resets; 
        unordered_map<Target, DigitalOutput> boots;

        uint8_t fdcan;
        DigitalOutput LED_OPERATIONAL;
        DigitalOutput LED_FAULT;
        DigitalOutput LED_CAN;
        DigitalOutput LED_FLASH;
        DigitalOutput LED_SLEEP;

        
        static ServerSocket* tcp_socket;//Hacer nullptr
        bool tcp_timeout = false;

        bool programming_error = false;

        void finish_write_read_order(bool error_ok);
        void turn_off_all_boards();
        void turn_on_all_boards();
        void send_to_bootmode();
        void stop_booting();


        void setup_state_machine();
        void setup_specific_state_machine();

        Comms* tcp =nullptr;

        public:

        enum GeneralStates{
            INITIAL,
            OPERATIONAL,
            FAULT,
        };
    
    
        enum SpecificStates{
            READY,
            BOOTING,
        };

        BLCU();
        void init();
        void update();

        void abort_booting();
        void reset_all();

        //PUBLIC VARIABLES:
        Target current_target;
        
        StateMachine general_state_machine;
        StateMachine specific_state_machine;

        static constexpr string ip{"192.168.0.27"};
        static constexpr string mask{"255.255.0.0"}; 
        static constexpr string gateway{"192.168.1.1"};
        static constexpr uint32_t port{50500};
        

    };

