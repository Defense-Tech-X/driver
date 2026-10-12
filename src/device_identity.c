#include "driver.h"

#define MANUFACTURER_STRING L"Nikita Biziaev"
#define PRODUCT_STRING      L"Virtual Drone Controller"

static const UCHAR ReportDescriptor[] = {
    // Joystick seen by games: field order must match VDC_CONTROLLER_STATE.
    0x05, 0x01,                     // USAGE_PAGE (Generic Desktop)
    0x09, 0x04,                     // USAGE (Joystick)
    0xA1, 0x01,                     // COLLECTION (Application)
    0x85, VDC_JOYSTICK_REPORT_ID,   //   REPORT_ID
    0x09, 0x32,                     //   USAGE (Z)  Throttle
    0x09, 0x33,                     //   USAGE (Rx) Yaw
    0x09, 0x31,                     //   USAGE (Y)  Pitch
    0x09, 0x30,                     //   USAGE (X)  Roll
    0x15, 0x00,                     //   LOGICAL_MINIMUM (0)
    0x26, 0xFF, 0x7F,               //   LOGICAL_MAXIMUM (32767)
    0x75, 0x10,                     //   REPORT_SIZE (16)
    0x95, 0x04,                     //   REPORT_COUNT (4)
    0x81, 0x02,                     //   INPUT (Data,Var,Abs)
    0x05, 0x09,                     //   USAGE_PAGE (Button)
    0x19, 0x01,                     //   USAGE_MINIMUM (Button 1) Acro mode
    0x29, 0x01,                     //   USAGE_MAXIMUM (Button 1)
    0x15, 0x00,                     //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                     //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                     //   REPORT_SIZE (1)
    0x95, 0x01,                     //   REPORT_COUNT (1)
    0x81, 0x02,                     //   INPUT (Data,Var,Abs)
    0x75, 0x07,                     //   REPORT_SIZE (7)
    0x95, 0x01,                     //   REPORT_COUNT (1)
    0x81, 0x03,                     //   INPUT (Cnst,Var,Abs)
    0xC0,                           // END_COLLECTION

    // Feeder channel: VDC_CONTROLLER_STATE as an opaque feature report.
    0x06, 0x00, 0xFF,               // USAGE_PAGE (Vendor Defined 0xFF00)
    0x09, VDC_FEEDER_USAGE,         // USAGE (Vendor Usage 1)
    0xA1, 0x01,                     // COLLECTION (Application)
    0x85, VDC_FEEDER_REPORT_ID,     //   REPORT_ID
    0x09, VDC_FEEDER_USAGE,         //   USAGE (Vendor Usage 1)
    0x15, 0x00,                     //   LOGICAL_MINIMUM (0)
    0x26, 0xFF, 0x00,               //   LOGICAL_MAXIMUM (255)
    0x75, 0x08,                     //   REPORT_SIZE (8)
    0x95, (UCHAR)sizeof(VDC_CONTROLLER_STATE), // REPORT_COUNT
    0xB1, 0x02,                     //   FEATURE (Data,Var,Abs)
    0xC0,                           // END_COLLECTION
};

static const HID_DESCRIPTOR HidDescriptor = {
    0x09,                           // bLength
    0x21,                           // bDescriptorType (HID)
    0x0100,                         // bcdHID
    0x00,                           // bCountry
    0x01,                           // bNumDescriptors
    { 0x22, sizeof(ReportDescriptor) } // report descriptor type and length
};

static const HID_DEVICE_ATTRIBUTES DeviceAttributes = {
    sizeof(HID_DEVICE_ATTRIBUTES),
    VDC_VENDOR_ID,
    VDC_PRODUCT_ID,
    VDC_VERSION_NUMBER,
};

NTSTATUS
IdentityCopyHidDescriptor(
    _In_ WDFREQUEST Request
    )
{
    return RequestCopyFromBuffer(Request, &HidDescriptor, HidDescriptor.bLength);
}

NTSTATUS
IdentityCopyReportDescriptor(
    _In_ WDFREQUEST Request
    )
{
    return RequestCopyFromBuffer(Request, ReportDescriptor, sizeof(ReportDescriptor));
}

NTSTATUS
IdentityCopyDeviceAttributes(
    _In_ WDFREQUEST Request
    )
{
    return RequestCopyFromBuffer(Request, &DeviceAttributes, sizeof(DeviceAttributes));
}

NTSTATUS
IdentityCopyString(
    _In_ WDFREQUEST Request
    )
{
    ULONG    stringId;
    NTSTATUS status = RequestGetStringId(Request, &stringId);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    switch (stringId) {
    case HID_STRING_ID_IMANUFACTURER:
        return RequestCopyFromBuffer(Request, MANUFACTURER_STRING, sizeof(MANUFACTURER_STRING));
    case HID_STRING_ID_IPRODUCT:
        return RequestCopyFromBuffer(Request, PRODUCT_STRING, sizeof(PRODUCT_STRING));
    default:
        return STATUS_INVALID_PARAMETER;
    }
}
