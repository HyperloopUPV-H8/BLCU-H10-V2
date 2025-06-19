#pragma once

#include "BLCU/BLCU_Definition/BLCU_Definition.hpp"

namespace BLCU{

	void __set_up_peripherals(){
		BLCU::__set_up_fdcan();
		BLCU::__set_up_ethernet();
		//BLCU::__set_up_resets();
		//BLCU::__set_up_boots();
		//BLCU::__set_up_leds();
	}

	void __set_up_fdcan(){
		BLCU::fdcan = FDCAN::inscribe<
			CANBitRatesSpeed::CAN_1_Mbit,    
			CANFormat::CAN_FDCAN_FORMAT,                
			CANIdentifier::CAN_29_BIT_IDENTIFIER,      
			CANMode::CAN_MODE_NORMAL              
		>(FDCAN::fdcan1);
	}


   void __set_up_ethernet(){
		BLCU::ip = BLCU_IP;
		BLCU::mask = BLCU_MASK;
		BLCU::gateway = BLCU_GATEWAY;
		BLCU::port = BLCU_PORT;
   }

   void __set_up_resets(){
		BLCU::resets[VCU] = DigitalOutput(PA12);
		BLCU::resets[HVSCU] = DigitalOutput(PG3);
		BLCU::resets[BCU] = DigitalOutput(PD11);
		BLCU::resets[BMSL] = DigitalOutput(PD9);
		BLCU::resets[LCU] = DigitalOutput(PB12);
		BLCU::resets[PCU] = DigitalOutput(PG1);
   }

   void __set_up_boots(){
		BLCU::boots[VCU] = DigitalOutput(PA11);
		BLCU::boots[HVSCU] = DigitalOutput(PG2);
		BLCU::boots[BCU] = DigitalOutput(PD10);
		BLCU::boots[BMSL] = DigitalOutput(PD8);
		BLCU::boots[LCU] = DigitalOutput(PE15);
		BLCU::boots[PCU] = DigitalOutput(PE7);
   }

   void __set_up_leds(){
		BLCU::LED_OPERATIONAL = DigitalOutput(PG8);
		BLCU::LED_FAULT = DigitalOutput(PG7);
		BLCU::LED_CAN = DigitalOutput(PG6);
		BLCU::LED_FLASH = DigitalOutput(PG5);
		BLCU::LED_SLEEP = DigitalOutput(PG4);
   }

}
