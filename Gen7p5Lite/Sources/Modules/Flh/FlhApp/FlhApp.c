

/***************************************************************************************************
                                   Local types, enums definitions
*****************************************************************************/
static uint8_t appFlhInstantFault_au8[FLH_NUMBER_OF_EVENTS];

/***************************************************************************************************
                  Function definitions
*****************************************************************************/
/*----------------------------------------------------------------------------
 *
 * FUNCTION NAME: FLHAPP_SetInstantFaultStatus()
 *
 * FUNCTION ARGUMENTS:
 *     Flh_EventId_t EventId            FLH fault ID
 *     Flh_EventStatus_t EventStatus    FLH fault status
 *
 * RETURN VALUE:
 *     None
 *
 * FUNCTION DESCRIPTION:
 *    Debounces the fault or passes it through depending on EventStatus.
 *    It does not report 'PREPASSED' if last status is also 'PREPASSED' not to dequalify the fault.
 *
 *---------------------------------------------------------------------------*/
void FLHAPP_SetInstantFaultStatus(Flh_EventId_t Flh_EventId, Flh_EventStatus_t Flh_EventStatus)
{
    if ((Flh_EventStatus == FLH_EVENT_STATUS_PREFAILED) || (Flh_EventStatus == FLH_EVENT_STATUS_FAILED))
    {
        appFlhInstantFault_au8[Flh_EventId] = 1U;
    }
    else
    {
        appFlhInstantFault_au8[Flh_EventId] = 0U;
    }
    /* store the APM faults data (i.e, module sates when provout time exceeded) when AB PROVEOUT INCOMPLETE fault report as failed */
    if ((Flh_AB_PROVEOUT_INCOMPLETE == Flh_EventId) && (Flh_EventStatus == FLH_EVENT_STATUS_FAILED) && (FLH_FAULT_UNTESTED == (fault_status_bits[Flh_AB_PROVEOUT_INCOMPLETE] & FLH_FAULT_UNTESTED)))
    {
        /* copy APM Ab faults details in to RAM buffer*/
        (void)DEM_ApmDataElementRead(&APM_FaultsDataElement_RAM[0]);
        /*write APM Ab faults details in to NVM  */
        (void)NvM_WriteBlock(NvMBlock_APM_FaultsDataElement, NULL_PTR);
    }
}