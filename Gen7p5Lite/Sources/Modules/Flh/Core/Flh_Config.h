

/*******************************************************************************
    Flh event table
 *******************************************************************************/
typedef enum
{
#include "Flh_FlhFaults.inc"
    /* This must always be present!!! */
    FLH_NUMBER_OF_EVENTS,
    FLH_INVALID_EVENT_ID
} Flh_EventId_t;