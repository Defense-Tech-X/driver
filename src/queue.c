#include "driver.h"

static EVT_WDF_IO_QUEUE_IO_DEVICE_CONTROL EvtIoDeviceControl;

NTSTATUS
CreateDefaultQueue(
    _In_ WDFDEVICE Device
    )
{
    WDF_IO_QUEUE_CONFIG config;

    // Sequential dispatch serializes all access to DEVICE_CONTEXT::State without extra locking.
    WDF_IO_QUEUE_CONFIG_INIT_DEFAULT_QUEUE(&config, WdfIoQueueDispatchSequential);
    config.EvtIoDeviceControl = EvtIoDeviceControl;

    return WdfIoQueueCreate(Device, &config, WDF_NO_OBJECT_ATTRIBUTES, WDF_NO_HANDLE);
}

static VOID
EvtIoDeviceControl(
    _In_ WDFQUEUE   Queue,
    _In_ WDFREQUEST Request,
    _In_ size_t     OutputBufferLength,
    _In_ size_t     InputBufferLength,
    _In_ ULONG      IoControlCode
    )
{
    PDEVICE_CONTEXT deviceContext   = GetDeviceContext(WdfIoQueueGetDevice(Queue));
    BOOLEAN         completeRequest = TRUE;
    NTSTATUS        status;

    UNREFERENCED_PARAMETER(OutputBufferLength);
    UNREFERENCED_PARAMETER(InputBufferLength);

    switch (IoControlCode) {
    case IOCTL_HID_GET_DEVICE_DESCRIPTOR:
        status = IdentityCopyHidDescriptor(Request);
        break;
    case IOCTL_HID_GET_REPORT_DESCRIPTOR:
        status = IdentityCopyReportDescriptor(Request);
        break;
    case IOCTL_HID_GET_DEVICE_ATTRIBUTES:
        status = IdentityCopyDeviceAttributes(Request);
        break;
    case IOCTL_HID_GET_STRING:
        status = IdentityCopyString(Request);
        break;
    case IOCTL_HID_READ_REPORT:
        status = JoystickReadReport(deviceContext, Request, &completeRequest);
        break;
    case IOCTL_UMDF_HID_GET_INPUT_REPORT:
        status = JoystickGetInputReport(deviceContext, Request);
        break;
    case IOCTL_UMDF_HID_SET_FEATURE:
        status = FeederSetState(deviceContext, Request);
        break;
    case IOCTL_UMDF_HID_GET_FEATURE:
        status = FeederGetState(deviceContext, Request);
        break;
    default:
        status = STATUS_NOT_IMPLEMENTED;
        break;
    }

    if (completeRequest) {
        WdfRequestComplete(Request, status);
    }
}
