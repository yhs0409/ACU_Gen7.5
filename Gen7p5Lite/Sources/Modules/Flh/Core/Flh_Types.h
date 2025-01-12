

/*******************************************************************************
                  Exported types, enums definitions
*******************************************************************************/
typedef enum
{
    FLH_EVENT_STATUS_PASSED = 0x00U         // DEM_EVENT_STATUS_PASSED,
    FLH_EVENT_STATUS_FAILED = 0x01U         // DEM_EVENT_STATUS_FAILED,
    FLH_EVENT_STATUS_PREPASSED = 0x02U      // DEM_EVENT_STATUS_PREPASSED,
    FLH_EVENT_STATUS_PREFAILED = 0x03U      // DEM_EVENT_STATUS_PREFAILED,
    MIN_SIZE_Flh_EventStatus_t = MAX_UINT16 /* Force generation of 16bit enum for faster access */
} Flh_EventStatus_t;

typedef enum
{
    DEBOUNCE_FAULT_NO_ACTION = 0U,
    DEBOUNCE_FAULT_PASSED = 1U,
    DEBOUNCE_FAULT_FAILED = 2U,
    MIN_SIZE_Flh_AlgoReturn_t = MAX_UINT16 /* Force generation of 16bit enum for faster access */
} Flh_AlgoReturn_t;

typedef Flh_AlgoReturn_t (*Flh_DebounceAlgorithm_t)(Flh_EventId_t EventId, Flh_EventStatus_t EventStatus);
typedef void (*Flh_EventCallback)(Flh_EventId_t FlhId, Dem_EventStatusType EventStatus);

typedef enum
{
    FLH_LATCH_NONE,
    FLH_LATCH_PERMANENT, /* Applications will have additional code to complete this */
    FLH_LATCH_IGNITION
} Flh_FaultLatchingType_t;

typedef struct
{
    uint16_t CountDecStepSize;      /* step size for decrementation (PREPASSED) */
    uint16_t CountIncStepSize;      /* step size for incrementation (PREFAILED) */
    int16_t CounterPassedThreshold; /* threshold for Passed status */
    int16_t CounterFailedThreshold; /* threshold for Failed status */
    int16_t JumpDownValue;          /* Jump-down value */
    int16_t JumpUpValue;            /* Jump-up value */
    uint8_t JumpDown;               /* Jump-down enabled? */
    uint8_t JumpUp;                 /* Jump-up enabled? */
    uint8_t AlgorithmIndex;
    uint8_t group_number_u8;
    Flh_FaultLatchingType_t LatchingFault;
    Flh_EventCallback EventCallback;
} Flh_ConfigType;

/*******************************************************************************
                  Exported object declarations
*******************************************************************************/
extern uint8_t Flh_InhibitFlags[((uint16_t)FLH_REPORT_DIRECT_TO_DEM_AFTER_THIS) + 1U];
typedef uint8_t FlhFaultFlagType;

extern FlhFaultFlagType fault_status_bits[((uint16_t)FLH_REPORT_DIRECT_TO_DEM_AFTER_THIS) + 1U];