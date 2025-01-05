/*File Descpiption*/
/*CAN signal pretransmit update and CAN fault handling*/

/*****************************************************************************
                  Include files
*****************************************************************************/
#include "AcuBase_Types.h"

/*****************************************************************************
                  Local defined macros
*****************************************************************************/
#define CANSIG_CYCLE_MS 5U

/*****************************************************************************
                  Local types, enums definitions
*****************************************************************************/
typedef enum
{
    CANSIG_IDLE = ACUBASE_IDLE,
    CANSIG_INIT = ACUBASE_INIT,
    CANSIG_INIT_DONE = ACUBASE_INIT_DONE,
    CANSIG_PROVEOUT = ACUBASE_PROVEOUT,
    CANSIG_RUN = ACUBASE_RUN,
    CANSIG_DEINIT = ACUBASE_DEINIT,
    CANSIG_DISABLE_REQUEST = ACUBASE_DISABLE_REQUEST,
    CANSIG_DISABLED = ACUBASE_DISABLED
} CanSig_State_t;

typedef struct
{
    CanSig_State_t moduleState;
    uint8_t busoffMonitorTimer_u8;
    uint8_t timeoutCylicTimer_u8;
    uint8_t txFrameReferenceCounter_u8;
    CanSig_BusoffRecovery_t busoffRecovery;
    CanSig_timer_t wakeUpTimer;
} CanSig_status_t;

/*****************************************************************************
                  Local function prototypes
*****************************************************************************/
static void CanSig_KickTimers(void);

/*****************************************************************************
                  Local object definitions
*****************************************************************************/
static CanSig_status_t CanSig;

/*----------------------------------------------------------------------------
 *
 * FUNCTION NAME: CanSig_Cyclic()
 *
 * FUNCTION ARGUMENTS:
 *    None.
 *
 * RETURN VALUE:
 *    None.
 *
 * FUNCTION DESCRIPTION:
 *    CanSig_Cyclic function shall be called every 5ms.
 *---------------------------------------------------------------------------*/
void CanSig_Cyclic(void)
{
    CanSig_KickTimers();

    swtich(CanSig.moduleState)
    {
    case CANSIG_IDLE:
        /*do nothing*/
        break;
    case CANSIG_INIT:
        break;
    case CANSIG_INIT_DONE:
        break;
    case CANSIG_RUN:
    {
        boolean_t IsCanVoltageValid_bo = False;
        (void)PsmApp_IsValidCANVoltage(&IsCANVoltageValid_bo);

        if (False != IsCanVoltageValid_bo)
        {
            /* Monitor for Busoff and Node timeout DTCs setting condition */
            CanSig_ValidateSpeed();
            CanSig_UpdateSignalValidValues();
            CanSig_BusoffRecoveryProcessing();
            CanSig_NodeTimeoutMonitor();
            CanSig_MonitorElectricVehicleStartCondition();
        }
        else
        {
            if (CANSIG_CRASH == (uint8_t)Acn_GetCrashStatus())
            {
                /*dont stop CAN communication if crash is detected*/
            }
            else
            {
                /*disconnect the CAN controller from the network*/
                CanSig_SetControllerMode(CANSIG_DISCONNECT_CONTROLLER);
                (void)CanSig_SetApplFrameMode(CANSIG_APPL_TX_DISABLED_RX_DISABLED);
                CanSig.moduleState = CANSIG_DISABLED;
            }
        }
        break;
    }
    case CANSIG_DEINIT:
        break;
    case CANSIG_DISABLE_REQUEST:
        break;
    case CANSIG_DISABLED:
    {
        boolean_t IsCanVoltageValid_bo = FALSE;
        (void)PsmApp_IsValidCANVoltage(&IsCanVoltageValid_bo);
        CanSig_NodeTimeoutMonitor();
        if (FALSE != IsCanVoltageValid_bo)
        {
            /* connect the CAN controller from the network */
            CanSig_SetControllerMode(CANSIG_CONNECT_CONTROLLER);
            (void)CanSig_SetApplFrameMode(CANSIG_APPL_TX_ENABLED_RX_ENABLED);
            CanSig.moduleState = CANSIG_RUN;
        }
        break;
    }
    default:
        break;
    }
}

/*----------------------------------------------------------------------------
 *
 * FUNCTION NAME: CanSig_KickTimers()
 *
 * FUNCTION ARGUMENTS:
 *    None.
 *
 * RETURN VALUE:
 *    None.
 *
 * FUNCTION DESCRIPTION:
 *    Function must be placed in cyclic function with fixed interval. Process CanSig timers
 *---------------------------------------------------------------------------*/
static void CanSig_KickTimers(void)
{
    ProcessTimer(&CanSig.wakeUpTimer, CANSIG_CYCLE_MS);
    ProcessTimer(&CanSig.busoffRecovery.recoveryTimer, CANSIG_CYCLE_MS);
    ProcessTimer(&is1sAfterElectricVehicleStart.timer, CANSIG_CYCLE_MS);
}