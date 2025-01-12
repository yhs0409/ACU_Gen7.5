

/*****************************************************************************
                  Exported function prototypes
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
Std_ReturnType DEM_ApmDataElementRead(uint8_t *DataByte);