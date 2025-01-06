/*File description*/
/*this file supports PSM Application functionality*/

/***************************************************************************************************
                                   Function definitions
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
AcuBase_Return_t PsmApp_IsValidCANVoltage(bool_t *status_p)
{
    AcuBase_Return_t result = ACUBASE_NOT_OK;

    if (NULL != status_p)
    {
        *status_p = PsmAppData.PsmApp_IsValidCANVoltage_bo;
        result = ACUBASE_OK;
    }

    return result;
}