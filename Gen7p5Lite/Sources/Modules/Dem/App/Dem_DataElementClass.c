/*File Description*/
/*functionality*/

/*****************************************************************************
                  Include files
*****************************************************************************/
#include "ApmAb_If.h"

/*****************************************************************************
                  Function definitions
*****************************************************************************/
/*----------------------------------------------------------------------------
 *
 * FUNCTION NAME: DEM_ApmDataElementRead callback for APM to
 *                fill in the data
 *
 * FUNCTION ARGUMENTS:
 *   DataByte  - pointer to an eight byte buffer to place the data
 *
 * RETURN VALUE:
 *   Returns E_OK on success
 * FUNCTION DESCRIPTION AND RESTRICTIONS:
 *
 * This function is a call back function to fill in data.
 *---------------------------------------------------------------------------*/
Std_ReturnType DEM_ApmDataElementRead(uint8_t *DataByte)
{
    uint8_t index_u8;
    /* Fill all data bytes with information from APM. */
    for (index_u8 = 0U; index_u8 < 8U; index_u8++)
    {
        DataByte[index_u8] = Apm_AbGetFaultDetails(index_u8);
    }
    return E_OK;
}