#include "driver.h"

static BOOLEAN
IsAxisInRange(
    _In_ UINT16 Value
    )
{
    return (BOOLEAN)(Value >= VDC_AXIS_MIN && Value <= VDC_AXIS_MAX);
}

static BOOLEAN
IsValidState(
    _In_ const VDC_CONTROLLER_STATE* State
    )
{
    return (BOOLEAN)(IsAxisInRange(State->Throttle) &&
                     IsAxisInRange(State->Yaw) &&
                     IsAxisInRange(State->Pitch) &&
                     IsAxisInRange(State->Roll) &&
                     (State->FlightMode == VDC_FLIGHT_MODE_ANGLE || State->FlightMode == VDC_FLIGHT_MODE_ACRO));
}

NTSTATUS
FeederSetState(
    _In_ PDEVICE_CONTEXT DeviceContext,
    _In_ WDFREQUEST      Request
    )
{
    const UCHAR*            buffer;
    const VDC_STATE_REPORT* report;
    NTSTATUS                status;

    status = RequestRetrieveReportToWrite(Request, VDC_FEEDER_REPORT_ID, sizeof(VDC_STATE_REPORT), &buffer);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    report = (const VDC_STATE_REPORT*)buffer;
    if (!IsValidState(&report->State)) {
        return STATUS_INVALID_PARAMETER;
    }

    DeviceContext->State = report->State;
    JoystickPublishState(DeviceContext);

    WdfRequestSetInformation(Request, sizeof(VDC_STATE_REPORT));
    return STATUS_SUCCESS;
}

NTSTATUS
FeederGetState(
    _In_ PDEVICE_CONTEXT DeviceContext,
    _In_ WDFREQUEST      Request
    )
{
    PUCHAR            buffer;
    VDC_STATE_REPORT* report;
    NTSTATUS          status;

    status = RequestRetrieveReportToRead(Request, VDC_FEEDER_REPORT_ID, sizeof(VDC_STATE_REPORT), &buffer);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    report = (VDC_STATE_REPORT*)buffer;
    report->ReportId = VDC_FEEDER_REPORT_ID;
    report->State    = DeviceContext->State;

    WdfRequestSetInformation(Request, sizeof(VDC_STATE_REPORT));
    return STATUS_SUCCESS;
}
