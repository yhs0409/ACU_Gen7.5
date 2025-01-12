/*File description*/
/*this file supports SoC Fault handler module's main functionality*/

/******************************************************************************
 *  Function defs for various debounce algorithms
 *****************************************************************************/
static Flh_AlgoReturn_t Flh_DebounceEventCounterBased(Flh_EventId_t EventId, Flh_EventStatus_t EventStatus);

/******************************************************************************
 *   Table of counter based debounce functions
 *****************************************************************************/
const Flh_DebounceAlgorithm_t Flh_AlgoritmTable[FLH_NUM_ALGORITHMS] =
    {
        Flh_DebounceEventCounterBased};

/*----------------------------------------------------------------------------
 *
 * FUNCTION NAME: Flh_DebounceEventCounterBased()
 *
 * FUNCTION ARGUMENTS:
 *   Flh_EventId_t EventId         FLH fault ID
 *   Flh_EventStatus_t EventStatus FLH fault status
 *
 * RETURN VALUE:
 *     Flh_AlgoReturn_t.
 *
 * FUNCTION DESCRIPTION:
 *    Debounces the fault or passes it through depending on EventStatus.
 *
 *---------------------------------------------------------------------------*/
static Flh_AlgoReturn_t Flh_DebounceEventCounterBased(Flh_EventId_t EventId, Flh_EventStatus_t EventStatus)
{
    Flh_AlgoReturn_t pass_fail = DEBOUNCE_FAULT_NO_ACTION;
    const Flh_EventId_t DebounceIdx = EventId;
    Flh_ConfigType const *const DebounceCfg = &Flh_ConfigData[DebounceIdx];
    Flh_DebounceCounterStatus_t *DebounceStatus = &Flh_DebounceCounterStatus[DebounceIdx];

    switch (EventStatus)
    {
    case FLH_EVENT_STATUS_PREPASSED:
        if (((DebounceCfg->LatchingFault == FLH_LATCH_PERMANENT) || (DebounceCfg->LatchingFault == FLH_LATCH_IGNITION)) && ((fault_status_bits[EventId] & FLH_FAULT_FAILED) == FLH_FAULT_FAILED))
        {
            ; // Do nothing
        }
        else
        {
            // jump-down enabled and counter value is greater than configured
            // jump-down value
            if ((DebounceCfg->JumpDown != FALSE) && (DebounceStatus->InternalCounter > DebounceCfg->JumpDownValue))
            {
                // jump down (and step down in action below)
                DebounceStatus->InternalCounter = DebounceCfg->JumpDownValue;
            }

            if (DebounceStatus->InternalCounter > (DebounceCfg->CounterPassedThreshold + (int16_t)DebounceCfg->CountDecStepSize))
            {
                // step down
                DebounceStatus->InternalCounter -= (int16_t)DebounceCfg->CountDecStepSize;
            }
            else
            {
                // step down to threshold: qualify event as passed
                DebounceStatus->InternalCounter = DebounceCfg->CounterPassedThreshold;
                if (((fault_status_bits[EventId] & 0xE0U) != 0U) || ((fault_status_bits[EventId] & FLH_FAULT_FAILED) == FLH_FAULT_FAILED))
                {
                    fault_status_bits[EventId] = FLH_FAULT_PASSED;
                    pass_fail = DEBOUNCE_FAULT_PASSED;
                }
            }
        }
        break;

    case FLH_EVENT_STATUS_PREFAILED:
        // jump-up enabled and counter value is less than configured
        // jump-up value
        if ((DebounceCfg->JumpUp != FALSE) && (DebounceStatus->InternalCounter < DebounceCfg->JumpUpValue))
        {
            // jump up (and step up in action below)
            DebounceStatus->InternalCounter = DebounceCfg->JumpUpValue;
        }

        if (DebounceStatus->InternalCounter < (DebounceCfg->CounterFailedThreshold - (int16_t)DebounceCfg->CountIncStepSize))
        {
            // step up
            DebounceStatus->InternalCounter += (int16_t)DebounceCfg->CountIncStepSize;
        }
        else
        {
            // step up to threshold: qualify event as failed
            DebounceStatus->InternalCounter = DebounceCfg->CounterFailedThreshold;
            if (((fault_status_bits[EventId] & 0xE0U) != 0U) || ((fault_status_bits[EventId] & FLH_FAULT_FAILED) == FLH_FAULT_PASSED))
            {
                fault_status_bits[EventId] = FLH_FAULT_FAILED;
                pass_fail = DEBOUNCE_FAULT_FAILED;
            }
        }
        break;

    case FLH_EVENT_STATUS_PASSED:
        if (((DebounceCfg->LatchingFault == FLH_LATCH_PERMANENT) || (DebounceCfg->LatchingFault == FLH_LATCH_IGNITION)) && ((fault_status_bits[EventId] & FLH_FAULT_FAILED) == FLH_FAULT_FAILED))
        {
            ; // Do nothing
        }
        else
        {
            DebounceStatus->InternalCounter = DebounceCfg->CounterPassedThreshold;
            if (((fault_status_bits[EventId] & 0xE0U) != 0U) || ((fault_status_bits[EventId] & FLH_FAULT_FAILED) == FLH_FAULT_FAILED))
            {
                fault_status_bits[EventId] = FLH_FAULT_PASSED;
                pass_fail = DEBOUNCE_FAULT_PASSED;
            }
        }
        break;

    default: // FLH_EVENT_STATUS_FAILED
        DebounceStatus->InternalCounter = DebounceCfg->CounterFailedThreshold;
        if (((fault_status_bits[EventId] & 0xE0U) != 0U) || ((fault_status_bits[EventId] & FLH_FAULT_FAILED) == FLH_FAULT_PASSED))
        {
            fault_status_bits[EventId] = FLH_FAULT_FAILED;
            pass_fail = DEBOUNCE_FAULT_FAILED;
        }
        break;
    }
    return pass_fail;
}