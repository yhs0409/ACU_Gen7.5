
/*******************************************************************************
                  Include files
*******************************************************************************/
#include "Flh_Types.h"


/*******************************************************************************
                  Exported function prototypes
*******************************************************************************/

/*----------------------------------------------------------------------------
 *
 * NOTE: ALL INTERFACES USE THE FUNCTION MACRO "Flh_ReportErrorStatus"!!!!!!!
 *       If you define "USE_DEM_DIRECT_ACCESS_METHOD" before you include
 *       this header file the macro will directly call the DEM FUNCTION!
 *
 * FUNCTION NAME: Flh_ReportErrorStatus()
 *
 * FUNCTION ARGUMENTS:
 *   Flh_EventId_t EventId - Flh fault ID
 *   Flh_EventStatus_t EventStatus - FLH action to perform
 *
 * RETURN VALUE:
 *     None.
 *
 * FUNCTION DESCRIPTION:
 *    Debounces the fault or passes it through depending on EventStatus.
 *
 *---------------------------------------------------------------------------*/
static inline void Flh_ReportErrorStatus(Flh_EventId_t EventId, Flh_EventStatus_t EventStatus)
{
    if (EventId < FLH_NUMBER_OF_EVENTS)
    {
        FLHAPP_SetInstantFaultStatus(EventId, EventStatus);
        if (EventId > FLH_REPORT_DIRECT_TO_DEM_AFTER_THIS)
        {
            Dem_ReportErrorStatus(FlhEventIdTypeToDemEventIdTypeTable[EventId], ((Dem_EventStatusType)EventStatus));
        }
        else
        {
            Flh_ReportErrorStatusFunc(EventId, EventStatus);
        }
    }
}