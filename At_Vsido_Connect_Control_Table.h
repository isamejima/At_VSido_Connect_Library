#ifndef AT_VSIDO_CONNECT_CONTROL_TABLE
#define AT_VSIDO_CONNECT_CONTROL_TABLE

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#pragma pack(1)
typedef struct
{
    unsigned char reserved[18];
    short present_angle;  // 19-20
    short present_time;   // 21-22
    short present_speed;  // 23-24
    short present_torque; // 25-26
} AT_Vsido_Control_Table;
#pragma pack()

#endif // AT_VSIDO_CONNECT_CONTROL_TABLE
