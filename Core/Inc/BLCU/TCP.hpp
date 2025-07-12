#include "BLCU.hpp"
#include "ST-LIB.hpp"

    class TCP{
        private:

        BLCU* board;
        static HeapOrder* write_program_order;
        static HeapOrder*  reset_all_order;
        // static HeapOrder* get_version_order;
        // static HeapOrder* read_program_order;
        // static HeapOrder* erase_program_order;

        public:
        TCP(BLCU* board);

        static ServerSocket* tcp_socket;
        
        static HeapOrder ack = {
            3,
        };
        
        static HeapOrder nack = {
            4,
        };

    };
    



    