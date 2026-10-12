#include "driver.h"

static VOID
FillJoystickReport(
    _In_  PDEVICE_CONTEXT   DeviceContext,
    _Out_ VDC_STATE_REPORT* Report
    )
{
    Report->ReportId = VDC_JOYSTICK_REPORT_ID;
    Report->State    = DeviceContext->State;
}

static NTSTATUS
CopyJoystickReport(
    _In_ PDEVICE_CONTEXT DeviceContext,
    _In_ WDFREQUEST      Request
    )
{
    VDC_STATE_REPORT report;
    NTSTATUS         status;

    FillJoystickReport(DeviceContext, &report);
    status = RequestCopyFromBuffer(Request, &report, sizeof(report));
    if (NT_SUCCESS(status)) {
        DeviceContext->HasUnreportedState = FALSE;
    }
    return status;
}

NTSTATUS
JoystickCreatePendingReadQueue(
    _In_  WDFDEVICE Device,
    _Out_ WDFQUEUE* Queue
    )
{
    WDF_IO_QUEUE_CONFIG config;

    WDF_IO_QUEUE_CONFIG_INIT(&config, WdfIoQueueDispatchManual);
    return WdfIoQueueCreate(Device, &config, WDF_NO_OBJECT_ATTRIBUTES, Queue);
}

NTSTATUS
JoystickReadReport(
    _In_  PDEVICE_CONTEXT DeviceContext,
    _In_  WDFREQUEST      Request,
    _Out_ BOOLEAN*        CompleteRequest
    )
{
    NTSTATUS status;

    if (DeviceContext->HasUnreportedState) {
        *CompleteRequest = TRUE;
        return CopyJoystickReport(DeviceContext, Request);
    }

    status = WdfRequestForwardToIoQueue(Request, DeviceContext->PendingReadQueue);
    *CompleteRequest = (BOOLEAN)!NT_SUCCESS(status);
    return status;
}

VOID
JoystickPublishState(
    _In_ PDEVICE_CONTEXT DeviceContext
    )
{
    WDFREQUEST pendingRead;

    DeviceContext->HasUnreportedState = TRUE;
    if (NT_SUCCESS(WdfIoQueueRetrieveNextRequest(DeviceContext->PendingReadQueue, &pendingRead))) {
        WdfRequestComplete(pendingRead, CopyJoystickReport(DeviceContext, pendingRead));
    }
}

NTSTATUS
JoystickGetInputReport(
    _In_ PDEVICE_CONTEXT DeviceContext,
    _In_ WDFREQUEST      Request
    )
{
    PUCHAR   buffer;
    NTSTATUS status = RequestRetrieveReportToRead(Request, VDC_JOYSTICK_REPORT_ID, sizeof(VDC_STATE_REPORT), &buffer);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    FillJoystickReport(DeviceContext, (VDC_STATE_REPORT*)buffer);
    WdfRequestSetInformation(Request, sizeof(VDC_STATE_REPORT));
    return STATUS_SUCCESS;
}
