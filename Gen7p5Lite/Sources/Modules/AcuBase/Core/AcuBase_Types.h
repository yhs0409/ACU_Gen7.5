/*File Description*/
/*this file is header file for template.c*/

/***************************************************************************************************
                  Exported types, enums definitions
***************************************************************************************************/

/*--------------------------------------------------------------------------------------------------
* Enumeration defining the common high level states of the module.
*-------------------------------------------------------------------------------------------------*/
/* toDo : ACUBASE_ModuleState_e already defined via Rte, but currently not the following defs */
#define ACUBASE_IDLE                    0U
#define ACUBASE_INIT                    1U
#define ACUBASE_INIT_DONE               2U
#define ACUBASE_PROVEOUT                3U
#define ACUBASE_RUN                     4U
#define ACUBASE_DEINIT                  5U
#define ACUBASE_DISABLE_REQUEST         6U
#define ACUBASE_DISABLED                7U
#define ACUBASE_PRE_INIT                8U
#define ACUBASE_PRE_IDLE                9U