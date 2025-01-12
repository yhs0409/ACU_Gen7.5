/*File Description*/
/*this file supports Soc Fault handler module's main functionality*/

/*******************************************************************************
                  Include files
*******************************************************************************/
#include "Flh_If.h"

/*******************************************************************************
                  Local object definitions
*******************************************************************************/
/* module state */
static Flh_ModuleState_t flh_module_state = FLH_MODULE_IDLE_STATE;
/*******************************************************************************
                  Exported object definitions
*******************************************************************************/
FlhFaultFlagType fault_status_bits[((uint16_t)FLH_REPORT_DIRECT_TO_DEM_AFTER_THIS) + 1U];

uint8_t Flh_InhibitFlags[((uint16_t)FLH_REPORT_DIRECT_TO_DEM_AFTER_THIS) + 1U];

/*******************************************************************************
                  Local Function definitions
*******************************************************************************/

/*----------------------------------------------------------------------------
 *
 * FUNCTION NAME: Flh_ReportErrorStatusFunc()
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
void Flh_ReportErrorStatusFunc(Flh_EventId EventId, Flh_EventStatus_t EventStatus)
{
    Flh_ConfigType const *pConfig;

    if ((flh_module_state == FLH_MODULE_RUN_STATE) && (Flh_InhibitFlags[EventId] != 0x01U))
    {
        pConfig = &Flh_ConfigData[EventId];

        if (Flh_AlgoritmTable[pConfig->AlgorithmIndex](EventId, EventStatus) != DEBOUNCE_FAULT_NO_ACTION)
        {
            FLH_REF_COUNTED_DISABLE_INTERRUPTS();
            if (GetNumEmptyElements() != 0U)
            {
                queue_elements[putIndex].flhEventId = EventId;
                queue_elements[putIndex].flhEventStatus = EventStatus;
                putIndex++;
                putIndex &= FLH_QUEUE_LENGTH_MASK;
            }
            else
            {
                /* Report out of queue elements to send to the DEM */
                Flh_DetReportError(MODULE_ID_Flh, 0, API_Flh_ReportErrorStatusFunc, FLH_QUEUE_FULL);
                __debug();
            }
            FLH_REF_COUNTED_ENABLE_INTERRUPTS();
        }
    }
}