#include "CRC8.h"

namespace romiserial {

        uint8_t CRC8::table_[256];
        bool CRC8::table_initialized_ = false;

        CRC8::CRC8() : crc_(0)
        {
                init_table(0x07);
        }

        void CRC8::init_table(uint8_t poly)
        {
                if (!table_initialized_) {
                        do_init_table(poly);
                        table_initialized_ = true;
                }
        }
        
        void CRC8::do_init_table(uint8_t poly)
        {
                uint8_t crc;                
                for (uint16_t i = 0; i <= UINT8_MAX; i++) {
                        crc = (uint8_t) i;
                        for (uint8_t j = 0; j < 8; j++)
                                crc = (uint8_t) ((uint8_t)(crc << 1) ^ (uint8_t) ((crc & (uint8_t) 0x80) ? poly : 0));
                        table_[i] = crc;
                }
        }

}
