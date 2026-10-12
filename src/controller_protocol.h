#pragma once

#include <windows.h>

#define VDC_VENDOR_ID           0xDEED
#define VDC_PRODUCT_ID          0xFEED
#define VDC_VERSION_NUMBER      0x0101

#define VDC_JOYSTICK_REPORT_ID  1
#define VDC_FEEDER_REPORT_ID    2
#define VDC_FEEDER_USAGE_PAGE   0xFF00
#define VDC_FEEDER_USAGE        0x01

#define VDC_AXIS_MIN            0
#define VDC_AXIS_CENTER         16384
#define VDC_AXIS_MAX            32767

#define VDC_FLIGHT_MODE_ANGLE   0
#define VDC_FLIGHT_MODE_ACRO    1

#include <pshpack1.h>

// Field order and sizes must match the joystick report descriptor: Z, Rx, Y, X, Button 1 + 7 bits padding.
typedef struct _VDC_CONTROLLER_STATE {
    UINT16 Throttle;
    UINT16 Yaw;
    UINT16 Pitch;
    UINT16 Roll;
    UINT8 FlightMode;
} VDC_CONTROLLER_STATE;

typedef struct _VDC_STATE_REPORT {
    UINT8                ReportId;
    VDC_CONTROLLER_STATE State;
} VDC_STATE_REPORT;

#include <poppack.h>
