#ifndef SIM_ON
#include "main.h"

#include "lwip.h"
#endif

#include "ST-LIB.hpp"
#include "BLCU/BLCU.hpp"


int main(void) {
#ifdef SIM_ON
    SharedMemory::start();
#endif



  

    BLCU board;
    board.init();
    

    while (1) {
    board.update();
    }
}

void Error_Handler(void) {
    ErrorHandler("HAL error handler triggered");
    while (1) {
    }
}
