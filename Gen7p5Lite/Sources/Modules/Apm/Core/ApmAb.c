/*File description*/
/*this file contains functionality of APM(Application Mode Manager)/

/***************************************************************************************************
                  Function definitions
***************************************************************************************************/
/*--------------------------------------------------------------------------------------------------
 *
 * FUNCTION NAME: Apm_AbGetFaultDetails()
 *
 * FUNCTION ARGUMENTS: uint8_t - requested byte
 *
 * RETURN VALUE: uint8_t - requested byte value
 *
 * FUNCTION DESCRIPTION AND RESTRICTIONS:
 *    This function is used to get extended information about AB_TRANSITION_FAULT.
 *-------------------------------------------------------------------------------------------------*/
uint8_t Apm_AbGetFaultDetails(uint8_t byteNumber_u8)
{
    uint8_t data_u8;
    if (byteNumber_u8 < APM_DEM_DATA_SIZE)
    {
        data_u8 = Apm_AbFaultDetails_au8[byteNumber_u8];
    }
    else
    {
        data_u8 = 0U;
    }
    return data_u8;
}