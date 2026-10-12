#include "driver.h"

//
// UMDF cannot marshal the embedded pointer of HID_XFER_PACKET, so mshidumdf.sys
// splits it into two request buffers:
//   read from device: report buffer = output buffer, report id = first byte of input buffer;
//   write to device:  report buffer = input buffer,  report id = output buffer LENGTH,
//                     because UMDF output buffers are write-only.
//

static NTSTATUS
GetXferPacketToRead(
    _In_  WDFREQUEST       Request,
    _Out_ HID_XFER_PACKET* Packet
    )
{
    WDFMEMORY inputMemory;
    WDFMEMORY outputMemory;
    size_t    inputLength;
    size_t    outputLength;
    PUCHAR    input;
    NTSTATUS  status;

    status = WdfRequestRetrieveInputMemory(Request, &inputMemory);
    if (!NT_SUCCESS(status)) {
        return status;
    }
    input = (PUCHAR)WdfMemoryGetBuffer(inputMemory, &inputLength);
    if (inputLength < sizeof(UCHAR)) {
        return STATUS_INVALID_BUFFER_SIZE;
    }

    status = WdfRequestRetrieveOutputMemory(Request, &outputMemory);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    Packet->reportId        = *input;
    Packet->reportBuffer    = (PUCHAR)WdfMemoryGetBuffer(outputMemory, &outputLength);
    Packet->reportBufferLen = (ULONG)outputLength;
    return STATUS_SUCCESS;
}

static NTSTATUS
GetXferPacketToWrite(
    _In_  WDFREQUEST       Request,
    _Out_ HID_XFER_PACKET* Packet
    )
{
    WDFMEMORY inputMemory;
    WDFMEMORY outputMemory;
    size_t    inputLength;
    size_t    outputLength;
    NTSTATUS  status;

    status = WdfRequestRetrieveOutputMemory(Request, &outputMemory);
    if (!NT_SUCCESS(status)) {
        return status;
    }
    WdfMemoryGetBuffer(outputMemory, &outputLength);

    status = WdfRequestRetrieveInputMemory(Request, &inputMemory);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    Packet->reportId        = (UCHAR)outputLength;
    Packet->reportBuffer    = (PUCHAR)WdfMemoryGetBuffer(inputMemory, &inputLength);
    Packet->reportBufferLen = (ULONG)inputLength;
    return STATUS_SUCCESS;
}

static NTSTATUS
CheckReport(
    _In_ const HID_XFER_PACKET* Packet,
    _In_ UCHAR                  ReportId,
    _In_ size_t                 ReportSize
    )
{
    if (Packet->reportId != ReportId) {
        return STATUS_INVALID_PARAMETER;
    }
    if (Packet->reportBufferLen < ReportSize) {
        return STATUS_INVALID_BUFFER_SIZE;
    }
    return STATUS_SUCCESS;
}

NTSTATUS
RequestCopyFromBuffer(
    _In_ WDFREQUEST Request,
    _In_reads_bytes_(Size) const VOID* Source,
    _In_ size_t     Size
    )
{
    WDFMEMORY memory;
    size_t    outputLength;
    NTSTATUS  status;

    status = WdfRequestRetrieveOutputMemory(Request, &memory);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    WdfMemoryGetBuffer(memory, &outputLength);
    if (outputLength < Size) {
        return STATUS_INVALID_BUFFER_SIZE;
    }

    status = WdfMemoryCopyFromBuffer(memory, 0, (PVOID)Source, Size);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    WdfRequestSetInformation(Request, Size);
    return STATUS_SUCCESS;
}

NTSTATUS
RequestRetrieveReportToRead(
    _In_  WDFREQUEST Request,
    _In_  UCHAR      ReportId,
    _In_  size_t     ReportSize,
    _Out_ PUCHAR*    Report
    )
{
    HID_XFER_PACKET packet;
    NTSTATUS        status;

    *Report = NULL;
    status = GetXferPacketToRead(Request, &packet);
    if (NT_SUCCESS(status)) {
        status = CheckReport(&packet, ReportId, ReportSize);
    }
    if (NT_SUCCESS(status)) {
        *Report = packet.reportBuffer;
    }
    return status;
}

NTSTATUS
RequestRetrieveReportToWrite(
    _In_  WDFREQUEST    Request,
    _In_  UCHAR         ReportId,
    _In_  size_t        ReportSize,
    _Out_ const UCHAR** Report
    )
{
    HID_XFER_PACKET packet;
    NTSTATUS        status;

    *Report = NULL;
    status = GetXferPacketToWrite(Request, &packet);
    if (NT_SUCCESS(status)) {
        status = CheckReport(&packet, ReportId, ReportSize);
    }
    if (NT_SUCCESS(status)) {
        *Report = packet.reportBuffer;
    }
    return status;
}

NTSTATUS
RequestGetStringId(
    _In_  WDFREQUEST Request,
    _Out_ ULONG*     StringId
    )
{
    WDFMEMORY inputMemory;
    size_t    inputLength;
    PULONG    input;
    NTSTATUS  status;

    *StringId = 0;
    status = WdfRequestRetrieveInputMemory(Request, &inputMemory);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    input = (PULONG)WdfMemoryGetBuffer(inputMemory, &inputLength);
    if (inputLength < sizeof(ULONG)) {
        return STATUS_INVALID_BUFFER_SIZE;
    }

    // Low word is the string id, high word is the language id.
    *StringId = *input & 0xFFFF;
    return STATUS_SUCCESS;
}
