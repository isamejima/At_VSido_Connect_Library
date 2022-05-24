#include "gtest/gtest.h"
#include "gmock/gmock.h"

#include "At_Vsido_Connect_Library.h"

TEST(AT_VSIDO_CONNECT_LIBRARY_Test,op_o_test_target_0)
{
    At_Vsido_Connect_Library vsido;
    vsido.servo_present_angles[1] = 0;
    unsigned char receive_packet[]={0xff,0x6f,0x08,0x02,0x01,0x00,0x00,0x9b};
    unsigned char return_packet[] = {0xff, 0x6f, 0x08, 0x01, 0x00, 0x00, 0x00, 0x99};

    bool ret=false;

    for(int i=0;i<8;i++){
        ret=vsido.read1byte(receive_packet[i]);
    }
    ASSERT_EQ(ret, true);

    ASSERT_THAT(receive_packet, testing::ElementsAreArray(vsido.pc_rstr,8));

    vsido.unpackPacket();

    EXPECT_EQ(vsido.servo_angles[1], 0);
    EXPECT_EQ(vsido.servo_present_angles[1],0);

    EXPECT_EQ(vsido.r_ln,8);

    ASSERT_EQ(vsido.r_ln, 8);
    EXPECT_THAT(return_packet, testing::ElementsAreArray(vsido.r_str, 8));
}

TEST(AT_VSIDO_CONNECT_LIBRARY_Test, op_o_test_target_900)
{
    At_Vsido_Connect_Library vsido;
    vsido.servo_present_angles[1] = 0;

    unsigned char receive_packet[] = {0xff, 0x6f, 0x08, 0x02, 0x01, 0x08, 0x0e, 0x9d};
    unsigned char return_packet[] = {0xff, 0x6f, 0x08, 0x01, 0x00, 0x00, 0x00, 0x99};

    bool ret;
    for (int i = 0; i < 8; i++)
    {
        ret = vsido.read1byte(receive_packet[i]);
    }
    ASSERT_EQ(ret, true);

    vsido.unpackPacket();

    EXPECT_EQ(vsido.servo_angles[1], 900);
    EXPECT_EQ(vsido.servo_present_angles[1], 0);

    ASSERT_EQ(vsido.r_ln, 8);
    EXPECT_THAT(return_packet, testing::ElementsAreArray(vsido.r_str, 8));
}

TEST(AT_VSIDO_CONNECT_LIBRARY_Test, op_o_test_target_minus900)
{
    At_Vsido_Connect_Library vsido;
    vsido.servo_present_angles[1]=0;

    unsigned char receive_packet[] = {0xff, 0x6f, 0x08, 0x02, 0x01, 0xf8, 0xf0, 0x93};
    unsigned char return_packet[] = {0xff, 0x6f, 0x08, 0x01, 0x00, 0x00, 0x00, 0x99};

    bool ret;
    for (int i = 0; i < 8; i++)
    {
        ret = vsido.read1byte(receive_packet[i]);
    }
    ASSERT_EQ(ret, true);

    vsido.unpackPacket();

    EXPECT_EQ(vsido.servo_angles[1], -900);
    EXPECT_EQ(vsido.servo_present_angles[1], 0);

    ASSERT_EQ(vsido.r_ln, 8);
    EXPECT_THAT(return_packet, testing::ElementsAreArray(vsido.r_str, 8));
}

TEST(AT_VSIDO_CONNECT_LIBRARY_Test, op_t_test_target_0)
{
    At_Vsido_Connect_Library vsido;
    vsido.servo_present_torques[1] = 0;
    unsigned char receive_packet[] = {0xff, 0x74, 0x08, 0x02, 0x01, 0x00, 0x00, 0x80};
    unsigned char return_packet[] = {0xff, 0x74, 0x08, 0x01, 0x00, 0x00, 0x00, 0x82};

    bool ret = false;

    for (int i = 0; i < 8; i++)
    {
        ret = vsido.read1byte(receive_packet[i]);
    }
    ASSERT_EQ(ret, true);

    vsido.unpackPacket();

    EXPECT_EQ(vsido.servo_torques[1], 0);
    EXPECT_EQ(vsido.servo_present_torques[1], 0);

    ASSERT_EQ(vsido.r_ln, 8);
    EXPECT_THAT(return_packet, testing::ElementsAreArray(vsido.r_str, 8));
}

TEST(AT_VSIDO_CONNECT_LIBRARY_Test, op_t_test_target_1000)
{
    At_Vsido_Connect_Library vsido;
    vsido.servo_present_torques[1] = 0;

    unsigned char receive_packet[] = {0xff, 0x74, 0x08, 0x02, 0x01, 0xd0, 0x0e, 0x5e};
    unsigned char return_packet[] = {0xff, 0x74, 0x08, 0x01, 0x00, 0x00, 0x00, 0x82};

    bool ret;
    for (int i = 0; i < 8; i++)
    {
        ret = vsido.read1byte(receive_packet[i]);
    }
    ASSERT_EQ(ret, true);

    vsido.unpackPacket();

    EXPECT_EQ(vsido.servo_torques[1], 1000);
    EXPECT_EQ(vsido.servo_present_torques[1], 0);

    ASSERT_EQ(vsido.r_ln, 8);
    EXPECT_THAT(return_packet, testing::ElementsAreArray(vsido.r_str, 8));
}

TEST(AT_VSIDO_CONNECT_LIBRARY_Test, op_t_test_target_minus1000)
{
    At_Vsido_Connect_Library vsido;
    vsido.servo_present_torques[1] = 0;

    unsigned char receive_packet[] = {0xff, 0x74, 0x08, 0x02, 0x01, 0x30, 0xf0, 0x40};
    unsigned char return_packet[] = {0xff, 0x74, 0x08, 0x01, 0x00, 0x00, 0x00, 0x82};

    bool ret;
    for (int i = 0; i < 8; i++)
    {
        ret = vsido.read1byte(receive_packet[i]);
    }
    ASSERT_EQ(ret, true);

    vsido.unpackPacket();

    EXPECT_EQ(vsido.servo_torques[1], -1000);
    EXPECT_EQ(vsido.servo_present_torques[1], 0);

    ASSERT_EQ(vsido.r_ln, 8);
    EXPECT_THAT(return_packet, testing::ElementsAreArray(vsido.r_str, 8));
}