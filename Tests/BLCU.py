import sys, os
sys.path.append(os.path.join(os.path.dirname(__file__), "VirtualMCU", "src"))


import vmcu.assertions as assertions
from vmcu.services.communications.FDCAN import FDCAN,Packet
from vmcu.pin.pinout import Pinout
from vmcu.shared_memory import SharedMemory
from enum import Enum

class Bootloader:
    class fdcan:
        FDCAN_MAX_QUEUE_SIZE = 16
        
        class can_ids(Enum):
            Bootloader_ACK = 0x79
            Bootloader_NACK = 0x1f
            
        class fdcan_error(Enum):
            FDCAN_OK = 0x00
            FDCAN_EMPTY = 0x01
            FDCAN_ERROR = 0xff
        
                
        def __init__(self,can_ip, can_sendport,shm:SharedMemory):
            self.shm = shm
            self._fdcan = FDCAN(self.shm,Pinout.PB11, Pinout.PB10)
            self._fdcan.start(can_ip, can_sendport)
        
        def fdcan_transmit(self,Packet:"Packet")->fdcan_error:
            if self._fdcan.transmit(Packet.message_id, Packet.data, Packet.data_length) == False:
                return self.fdcan_error.FDCAN_ERROR
            return self.fdcan_error.FDCAN_OK
        
        def fdcan_read(self)->fdcan_error:
            packet = self._fdcan.read()
            if packet == None:
                return self.fdcan_error.FDCAN_EMPTY
            return packet,self.fdcan_error.FDCAN_OK
    
    BOOTLOADER_MAX_TIMEOUT = 2000
    FLASH_SECTOR_0= 0x08000000
    FLASH_MAX_SECTOR= 0x080FFFFF
    FLASH_SECTOR1_START_ADDRESS =0x08020000
    FLASH_SECTOR2_START_ADDRESS =0x08040000
    FLASH_SECTOR3_START_ADDRESS =0x08060000
    FLASH_SECTOR4_START_ADDRESS = 0x08080000
    FLASH_SECTOR5_START_ADDRESS = 0x080A0000
    FLASH_SECTOR6_START_ADDRESS = 0x080C0000
    FLASH_SECTOR_ERROR = 0xFFFFFFFF
    
    def flash_get_sector_starting_address(self,sector:int)->int:
        address = self.FLASH_SECTOR_ERROR
        #Esta mierda no se cuanto vale: case FLASH_SECTOR_0
        
    class bootloader_order_t(Enum):
        GET_VERSION_ORDER = 0x50
        READ_MEMORY_ORDER = 0x40
        WRITE_MEMORY_ORDER = 0x30
        ERASE_MEMORY_ORDER = 0x20
        GO_ORDER = 0x10
        
    class bootloader_error_t(Enum):
        BOOTLOADER_OK = 1
        BOOTLOADER_ERROR = 0
    
    def __init__(self,ip,send_port):
        self.shm = SharedMemory("gpio_BLCU","state_machine_BLCU")
        self.fdcan = self.fdcan(ip,send_port,self.shm)
        
    def __b_wait_until_fdcan_message_received(self):
        while self.fdcan.fdcan_read() != self.fdcan.fdcan_error.FDCAN_OK:
            pass
        return self.bootloader_error_t.BOOTLOADER_OK 

    def __b_clean_fdcan_packet_data(self,packet:"Packet"):
        aux_packet = packet
        for i in range(64):
            aux_packet.data[i] = 0   
        return aux_packet 
             
    def __b_clean_fdcan_packet_all(self,packet:"Packet"):
        aux_packet = packet
        self.__b_clean_fdcan_packet_data(aux_packet)
        aux_packet.message_id = 0  
        return Packet 
    
    def __b_get_addr_from_data(self,data:list[bytes])->int:
        address = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3]
        return address
    
    def __b_send_nack(self,packet:"Packet"):
        aux_packet = packet
        self.__b_clean_fdcan_packet_data(aux_packet)
        aux_packet.data[0] = self.fdcan.can_ids.Bootloader_NACK
        self.fdcan.fdcan_transmit(aux_packet)
    
    def __b_send_ack(self,packet:"Packet"):
        aux_packet = packet
        self.__b_clean_fdcan_packet_data(aux_packet)
        aux_packet.data[0] = self.fdcan.can_ids.Bootloader_ACK
        self.fdcan.fdcan_transmit(aux_packet)
    
    def __b_wait_for_ack(self,order:bootloader_order_t)->bootloader_error_t:
        if self.__b_wait_until_fdcan_message_received() != self.bootloader_error_t.BOOTLOADER_OK:
            return self.bootloader_error_t.BOOTLOADER_ERROR
        
        aux_packet = self.fdcan.fdcan_read()
        
        if aux_packet.identifier != order:
            return self.bootloader_error_t.BOOTLOADER_ERROR
        
        if aux_packet.data[0] != self.fdcan.can_ids.Bootloader_ACK:
            return self.bootloader_error_t.BOOTLOADER_ERROR
        
        return self.bootloader_error_t.BOOTLOADER_OK
    
    def __b_copy_data_from_packet(self,packet:"Packet")->list[bytes]:
        data = []
        for i in range(64):
            data.append(packet.data[i])
        return data
    
    def __b_data_copy_to_packet(self,data:list[bytes])->"Packet":
        for i in range(64):
            self.Packet.data[i] = data[i]
        return self.Packet
    
    def __b_go(self,packet:"Packet"):
        self.__b_send_nack(packet)
        
    def __b_erase_memory(self,packet:"Packet"):
        sector1 = packet.data[0]
        sector2 = packet.data[1]
        if (sector1 > sector2 or sector1 < self.FLASH_SECTOR_0 or sector2 > self.FLASH_MAX_SECTOR):
            self.__b_send_nack()
            return
        self.__b_send_ack(packet)
        self.__b_send_ack(packet)