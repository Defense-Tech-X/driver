#include "driver.h"

DRIVER_INITIALIZE DriverEntry;
static EVT_WDF_DRIVER_DEVICE_ADD EvtDeviceAdd;

static const VDC_CONTROLLER_STATE InitialState = {
    VDC_AXIS_MIN,           // Throttle
    VDC_AXIS_CENTER,        // Yaw
    VDC_AXIS_CENTER,        // Pitch
    VDC_AXIS_CENTER,        // Roll
    VDC_FLIGHT_MODE_ANGLE,
};

NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT  DriverObject,
    _In_ PUNICODE_STRING RegistryPath
    )
{
    WDF_DRIVER_CONFIG config;

    WDF_DRIVER_CONFIG_INIT(&config, EvtDeviceAdd);
    return WdfDriverCreate(DriverObject, RegistryPath, WDF_NO_OBJECT_ATTRIBUTES, &config, WDF_NO_HANDLE);
}

static NTSTATUS
EvtDeviceAdd(
    _In_    WDFDRIVER       Driver,
    _Inout_ PWDFDEVICE_INIT DeviceInit
    )
{
    WDF_OBJECT_ATTRIBUTES attributes;
    WDFDEVICE             device;
    PDEVICE_CONTEXT       deviceContext;
    NTSTATUS              status;

    UNREFERENCED_PARAMETER(Driver);

    // A HID minidriver under mshidumdf.sys must be a filter: power policy belongs to the HID class driver.
    WdfFdoInitSetFilter(DeviceInit);

    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&attributes, DEVICE_CONTEXT);
    status = WdfDeviceCreate(&DeviceInit, &attributes, &device);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    deviceContext = GetDeviceContext(device);
    deviceContext->State              = InitialState;
    deviceContext->HasUnreportedState = TRUE;

    status = JoystickCreatePendingReadQueue(device, &deviceContext->PendingReadQueue);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    return CreateDefaultQueue(device);
}
