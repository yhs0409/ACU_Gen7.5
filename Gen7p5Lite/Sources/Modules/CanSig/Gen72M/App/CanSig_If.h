
/*****************************************************************************
                  Exported defined macros
*****************************************************************************/
#define SIGNAL_RCVD 0x00U

/*****************************************************************************
                  Exported function prototypes
*****************************************************************************/

/*----------------------------------------------------------------------------
 *
 * FUNCTION NAME: CanSig_SDM_RxSignalData()
 *
 * FUNCTION ARGUMENTS:
 *    None.
 *
 * RETURN VALUE:
 *    None.
 *
 * FUNCTION DESCRIPTION:
 *    Request Signal data and status.
 *
 *    Note: For Vehicle Speed and Date-Time data this API should not be called
 *---------------------------------------------------------------------------*/
uint8_t CanSig_SDM_RxSignalData(CanSig_SignalID_t Signal, void *SignalDataPtr);

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
 *    CanSig_Cyclic function
 *---------------------------------------------------------------------------*/
void CanSig_Cyclic(void);

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
boolean_t CanSig_IsBusOffPresent(void);
