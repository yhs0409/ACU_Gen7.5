/*****************************************************************************
*                                                                            *
* Based on template: template.c rev. 4.1                                     *
*                                                                            *
* Copyright TRW Automotive U.S. LLC 2013, all rights reserved                *
*                                                                            *
* LOCATIONS:                                                                 *
*    TRW Automotive U.S. LLC, 24175 Research Drive,                          *
*    Farmington Hills, MI 48335-2642, USA                                    *
*                                                                            *
*    TRW Automotive GmbH, OSS / ESE                                          *
*    Fritz-Reichle-Ring 8, 78315 Radolfzell, Germany                         *
*                                                                            *
*    TRW Polska Sp. z o.o., Czestochowa Engineering Center                   *
*    ul. Rolnicza 33, 42-200 Czestochowa, Poland                             *
*                                                                            *
* PROPRIETARY:                                                               *
*    This code and all features are proprietary to TRW Automotive U.S. LLC   *
*    and may not be reproduced, modified or distributed in any manner        *
*    without the written permission of TRW Automotive U.S. LLC.              *
*                                                                            *
* PROJECT ID:                                                                *
*    PR11940 - Gen 7.2 Platform                                              *
*                                                                            *
* FILE DESCRIPTION:                                                          *
*    This file is the location for the initialization and idle task          *
*                                                                            *
******************************************************************************
*                                                                            *
*       Created By: Carsten Herrmann     Date: 2013-Jul-01                   *
*       Location responsible for maintenance: Radolfzell                     *
*                                                                            *
*                                                                            *
*****************************************************************************/

/*****************************************************************************
                  Include files
*****************************************************************************/
#include "TSAutosar.h"
#include "EcuM.h"

/*****************************************************************************
                  Local symbolic constants
*****************************************************************************/

/*****************************************************************************
                  Local types, enums definitions
*****************************************************************************/

/*****************************************************************************
                  Local function prototypes
*****************************************************************************/

/*****************************************************************************
                  Local object definitions
*****************************************************************************/

/*****************************************************************************
                  Exported object definitions
*****************************************************************************/

/*****************************************************************************
                  Local function-like macros
*****************************************************************************/

/*****************************************************************************
                  Local defined macros
*****************************************************************************/

/*****************************************************************************
                  Function definitions
*****************************************************************************/
/*----------------------------------------------------------------------------
*
* FUNCTION NAME: main(void)
*
* FUNCTION ARGUMENTS:
*    None
*
* RETURN VALUE:
*    None
*
* FUNCTION DESCRIPTION AND RESTRICTIONS:
*    This function serves as main
*
*---------------------------------------------------------------------------*/
int main(void)
{
    EcuM_Init();

    /* Never reached */
    return -1;
}

void ErrorHook(os_result_t res)
{
}
