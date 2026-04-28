#ifndef NETRONIX_FRAME_H_
#define NETRONIX_FRAME_H_

#include <array>
#include <cctype>
#include <cstddef>

namespace Netronix {

template <std::size_t kMaxDataLen>
class NetronixSerdes {
public:
    struct Frame {
        uint8_t address { 0 };
        uint8_t length { 0 };
        uint8_t command { 0 };

        std::array<uint8_t, kMaxDataLen> payload {};
        std::size_t payload_len { 0 };

        uint8_t op_code { 0 };

        Frame(uint8_t addr, 
            uint8_t cmd,
            uint8_t pload_len,
            const std::array<uint8_t, kMaxDataLen>& pload = {})
            : address { addr }, command { cmd }, payload { pload },  payload_len { pload_len } {}

        Frame(uint8_t addr, 
            uint8_t cmd,
            uint8_t pload_len,
            const uint8_t* data)
            : address { addr }, command { cmd }, payload_len { pload_len } 
            {
                if (data != nullptr) {

                    for (std::size_t i = 0; i < payload_len; ++i) {
                        payload[i] = data[i];
                    }

                }
            }

        Frame() {}
        };

    static constexpr std::size_t kMinFrameSize = 5;
    static constexpr std::size_t kHeaderSize = 3;
    static constexpr std::size_t kCrcSize = 2;
    static constexpr std::size_t kOpCodeSize = 1;
    static constexpr std::size_t kMinResponseLen = kHeaderSize + kOpCodeSize + kCrcSize;


    static size_t Serialize(
        uint8_t* out_buffer,
        std::size_t out_buffer_size,
        const Frame& frame) noexcept 
    {
        if (out_buffer == nullptr ||
            frame.payload_len > kMaxDataLen) 
        {
            return 0;
        }

        /*
            Request frame:
            ADDRESS + LENGTH + COMMAND + PAYLOAD + CRC16

            LENGTH field:
            COMMAND + PAYLOAD
        */

        const std::size_t frame_len = 
            kHeaderSize +
            frame.payload_len +
            kCrcSize;

        if (out_buffer_size < frame_len) 
        {
            return 0;
        }

        std::size_t write_index = 0;
        
        out_buffer[write_index++] = frame.address;

        const uint8_t length = frame_len;

        out_buffer[write_index++] = length;

        out_buffer[write_index++] = frame.command;

        for(std::size_t i = 0; i < frame.payload_len; i++)
        {
            out_buffer[write_index++] = frame.payload[i];
        }

        const uint16_t crc = Crc16(out_buffer, write_index);
    
        out_buffer[write_index++] =
            static_cast<uint8_t>((crc >> 8) & 0xFF);
    
        out_buffer[write_index++] =
            static_cast<uint8_t>(crc & 0xFF);

        return write_index;
    }

    static bool Deserialize(
        const uint8_t* data,
        std::size_t data_len,
        Frame& frame) noexcept
    {
        if(data == nullptr || 
            data_len < kMinFrameSize ||
            !Validate(data, data_len) ||
            data_len < kMinResponseLen)
        {
            return false;
        }

        frame.address = data[0];
        frame.length = data[1];
        frame.command = data[2];

        /*
            Response frame format:

            ADDRESS
            LENGTH
            RESPONSE
            PAYLOAD
            OP_CODE
            CRC_H
            CRC_L
        */

        const std::size_t payload_len =
            data_len -
            kHeaderSize -
            kOpCodeSize -
            kCrcSize;

        if(payload_len > kMaxDataLen)
        {
            return false;
        }

        frame.payload_len = payload_len;

        for(std::size_t i = 0; i < payload_len; i++)
        {
            frame.payload[i] = data[3 + i];
        }

        frame.op_code =
            data[data_len - 3];

        return true;
    }

    static bool Validate(
        const uint8_t* data,
        std::size_t data_len) noexcept
    {
        if(data == nullptr || 
            data_len < kMinFrameSize)
        {
            return false;
        }

        const uint16_t received_crc =
            (static_cast<uint16_t>(data[data_len - 2]) << 8) |
            static_cast<uint16_t>(data[data_len - 1]);

        const uint16_t calculated_crc =
            Crc16(data, data_len - kCrcSize);

        return received_crc == calculated_crc;
    }

    static uint16_t Crc16(
        const uint8_t* data,
        std::size_t data_len) noexcept
    {
        uint16_t crc = 0x0000;

        for(std::size_t j = 0; j < data_len; j++)
        {
            uint16_t c =
                static_cast<uint16_t>(
                    ((crc >> 8) ^ data[j]) << 8);

            for(uint8_t i = 0; i < 8; i++)
            {
                if(c & 0x8000)
                {
                    c =
                        static_cast<uint16_t>(
                            (c << 1) ^ 0x1021);
                }
                else
                {
                    c =
                        static_cast<uint16_t>(
                            c << 1);
                }
            }

            crc =
                static_cast<uint16_t>(
                    c ^ (crc << 8));
        }

        return crc;
    }
};

} // Netronix

#endif