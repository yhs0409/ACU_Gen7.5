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

/* Factors to rescaled raw CAN speed value to physical value with 1km/h resolution */
#define CAN_SPEED_RESOLUTION 5625U
/* CAN_SPEED_RESOLUTION_SCALING_FACTOR is already multiply by 100U - Factor to rescale 1km/h
 * to 0.01km/h. The reason was that used resolution is to big for 32bit value */
#define CAN_SPEED_RESOLUTION_SCALING_FACTOR 1000U

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
} CanSig_BusoffRecovery_t;

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

static void CanSig_BusoffRecoveryProcessing(void);
static void CanSig_KickTimers(void);

static void ProcessTimer(CanSig_timer_t *timer, uint8_t step);
static void StartTimer(CanSig_timer_t *timer, uint16_t limit);
static void CanSig_ValidateSpeed(void);
static uint16_t CanSig_RecalculateSpeed(uint16_t RawSpeedInput_u16);
static boolean_t CanSig_IsSpeedValid(void);

static void CanSig_UpdateSignalValidValues(void);
/*****************************************************************************
                  Local object definitions
*****************************************************************************/
static CanSig_status_t CanSig;

static CanSig_1sAfterElectricVehicleStart_t is1sAfterElectricVehicleStart = {{TIMER_STOPPED, 0U}, FALSE};

static uint8_t DateTimeDataRcvd_u8; // from DateTime Rx message
static uint32_t OdometerData_u32;   // from Odo Rx message

/*----------------------------------------------------------------------------
 *
 * FUNCTION NAME: CanSig_SDM_RxSignalData
 *
 * FUNCTION ARGUMENTS:
 *    None
 *
 * RETURN VALUE:
 *    None
 *
 * FUNCTION DESCRIPTION AND RESTRICTIONS:
 *    Interface function to propagate CAN signal to application layer components.
 *
 *    Note: For Vehicle Speed and Date-Time data this API should not be called
 *---------------------------------------------------------------------------*/
uint8_t CanSig_SDM_RxSignalData(CanSig_SignalID_t Signal, void *SignalDataPr)
{
    uint8_t retVal = SIGNAL_NA;
    uint8_t signalLength_u8;

    if (Signal < SGMaxNumOfSignals)
    {
        if (FALSE != ((uint8_t) * (SignalData[Signal].frameTimeoutFlag_u8) & ((uint8_t)((uint8_t)0x01U << SignalData[Signal].timeoutflag_position_u8))))
        {
            retVal = SIGNAL_NA;
        }
        else
        {
            retVal = SIGNAL_RCVD;
            SigRecOnceFlag[Signal] = TRUE;
        }

        (void)Com_ReceiveSignal((uint16_t)Signal, SignalDataPtr);

        /*if timeout of message is true, then update the default values for particular signals*/
        if (((SIGNAL_NA == retVal) && (*(SignalData[Signal].TimeoutCounter_u8) >= 3U)) || (TRUE != SigRecOnceFlag[Signal]))
        {
            if (signalLength_u8 <= 8U)
            {

                *(uint8_t *)SignalDataPtr = (uint8_t)0x00U;
            }
            else if ((9U <= signalLength_u8) && (signalLength_u8 <= 16U))
            {
                *(uint16_t *)SignalDataPtr = (uint16_t)0x00U;
            }
            else if ((17U <= signalLength_u8) && (signalLength_u8 <= 32U))
            {
                *(uint32_t *)SignalDataPtr = (uint32_t)0x00U;
            }
            else
            {
                /*From 33 bit signals to 64 bit*/
                *(uint64_t *)SignalDataPtr = (uint64_t)0x00U;
            }
            retVal = SIGNAL_TOUT;
            CanSig_GetDefaultSignalData(Signal, SignalDataPtr);
        }
    }
}

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
 * FUNCTION NAME: CanSig_BusoffRecoveryProcessing()
 *
 * FUNCTION ARGUMENTS:
 *    None.
 *
 * RETURN VALUE:
 *    None.
 *
 * FUNCTION DESCRIPTION:
 *    Process recovery from busoff
 *---------------------------------------------------------------------------*/
static void CanSig_BusoffRecoveryProcessing(void)
{
    Dem_EventStatusExtendedType IgnHighFaultStatus;
    Dem_EventStatusExtendedType IgnLowFaultStatus;
    static uint8 recoveryingcnt;

    switch (CanSig.busoffRecovery.state)
    {
    case BOR_IDLE:
        break;
    case BOR_BUSOFF_DETECTED:
        recoveryingcnt = 0U;
        /* Count short recovery retries. After 9 times of consecutive bus off states move to long recovery */
        if (BUS_OFF_QUICK_REC_LIMIT > CanSig.busoffRecovery.busoffsCountLimit)
        {
            CanSig.busoffRecovery.busoffsCountLimit++;
            StartTimer(&CanSig.busoffRecovery.recoveryTimer, BUS_OFF_DELAY_SHORT);
        }
        else
        {
            StartTimer(&CanSig.busoffRecovery.recoveryTimer, BUS_OFF_DELAY_LONG);
        }
        /*busoff DTC shall be performed */
        if (SHORT_RECOVERY_FOR_FAULT_REPORT <= CanSig.busoffRecovery.busoffsCountLimit)
        {
            Flh_ReportErrorStatus(Flh_CanBusOff_0, FLH_EVENT_STATUS_FAILED);
        }
        CanSig.busoffRecovery.state = BOR_TRY_TO_RECOVER;
        (void)CanIf_SetControllerMode(CONTROLLER_ID_0, CANIF_CS_STOPPED);
        break;
    case BOR_TRY_TO_RECOVER:
        if (TIMER_ELAPSED == GetTimerState(CanSig.busoffRecovery.recoveryTimer))
        {
            /*connect the CAN controller to network*/
            (void)CanIf_SetControllerMode(CONTROLLER_ID_0, CANIF_CS_STARTED);
            (void)CanSig_SetApplFrameMode(CANSIG_APPL_TX_ENABLED_RX_ENABLED);
            CanSig.busoffRecovery.state = BOR_BUSOFF_RECOVERING;
        }
        break;
    case BOR_BUSOFF_RECOVERING:
        /*    Count 10ms */
        if (recoveryingcnt == 2U)
        {
            CanSig.busoffRecovery.state = BOR_BUSOFF_RECOVERED;
        }
        else
        {
            recoveryingcnt++;
        }
        break;
    case BOR_BUSOFF_RECOVERED:
        recoveryingcnt = 0U;
        CanSig.busoffRecovery.isBusoffDetected = FALSE;
        if (FALSE == CanSig.busoffRecovery.isBusoffOccur)
        {
            CanSig.busoffRecovery.busoffsCountLimit = RESET;

            /* Get Ignition Low and High fault status */
            (void)Dem_GetEventStatus(DemConf_DemEventParameter_PmIgnPLow, &IgnLowFaultStatus);
            (void)Dem_GetEventStatus(DemConf_DemEventParameter_PmIgnPHigh, &IgnHighFaultStatus);
            if (((IgnLowFaultStatus & DEM_UDS_STATUS_TF) == RESET) && ((IgnHighFaultStatus & DEM_UDS_STATUS_TF) == RESET))
            {
                Flh_ReportErrorStatus(Flh_CanBusOff_0, FLH_EVENT_STATUS_PASSED);
                CanSig.busoffRecovery.state = BOR_IDLE;
            }
        }
        else
        {
        }
        break;
    default:
        break;
    }
}

/*----------------------------------------------------------------------------
 *
 * FUNCTION NAME: CanSig_IsBusOffPresent
 *
 * FUNCTION ARGUMENTS:
 *    None
 *
 * RETURN VALUE:
 *    retVal - busoff status
 *
 * FUNCTION DESCRIPTION AND RESTRICTIONS:
 *    Checking BusOff status
 *
 *---------------------------------------------------------------------------*/
boolean_t CanSig_IsBusOffPresent(void)
{
    return CanSig.busoffRecovery.isBusoffOcur;
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
        CanSig_ValidatedSpeed.validatedRecalculatedSpeed = CanSig_RecalculateSpeed(canFrameSpeed); // get true speed
        CanSig_ValidatedSpeed.isSpeedValid = TRUE;
    }
    else
    {
        CanSig_ValidatedSpeed.isSpeedValid = FALSE;
        CanSig_ValidatedSpeed.validatedSpeed = 0x00U;
    }
}

/*----------------------------------------------------------------------------
 *
 * FUNCTION NAME: CanSig_UpdateSignalValidValues
 *
 * FUNCTION ARGUMENTS:
 *    None
 *
 * RETURN VALUE:
 *    None
 *
 * FUNCTION DESCRIPTION AND RESTRICTIONS:
 *    Update signals values only if message is received
 *
 *---------------------------------------------------------------------------*/
static void CanSig_UpdateSignalValidValues(void)
{
    uint8_t SignalValue_u8;

    uint32_t SignalValue_u32;

    // boolean_t SignalValue_u2;

    /*update signals value only if message is received*/
    if (TRUE == DateTimeDataRcvd_u8)
    {
        if (SIGNAL_RCVD == CanSig_SDM_RxSignalData(SGHU_LocalTimeValid, &SignalValue_u8))
        {
            if ((uint8_t)FALSE == SignalValue_u8)
            {
                if (SIGNAL_RCVD == CanSig_SDM_RxSignalData(SGHU_LocalTimeDate, &SignalValue_u8))
                {
                    DateTimeData.CalendarDay_u8 = SignalValue_u8;
                }

                if (SIGNAL_RCVD == CanSig_SDM_RxSignalData(SGHU_LocalTimeMonth, &SignalValue_u8))
                {
                    DateTimeData.CalendarMonth_u8 = SignalValue_u8;
                }

                if (SIGNAL_RCVD == CanSig_SDM_RxSignalData(SGHU_LocalTimeHour, &SignalValue_u8))
                {
                    DateTimeData.HourOfDay_u8 = SignalValue_u8;
                }

                if (SIGNAL_RCVD == CanSig_SDM_RxSignalData(SGHU_LocalTimeMinute, &SignalValue_u8))
                {
                    DateTimeData.MinuteOfHour_u8 = SignalValue_u8;
                }

                if (SIGNAL_RCVD == CanSig_SDM_RxSignalData(SGHU_LocalTimeSecond, &SignalValue_u8))
                {
                    DateTimeData.SecsOfMinute_u8 = SignalValue_u8;
                }

                if (SIGNAL_RCVD == CanSig_SDM_RxSignalData(SGHU_LocalTimeYear, &SignalValue_u8))
                {
                    DateTimeData.CalendarYear_u8 = SignalValue_u8;
                }
            }
        }
    }

    /*get Odometerdata from CAN530*/
    if (TRUE == OdometerRcvd_u8)
    {
        if (SIGNAL_RCVD == CanSig_SDM_RxSignalData(SGIP_TotalOdometer, &SignalValue_u32))
        {
            OdometerData_u32 = SignalValue_u32;
        }
    }
}

/*----------------------------------------------------------------------------
 *
 * FUNCTION NAME: SBR_RecalculateSpeed()
 *
 * FUNCTION ARGUMENTS:
 *    none.
 *
 * RETURN VALUE:
 *    None.
 *
 * FUNCTION DESCRIPTION:
 * The function provides vehicle speed value with 0.01 km per count resolution
 *
 *---------------------------------------------------------------------------*/
static uint16_t CanSig_RecalculateSpeed(uint16_t RawSpeedInput_u16)
{
    uint16_t result = (uint16_t)(((uint32_t)RawSpeedInput_u16 * (uint32_t)CAN_SPEED_RESOLUTION) / ((uint32_t)CAN_SPEED_RESOLUTION_SCALING_FACTOR));

    return result;
}