// test/frame_ut.cpp

#include <gtest/gtest.h>

#include <iomanip>
#include <iostream>

#include "../src/netronix_frame.h"


template <std::size_t kLen>
void DumpFrame(const typename Netronix::NetronixSerdes<kLen>::Frame& f)
{
    std::cout << "=== Netronix Frame Dump ===\n";

    std::cout << "Address   : 0x"
              << std::hex << std::setw(2) << std::setfill('0')
              << static_cast<int>(f.address)
              << std::dec << "\n";

    std::cout << "Length    : "
              << static_cast<int>(f.length)
              << "\n";

    std::cout << "Command   : 0x"
              << std::hex << std::setw(2) << std::setfill('0')
              << static_cast<int>(f.command)
              << std::dec << "\n";

    std::cout << "Payload len: "
              << f.payload_len << "\n";

    std::cout << "Payload   : ";

    for (std::size_t i = 0; i < f.payload_len; i++)
    {
        std::cout << "0x"
                  << std::hex << std::setw(2) << std::setfill('0')
                  << static_cast<int>(f.payload[i]) << " ";
    }

    std::cout << std::dec << "\n";

    std::cout << "Op code   : 0x"
              << std::hex << std::setw(2) << std::setfill('0')
              << static_cast<int>(f.op_code)
              << std::dec << "\n";

    std::cout << "===========================\n";
}

template <std::size_t kLen>
void DumpFrame(const uint8_t (&data)[kLen])
{
    std::cout << "=== Netronix Frame Dump ===\n";

    for (const auto& i: data) {
        std::cout << std::hex << std::setw(2) << std::setfill('0')
        << static_cast<int>(i) << " ";
    }

    std::cout << std::endl;

    std::cout << "===========================\n";
}

template <std::size_t kLen>
std::size_t BuildResponseFrame(
    uint8_t* out,
    uint8_t address,
    uint8_t command,
    const uint8_t* payload,
    std::size_t payload_len,
    uint8_t opcode)
{
    using Serdes =
        Netronix::NetronixSerdes<kLen>;

    std::size_t idx = 0;

    out[idx++] = address;

    /*
        Response format:

        ADDRESS
        LENGTH
        COMMAND
        PAYLOAD
        OPCODE
        CRC_H
        CRC_L
    */

    const uint8_t frame_len =
        static_cast<uint8_t>(
            Serdes::kHeaderSize +
            payload_len +
            Serdes::kOpCodeSize +
            Serdes::kCrcSize);

    out[idx++] = frame_len;

    out[idx++] = command;

    for(std::size_t i = 0; i < payload_len; ++i)
    {
        out[idx++] = payload[i];
    }

    out[idx++] = opcode;

    const uint16_t crc =
        Serdes::Crc16(out, idx);

    out[idx++] =
        static_cast<uint8_t>((crc >> 8) & 0xFF);

    out[idx++] =
        static_cast<uint8_t>(crc & 0xFF);

    return idx;
}

using Serdes = Netronix::NetronixSerdes<32>;

TEST(NetronixFrame, Crc16ShouldMatchKnownValue)
{
    const uint8_t data[] =
    {
        0x01,
        0x05,
        0x00
    };

    const uint16_t crc =
        Serdes::Crc16(data, sizeof(data));

    EXPECT_EQ(crc, 0xC8C5);
}

TEST(NetronixFrame, SerializeFirmwareRequest)
{
    Serdes::Frame frame;

    frame.address = 0x01;
    frame.command = 0x72;

    uint8_t buffer[32] {};

    const std::size_t len =
        Serdes::SerializeCommand(
            buffer,
            sizeof(buffer),
            frame);

    ASSERT_EQ(len, 5);

    EXPECT_EQ(buffer[0], 0x01);
    EXPECT_EQ(buffer[1], 0x05);
    EXPECT_EQ(buffer[2], 0x72);
    EXPECT_EQ(buffer[3], 0x96);
    EXPECT_EQ(buffer[4], 0x10);

    DumpFrame<32>(frame);
    DumpFrame(buffer);
}

TEST(NetronixFrame, ValidateCorrectFrame)
{
    const uint8_t frame[] =
    {
        0x01,
        0x05,
        0x72,
        0x96,
        0x10
    };

    EXPECT_TRUE(
        Serdes::Validate(
            frame,
            sizeof(frame)));
}

TEST(NetronixFrame, RejectInvalidCrc)
{
    const uint8_t frame[] =
    {
        0x01,
        0x01,
        0x72,
        0x00,
        0x00
    };

    EXPECT_FALSE(
        Serdes::Validate(
            frame,
            sizeof(frame)));
}

TEST(NetronixFrame, RejectNullptrInValidate)
{
    EXPECT_FALSE(
        Serdes::Validate(
            nullptr,
            5));
}

TEST(NetronixFrame, RejectTooShortFrame)
{
    const uint8_t frame[] =
    {
        0x01,
        0x02
    };

    EXPECT_FALSE(
        Serdes::Validate(
            frame,
            sizeof(frame)));
}

TEST(NetronixFrame, SerializeFailsOnSmallBuffer)
{
    Serdes::Frame frame;

    frame.address = 0x01;
    frame.command = 0x72;

    uint8_t buffer[2] {};

    const std::size_t len =
        Serdes::SerializeCommand(
            buffer,
            sizeof(buffer),
            frame);

    EXPECT_EQ(len, 0);
}

TEST(NetronixFrame, SerializeFailsOnTooLargePayload)
{
    Netronix::NetronixSerdes<2>::Frame frame;

    frame.address = 0x01;
    frame.command = 0x72;

    frame.payload_len = 3;

    uint8_t buffer[32] {};

    const std::size_t len =
        Netronix::NetronixSerdes<2>::SerializeCommand(
            buffer,
            sizeof(buffer),
            frame);

    EXPECT_EQ(len, 0);
}

TEST(NetronixFrame, SerializeDummyFrame1) {
    uint8_t buffer[32] {};

    const uint8_t data[] =
    {
        0x01,
        0x02,
        0x03,
        0x04,
        0x05
    };

    Serdes::Frame frame {
        0x01, 
        0x72,
        sizeof(data),
        data
    };

    Serdes::SerializeCommand(
        buffer, 
        sizeof(buffer),
        frame);

    std::size_t i = 0;

    EXPECT_EQ(buffer[i++], 0x01);
    EXPECT_EQ(buffer[i++], 0x0A);
    EXPECT_EQ(buffer[i++], 0x72);

    for (std::size_t j = 0; j < sizeof(data); ++j) {
        EXPECT_EQ(buffer[i++], data[j]);
    }

    EXPECT_EQ(buffer[i++], 0xF8);
    EXPECT_EQ(buffer[i], 0x49);

    DumpFrame<32>(frame);
    DumpFrame(buffer);
}

TEST(NetronixFrame, SerializeDummyFrame2) {
    uint8_t buffer[32] {};

    const uint8_t data[] =
    {
        0x01,
        0x02,
        0x03,
        0x04,
        0x05
    };

    std::array<uint8_t, 32> initializer;

    for (std::size_t i = 0; i < sizeof(data); ++i) {
        initializer.at(i) = data[i];
    }

    Serdes::Frame frame {
        0x01, 
        0x72,
        sizeof(data),
        initializer
    };

    Serdes::SerializeCommand(
        buffer, 
        sizeof(buffer),
        frame);

    std::size_t i = 0;

    EXPECT_EQ(buffer[i++], 0x01);
    EXPECT_EQ(buffer[i++], 0x0A);
    EXPECT_EQ(buffer[i++], 0x72);

    for (std::size_t j = 0; j < sizeof(data); ++j) {
        EXPECT_EQ(buffer[i++], initializer[j]);
    }

    EXPECT_EQ(buffer[i++], 0xF8);
    EXPECT_EQ(buffer[i], 0x49);

    DumpFrame<32>(frame);
    DumpFrame(buffer);
}

TEST(NetronixFrame, DeserializeCorrectFrame)
{
    uint8_t frame[32] {};

    const uint8_t payload[] =
    {
        0x11,
        0x22,
        0x33
    };

    const std::size_t len =
        BuildResponseFrame<32>(
            frame,
            0x01,
            0x72,
            payload,
            sizeof(payload),
            0x99);

    
    
    Serdes::Frame parsed {};

    EXPECT_TRUE(
        Serdes::DeserializeResponse(
            frame,
            len,
            parsed));

    EXPECT_EQ(parsed.address, 0x01);
    EXPECT_EQ(parsed.length, len);
    EXPECT_EQ(parsed.command, 0x72);

    EXPECT_EQ(parsed.payload_len, 3);

    EXPECT_EQ(parsed.payload[0], 0x11);
    EXPECT_EQ(parsed.payload[1], 0x22);
    EXPECT_EQ(parsed.payload[2], 0x33);

    EXPECT_EQ(parsed.op_code, 0x99);

    DumpFrame<32>(parsed);
    DumpFrame(frame);
}

TEST(NetronixFrame, DeserializeRejectInvalidCrc)
{
    const uint8_t frame[] =
    {
        0x01, 
        0x0a, 
        0x72, 
        0x01,
        0x02,
        0x03, 
        0x04, 
        0x05, 
        0xbe, 
        0xef
    };

    Serdes::Frame parsed {};

    EXPECT_FALSE(
        Serdes::DeserializeResponse(
            frame,
            sizeof(frame),
            parsed));
}

TEST(NetronixFrame, DeserializeRejectNullptr)
{
    Serdes::Frame parsed {};

    EXPECT_FALSE(
        Serdes::DeserializeResponse(
            nullptr,
            10,
            parsed));
}

TEST(NetronixFrame, DeserializeRejectTooShortFrame)
{
    const uint8_t frame[] =
    {
        0x01,
        0x02
    };

    Serdes::Frame parsed {};

    EXPECT_FALSE(
        Serdes::DeserializeResponse(
            frame,
            sizeof(frame),
            parsed));
}