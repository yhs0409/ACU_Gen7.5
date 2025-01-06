/*file description*/
/*this file is header file for template.c*/

/***************************************************************************************************
                                   Exported function prototypes
***************************************************************************************************/

/*--------------------------------------------------------------------------------------------------
*
* FUNCTION NAME: PsmApp_IsValidCANVoltage()
*
* FUNCTION ARGUMENTS:
*    bool_t *status_p
*
* RETURN VALUE:
*    AcuBase_Return_t
*
* FUNCTION DESCRIPTION:
*    This function provides whether the voltage is in valid range for CAN communication.
*
*-------------------------------------------------------------------------------------------------*/
AcuBase_Return_t PsmApp_IsValidCANVoltage(bool_t *status_p);