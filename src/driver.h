#pragma once

#include <windows.h>
#include <wdf.h>
#include <hidport.h>

#include "controller_protocol.h"

typedef struct _DEVICE_CONTEXT {
    WDFQUEUE             PendingReadQueue;
    VDC_CONTROLLER_STATE State;
    BOOLEAN              HasUnreportedState;
} DEVICE_CONTEXT, *PDEVICE_CONTEXT;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(DEVICE_CONTEXT, GetDeviceContext);

// queue.c
NTSTATUS CreateDefaultQueue(_In_ WDFDEVICE Device);

// device_identity.c
NTSTATUS IdentityCopyHidDescriptor(_In_ WDFREQUEST Request);
NTSTATUS IdentityCopyReportDescriptor(_In_ WDFREQUEST Request);
NTSTATUS IdentityCopyDeviceAttributes(_In_ WDFREQUEST Request);
NTSTATUS IdentityCopyString(_In_ WDFREQUEST Request);

// joystick_input.c
NTSTATUS JoystickCreatePendingReadQueue(_In_ WDFDEVICE Device, _Out_ WDFQUEUE* Queue);
NTSTATUS JoystickReadReport(_In_ PDEVICE_CONTEXT DeviceContext, _In_ WDFREQUEST Request, _Out_ BOOLEAN* CompleteRequest);
NTSTATUS JoystickGetInputReport(_In_ PDEVICE_CONTEXT DeviceContext, _In_ WDFREQUEST Request);
VOID     JoystickPublishState(_In_ PDEVICE_CONTEXT DeviceContext);

// feeder_channel.c
NTSTATUS FeederSetState(_In_ PDEVICE_CONTEXT DeviceContext, _In_ WDFREQUEST Request);
NTSTATUS FeederGetState(_In_ PDEVICE_CONTEXT DeviceContext, _In_ WDFREQUEST Request);

// request_buffers.c
NTSTATUS RequestCopyFromBuffer(_In_ WDFREQUEST Request, _In_reads_bytes_(Size) const VOID* Source, _In_ size_t Size);
NTSTATUS RequestRetrieveReportToRead(_In_ WDFREQUEST Request, _In_ UCHAR ReportId, _In_ size_t ReportSize, _Out_ PUCHAR* Report);
NTSTATUS RequestRetrieveReportToWrite(_In_ WDFREQUEST Request, _In_ UCHAR ReportId, _In_ size_t ReportSize, _Out_ const UCHAR** Report);
NTSTATUS RequestGetStringId(_In_ WDFREQUEST Request, _Out_ ULONG* StringId);
