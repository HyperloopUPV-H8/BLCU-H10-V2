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
        
        def fdcan_read(self):
            packet = self._fdcan.read()
            if packet == None:
                return None,self.fdcan_error.FDCAN_EMPTY
            return packet,self.fdcan_error.FDCAN_OK
    
    class flash:
        FLASH_SECTOR_0 = 0  
        FLASH_SECTOR_1 = 1  
        FLASH_SECTOR_2 = 2  
        FLASH_SECTOR_3 = 3  
        FLASH_SECTOR_4 = 4  
        FLASH_SECTOR_5 = 5  
        FLASH_SECTOR_6 = 6  
        FLASH_SECTOR_7 = 7  
        
        FLASH_WORD_SIZE = 32
        FLASH_32BITS_WORLD = 4 #IDK if this is intentionall or a mistake, but it says world on the original script

        SECTOR_SIZE_IN_32BITS_WORDS = 32768
        SECTOR_SIZE_IN_BYTES = 131072
        SECTOR_SIZE_IN_KILOBYTES = 128

        FLASH_SECTOR0_START_ADDRESS = 0x08000000
        FLASH_SECTOR0_END_ADDRESS   = 0x0801FFFF

        FLASH_SECTOR1_START_ADDRESS = 0x08020000
        FLASH_SECTOR1_END_ADDRESS   = 0x0803FFFF

        FLASH_SECTOR2_START_ADDRESS = 0x08040000
        FLASH_SECTOR2_END_ADDRESS   = 0x0805FFFF

        FLASH_SECTOR3_START_ADDRESS = 0x08060000
        FLASH_SECTOR3_END_ADDRESS   = 0x0807FFFF

        FLASH_SECTOR4_START_ADDRESS = 0x08080000
        FLASH_SECTOR4_END_ADDRESS   = 0x0809FFFF

        FLASH_SECTOR5_START_ADDRESS = 0x080A0000
        FLASH_SECTOR5_END_ADDRESS   = 0x080BFFFF

        FLASH_SECTOR6_START_ADDRESS = 0x080C0000
        FLASH_SECTOR6_END_ADDRESS   = 0x080DFFFF

        FLASH_SECTOR7_START_ADDRESS = 0x080E0000
        FLASH_SECTOR7_END_ADDRESS   = 0x080FFFFF

        FLASH_SECTOR_ERROR = 0xFFFFFFFF

        FLASH_START_ADDRESS = FLASH_SECTOR0_START_ADDRESS
        FLASH_END_ADDRESS = FLASH_SECTOR7_END_ADDRESS
        FLASH_CODE_END_ADDRESS = FLASH_SECTOR5_END_ADDRESS

        FLASH_PROTECTED_SECTOR1 = FLASH_SECTOR6_START_ADDRESS
        FLASH_PROTECTED_SECTOR2 = FLASH_SECTOR7_START_ADDRESS
        FLASH_MAX_SECTOR = FLASH_PROTECTED_SECTOR1 - 1

        class FlashError(Enum):
            FLASH_OK = 0x00
            FLASH_PROTECTED_MEM = 0x01
            FLASH_ERROR = 0xff
       
        BOOTLOADER_MAX_TIMEOUT = 2000
        
        def __init__(self):
            total_bytes = self.FLASH_END_ADDRESS - self.FLASH_START_ADDRESS + 1
            self.memory = bytearray(total_bytes)
            
        def get_index(self,address:int)->int:
            index = address - self.FLASH_START_ADDRESS
            return index
        
        def get_address(self,index:int)->int:
            address = self.FLASH_START_ADDRESS + index
            return address
        
        
        def flash_get_sector_starting_address(self,sector:int)->int:
            address = self.FLASH_SECTOR_ERROR
            if sector == self.FLASH_SECTOR_0:
                address = self.FLASH_SECTOR0_START_ADDRESS
            elif sector == self.FLASH_SECTOR_1:
                address = self.FLASH_SECTOR1_START_ADDRESS
            elif sector == self.FLASH_SECTOR_2:
                address = self.FLASH_SECTOR2_START_ADDRESS
            elif sector == self.FLASH_SECTOR_3:
                address = self.FLASH_SECTOR3_START_ADDRESS
            elif sector == self.FLASH_SECTOR_4:
                address = self.FLASH_SECTOR4_START_ADDRESS
            elif sector == self.FLASH_SECTOR_5:
                address = self.FLASH_SECTOR5_START_ADDRESS
            elif sector == self.FLASH_SECTOR_6:
                address = self.FLASH_SECTOR6_START_ADDRESS
            elif sector == self.FLASH_SECTOR_7:
                address = self.FLASH_SECTOR7_START_ADDRESS
            else:
                address = self.FLASH_SECTOR_ERROR
            return address
        
        def __flash_get_sector(self,address:int)->int:
            sector = self.FLASH_SECTOR_0
            if address >= 0x08000000 and address < 0x08020000:
                sector = self.FLASH_SECTOR_0
            elif address >= 0x08020000 and address < 0x08040000:
                sector = self.FLASH_SECTOR_1
            elif address >= 0x08040000 and address < 0x08060000:
                sector = self.FLASH_SECTOR_2
            elif address >= 0x08060000 and address < 0x08080000:
                sector = self.FLASH_SECTOR_3
            elif address >= 0x08080000 and address < 0x080A0000:
                sector = self.FLASH_SECTOR_4
            elif address >= 0x080A0000 and address < 0x080C0000:
                sector = self.FLASH_SECTOR_5
            elif address >= 0x080C0000 and address < 0x080E0000:
                sector = self.FLASH_SECTOR_6
            elif address >= 0x080E0000 and address < 0x08100000:
                sector = self.FLASH_SECTOR_7
            return sector
        
        def flash_erease(self,start_sector:int,end_sector:int)->FlashError:
            if start_sector < 0 or end_sector > self.FLASH_MAX_SECTOR:
                return self.FlashError.FLASH_ERROR
            for i in range(start_sector, end_sector + 1):
                address = self.flash_get_sector_starting_address(i)
                address = self.get_index(address)
                for j in range(self.SECTOR_SIZE_IN_BYTES):
                    self.memory[address + j] = 0xFF
            return self.FlashError.FLASH_OK
        
        def flash_write(self,dest_addr:int,data:list,number_of_words:int)->FlashError:
            if dest_addr < self.FLASH_SECTOR0_START_ADDRESS or dest_addr > self.FLASH_CODE_END_ADDRESS:
                return self.FlashError.FLASH_PROTECTED_MEM
            
            index = 0
            start_sector = self.__flash_get_sector(dest_addr)
            start_sector_addr = self.flash_get_sector_starting_address(start_sector)
            end_address = dest_addr + ((number_of_words * self.FLASH_32BITS_WORLD) - self.FLASH_32BITS_WORLD)
            end_sector = self.__flash_get_sector(end_address)
            
            if (self.flash_erase(start_sector, end_sector) != self.FLASH_OK):
                return self.FlashError.FLASH_ERROR
            
            while index < self.SECTOR_SIZE_IN_32BITS_WORDS:
                if (start_sector_addr > self.FLASH_CODE_END_ADDRESS):
                    return self.FlashError.FLASH_PROTECTED_MEM
                address = self.get_index(start_sector_addr) #We are adding 32 bytes to the flash for each while call
                aux_index = index
                for i in range(32):
                    self.memory[address + i] = data[aux_index]
                    aux_index += 1
                start_sector_addr += self.FLASH_WORD_SIZE
                index += self.FLASH_WORD_SIZE / self.FLASH_32BITS_WORLD
                
            return self.FlashError.FLASH_OK
        
        def flash_read(self, source_addr: int, number_of_words: int):
            if source_addr < self.FLASH_START_ADDRESS or source_addr > self.FLASH_CODE_END_ADDRESS:
                return None, self.FlashError.FLASH_PROTECTED_MEM
            result = []
            for i in range(number_of_words):
                index = self.get_index(source_addr)
                word_bytes = self.memory[index:index+4]
                word_int = int.from_bytes(word_bytes, byteorder='big')
                result.append(word_int)
                source_addr += 4  
            return result, self.FlashError.FLASH_OK
                
            
    class bootloader_order_t(Enum):
        GET_VERSION_ORDER = 0x50
        READ_MEMORY_ORDER = 0x40
        WRITE_MEMORY_ORDER = 0x30
        ERASE_MEMORY_ORDER = 0x20
        GO_ORDER = 0x10
        
    class bootloader_error_t(Enum):
        BOOTLOADER_OK = 1
        BOOTLOADER_ERROR = 0
        
    BOOTLOADER_BLOCK_SIZE = 8
    BOOTLOADER_VERSION = 0x05
    
    def __init__(self,ip,send_port):
        self.shm = SharedMemory("gpio_BLCU","state_machine_BLCU")
        self.fdcan = self.fdcan(ip,send_port,self.shm)
        self.flash = self.flash()
        
    def __b_wait_until_fdcan_message_received(self,packet:"Packet"):
        packet,result=self.fdcan.fdcan_read() 
        while result != self.fdcan.fdcan_error.FDCAN_OK:
            packet,result=self.fdcan.fdcan_read() 
        return packet,self.bootloader_error_t.BOOTLOADER_OK 

    def __b_clean_fdcan_packet_data(self,packet:"Packet"):
        aux_packet = packet
        for i in range(64):
            aux_packet.data[i] = 0   
        return aux_packet 
             
    def __b_clean_fdcan_packet_all(self,packet:"Packet"):
        aux_packet = packet
        self.__b_clean_fdcan_packet_data(aux_packet)
        aux_packet.message_id = 0  
        return aux_packet 
    
    def __b_get_addr_from_data(self,data:list[bytes])->int:
        address = data[0] << 24 | data[1] << 16 | data[2] << 8 | data[3]
        return address
    
    def __b_send_nack(self,packet:"Packet"):
        self.__b_clean_fdcan_packet_data(packet)
        packet.data[0] = self.fdcan.can_ids.Bootloader_NACK
        self.fdcan.fdcan_transmit(packet)
        return packet
    
    def __b_send_ack(self,packet:"Packet"):
        aux_packet = packet
        self.__b_clean_fdcan_packet_data(aux_packet)
        aux_packet.data[0] = self.fdcan.can_ids.Bootloader_ACK
        self.fdcan.fdcan_transmit(aux_packet)
        return aux_packet
    
    def __b_wait_for_ack(self,order:bootloader_order_t)->bootloader_error_t:
        aux_packet,result=self.__b_wait_until_fdcan_message_received() 
        if result != self.bootloader_error_t.BOOTLOADER_OK:
            return self.bootloader_error_t.BOOTLOADER_ERROR
        
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
            Packet.data[i] = data[i]
        return Packet
    
    def __b_go(self,packet:"Packet"):
        packet =self.__b_send_nack(packet)
        return packet
        
    def __b_erase_memory(self,packet:"Packet")->"Packet":
        sector1 = packet.data[0]
        sector2 = packet.data[1]
        if (sector1 > sector2 or sector1 < 1 or sector2 > 7):
            packet =self.__b_send_nack()
            return
        packet =self.__b_send_ack(packet)
        
        if self.flash.flash_erase(sector1,sector2) != self.flash.FlashError.FLASH_OK:
            packet =self.__b_send_nack(packet)
            return
        
        self.__b_send_ack(packet)
        return packet
        
    def __b_write_memory(self,packet:"Packet"):
        buffer = [self.flash.SECTOR_SIZE_IN_32BITS_WORDS]
        sector= packet.data[0]
        address = self.flash.flash_get_sector_starting_address(sector)
        i=0
        counter = 1
        
        if(address==self.flash.FLASH_SECTOR_ERROR or sector>6):
            packet =self.__b_send_nack(packet)
            return
        
        packet =self.__b_send_ack(packet)
        
        if(self.__b_wait_for_ack(packet.identifier) != self.bootloader_error_t.BOOTLOADER_OK):
            packet =self.__b_send_nack(packet)
            return
        
        for i in range(0, self.flash.SECTOR_SIZE_IN_32BITS_WORDS, 16):
            packet,result = self.__b_wait_until_fdcan_message_received()
            
            if  result != self.fdcan.fdcan_error.FDCAN_OK:
                packet =self.__b_send_nack(packet)
                return
            
            if packet.identifier != self.bootloader_order_t.WRITE_MEMORY_ORDER:
                packet =self.__b_send_nack(packet)
                return
            data = self.__b_copy_data_from_packet(packet)
            buffer[i:i+len(data)] = data
            
            if counter >= self.BOOTLOADER_BLOCK_SIZE:
                packet =self.__b_send_ack(packet)
                if self.__b_wait_for_ack(packet.identifier) != self.bootloader_error_t.BOOTLOADER_OK:
                    packet =self.__b_send_nack(packet)
                    return
                counter = 1
            else:
                counter += 1
        self.flash.flash_write(address, buffer, self.SECTOR_SIZE_IN_32BITS_WORDS)
            
        packet =self.__b_send_ack(packet)
        return packet
    
    def __b_read_memory(self,packet):
        buffer=[self.flash.SECTOR_SIZE_IN_32BITS_WORDS]
        sector = packet.data[0]
        address = self.flash.flash_get_sector_starting_address(sector)
        i=0
        counter = 0
        if address == self.flash.FLASH_SECTOR_ERROR or sector >= self.flash.FLASH_PROTECTED_SECTOR2:
            packet =self.__b_send_nack(packet)
            return

        buffer, status = self.flash.flash_read(address, self.flash.SECTOR_SIZE_IN_32BITS_WORDS)
        if status != self.flash.FlashError.FLASH_OK:
            packet =self.__b_send_nack(packet)
            return

        packet =self.__b_send_ack(packet)

        if self.__b_wait_for_ack(packet.identifier) != self.bootloader_error_t.BOOTLOADER_OK:
            packet =self.__b_send_nack(packet)
            return
        
        for i in range(0, self.flash.SECTOR_SIZE_IN_32BITS_WORDS, 16):
            packet=self.__b_data_copy_to_packet(buffer)
            if (self.fdcan.fdcan_transmit(packet) != self.fdcan.fdcan_error.FDCAN_OK):
                packet =self.__b_send_nack(packet)
                return
            if counter >= self.BOOTLOADER_BLOCK_SIZE:
                packet =self.__b_send_ack(packet)
                if self.__b_wait_for_ack(packet.identifier) != self.bootloader_error_t.BOOTLOADER_OK:
                    packet =self.__b_send_nack(packet)
                    return
                counter = 0
            counter += 1
        
        packet =self.__b_send_ack(packet)
        return packet
    
    def __b_get_version(self,packet:"Packet"):
        packet =self.__b_send_ack(packet)
        packet.data[0] = self.BOOTLOADER_VERSION
        if self.fdcan.fdcan_transmit(packet) != self.fdcan.fdcan_error.FDCAN_OK:
            packet =self.__b_send_nack(packet)
            return
        packet =self.__b_send_ack(packet)
        return packet
    
    def bootloader_start(self):
        packet = Packet()  
        packet.data_length = FDCAN.DLC.DLC_64
        packet = self.__b_clean_fdcan_packet_all(packet)
        while True:
            packet,result = self.__b_wait_until_fdcan_message_received()
            
            if  result != self.bootloader_error_t.BOOTLOADER_OK:
                    continue            

            if packet.identifier == self.bootloader_order_t.GET_VERSION_ORDER:
                packet = self.__b_get_version(packet)
            elif packet.identifier == self.bootloader_order_t.READ_MEMORY_ORDER:
                packet = self.__b_read_memory(packet)
            elif packet.identifier == self.bootloader_order_t.WRITE_MEMORY_ORDER:
                packet = self.__b_write_memory(packet)
            elif packet.identifier == self.bootloader_order_t.ERASE_MEMORY_ORDER:
                packet = self.__b_erase_memory(packet)
            elif packet.identifier == self.bootloader_order_t.GO_ORDER:
                packet = self.__b_go(packet)
            else:
                packet = self.__b_send_nack(packet)

            packet = self.__b_clean_fdcan_packet_all(packet)
