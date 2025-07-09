#ifndef SIM_ON
#include "main.h"

#include "lwip.h"
#endif

#include "ST-LIB.hpp"
#include "BLCU/BLCU.hpp"


HeapOrder ack = {
    3,
};


HeapOrder nack = {
    4,
};

// HeapOrder get_version_order = {
//     703,
//     BLCU::get_version,
//     &BLCU::orders_data.target,
//     &BLCU::orders_data.version,
// };


int main(void) {
#ifdef SIM_ON
    SharedMemory::start();
#endif

    HeapOrder write_program_order = {
        700,
        BLCU::write_program,
        &BLCU::orders_data.target,
    };

    // HeapOrder read_program_order = {
    //     701,
    //     BLCU::read_program,
    //     &BLCU::orders_data.target,
    // };

    // HeapOrder erase_program_order = {
    //     702,
    //     BLCU::erase_program,
    //     &BLCU::orders_data.target,
    // };

    

    HeapOrder reset_all_order = {
        704,
        BLCU::reset_all,
        &BLCU::orders_data.target,
    };
    DigitalOutput BootPCU(PE7);
    DigitalOutput NReset(PG1);

    BLCU::set_up();
    BLCU::start();
    // RCC_ClocksTypeDef RCC_Clocks;
    // HAL_RCC_GetClockConfig()

    // BootPCU.turn_off();
    // HAL_Delay(100);
    // NReset.turn_on();
    // BLCU::orders_data.target = BLCU::Target::PCU;
    //  BLCU::write_program();

    
    
    // BootPCU.turn_on();
    // NReset.turn_off();
    // HAL_Delay(100);
    // NReset.turn_on();
    // HAL_Delay(100);
    // BootPCU.turn_off();


    while (1) {
    BLCU::update();
    }
}

void Error_Handler(void) {
    ErrorHandler("HAL error handler triggered");
    while (1) {
    }
}
