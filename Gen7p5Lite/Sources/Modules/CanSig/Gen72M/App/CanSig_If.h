
/*****************************************************************************
                  Exported defined macros
*****************************************************************************/
#define SIGNAL_RCVD         0x00U

/*****************************************************************************
                  Exported function prototypes
*****************************************************************************/

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
