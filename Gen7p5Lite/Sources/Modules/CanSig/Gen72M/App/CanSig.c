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
    TIMER_ELAPSED,
    TIMER_STOPPED,
    TIMER_RUNNING,
    TIMER_PAUSED
} CanSig_timer_state_t;

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

typedef enum
{
    BOR_IDLE,
    BOR_BUSOFF_DETECTED,
    BOR_TRY_TO_RECOVER,
    BOR_BUSOFF_RECOVERING,
    BOR_BUSOFF_RECOVERED
} CanSig_BOR_state_t;

typedef struct
{
    CanSig_timer_state_t state;
    uint16_t counter;
} CanSig_timer_t;

typedef struct
{
    CanSig_BOR_state_t state;
    volatile boolean_t isBusoffDetected;
    uint8_t busoffsCountLimit;
    CanSig_timer_t recoveryTimer;
    volatile boolean_t isBusoffOccur; /* TRUE: busoff occur, FALSE: Tx succeed */
    uint8_t TxEnable;
} CanSig_BussoffRecovery_t;

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

static void ProcessTimer(CanSig_timer_t *timer, uint8_t step);

static void CanSig_ValidateSpeed(void);
static boolean_t CanSig_IsSpeedValid(void);
/*****************************************************************************
                  Local object definitions
*****************************************************************************/
static CanSig_status_t CanSig;

static CanSig_1sAfterElectricVehicleStart_t is1sAfterElectricVehicleStart = {{TIMER_STOPPED, 0U}, FALSE};

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

/*----------------------------------------------------------------------------
 *
 * FUNCTION NAME: ProcessTimer
 *
 * FUNCTION ARGUMENTS:
 *    None
 *
 * RETURN VALUE:
 *    None
 *
 * FUNCTION DESCRIPTION AND RESTRICTIONS:
 *    Processing timer
 *
 *---------------------------------------------------------------------------*/
static void ProcessTimer(CanSig_timer_t *timer, uint8_t step)
{
    if (TIMER_RUNNING == timer->state)
    {
        if (timer->counter > 0U)
        {
            timer->counter -= step;
            if (timer->counter == 0U)
            {
                timer->state = TIMER_ELAPSED;
            }
        }
        else
        {
            timer->state = TIMER_ELAPSED;
        }
    }
}

/*----------------------------------------------------------------------------
 *
 * FUNCTION NAME: CanSig_IsSpeedValid
 *
 * FUNCTION ARGUMENTS:
 *    [IN/OUT] speed - pointer for returning speed value
 *
 * RETURN VALUE:
 *    retVal - speed validation E_OK - valid, E_NOT_OK - invalid
 *
 * FUNCTION DESCRIPTION AND RESTRICTIONS:
 *    Function returns valid speed value OR last validated speed value. In 2nd case function will return E_NOT_OK
 *
 *---------------------------------------------------------------------------*/
static boolean_t CanSig_IsSpeedValid(void)
{
    boolean_t retval_bo = FALSE;
    boolean_t canFrameValid = FALSE;

    if (FALSE == CanSig_IsBusOffPresent())
    {
        if (SIGNAL_RCVD == CanSig_SDM_RxSignalData(SGEspVehSpdVld, &canFrameValid))
        {
            if (FALSE != canFrameValid)
            {
                retVal_bo = FALSE;
            }
            else
            {
                retVal_bo = TRUE;
            }
        }
    }
}

/*----------------------------------------------------------------------------
 *
 * FUNCTION NAME: CanSig_ValidateSpeed
 *
 * FUNCTION ARGUMENTS:
 *    [IN/OUT] speed - pointer for returning speed value
 *
 * RETURN VALUE:
 *    retVal - speed validation E_OK - valid, E_NOT_OK - invalid
 *
 * FUNCTION DESCRIPTION AND RESTRICTIONS:
 *    Function returns valid speed value OR last validated speed value. In 2nd case function will return E_NOT_OK
 *
 *---------------------------------------------------------------------------*/
static void CanSig_ValidateSpeed(void)
{
    uint16_t canFrameSpeed;

    if (TRUE == CanSig_IsSpeedValid())
    {
        /* Signal SGVehSpdAvgDrvn was already validated by validating SGVehSpdAvgDrvnV in same frame */
        /* Signal SGVehSpdAvgDrvn was already validated by validating SGVehSpdAvgDrvnV in same frame */
        (void)CanSig_SDM_RxSignalData(SGEspVehSpd, &canFrameSpeed);
        CanSig_ValidatedSpeed.validatedSpeed = canFrameSpeed;
        CanSig_ValidatedSpeed.validatedRecalculatedSpeed = CanSig_RecalculateSpeed(canFrameSpeed);
        CanSig_ValidatedSpeed.isSpeedValid = TRUE;
    }
    else
    {
        CanSig_ValidatedSpeed.isSpeedValid = FALSE;
        CanSig_ValidatedSpeed.validatedSpeed = 0x00U;
    }
}