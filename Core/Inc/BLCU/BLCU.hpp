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

// Macro for BLCU id
#define BLCU_ID ((uint8_t)1)

enum Target {
    NOTARGET,
    VCU,    // 1
    HVSCU,  // 2
    BCU,    // 3
    BMSL,   // 4
    LCU,    // 5
    PCU     // 6
};

class BLCU {
private:
    BLCU() = delete;

    static constexpr uint16_t max_tcp_connection_timeout = 30000;

    static unordered_map<Target, DigitalOutput> resets;
    static unordered_map<Target, DigitalOutput> boots;

    static uint8_t fdcan;
    static DigitalOutput LED_OPERATIONAL;
    static DigitalOutput LED_FAULT;
    static DigitalOutput LED_CAN;
    static DigitalOutput LED_FLASH;
    static DigitalOutput LED_SLEEP;

    static bool tcp_timeout;
    static bool programming_error;


    static void finish_write_read_order(bool error_ok);
    static void turn_off_all_boards();
    static void turn_on_all_boards();
    static void send_to_bootmode();
    static void stop_booting();

    static void setup_state_machine();
    static void setup_specific_state_machine();

public:
    enum GeneralStates {
        INITIAL,
        OPERATIONAL,
        FAULT,
    };

    enum SpecificStates {
        READY,
        BOOTING,
    };

    static void init();
    static void update();
    static void abort_booting();
    static void reset_all();

    static Target current_target;
    static bool ready_flag;
    static StateMachine general_state_machine;
    static StateMachine specific_state_machine;

    static constexpr string ip{"192.168.0.27"};
    static constexpr string mask{"255.255.0.0"};
    static constexpr string gateway{"192.168.1.1"};
    static constexpr uint32_t port{50500};
};


