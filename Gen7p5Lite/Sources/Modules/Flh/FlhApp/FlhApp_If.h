

/*****************************************************************************
                  Exported function prototypes
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
 extern void FLHAPP_SetInstantFaultStatus(Flh_EventId_t Flh_EventId, Flh_EventStatus_t Flh_EventStatus);