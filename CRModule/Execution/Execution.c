/*****************************************************************************
 *
 *              Execution.c -- Execution Module v0.1
 *
 *****************************************************************************
 *
 * Author:          Guillermo Jara Luengos
 * FileName:        Execution.c
 * Dependencies:    Execution.h
 * Processor:
 * BOARD:
 * Compiler:        C32 01.00.02 or higher
 * Linker:          MPLINK 03.40.00 or higher
 * Company:         B105 -FAILURE IS NOT AN OPTION-
 *
 *****************************************************************************
 * File Description:
 *
 * Implementa las funciones de ejecucion que llevara a cabo en el nodo basadas
 * en las decisiones tomadas por el optimizador.
 *
 * Change History:
 *  Rev   Date(m/d/y)   Description
 *  0.1   23/10/2012    Initial revision
 *****************************************************************************/
#include "CRModule/Execution/Execution.h"
#include "CRModule/Execution/ConfigExecution.h"

/*****************************************************************************/
/*********************************FUNCIONES***********************************/
/*****************************************************************************/

/*De interfaz con messenger*/
BOOL CRM_Exec_Mssg_Rcvd(EXEC_MSSG_RCVD *Peticion)
{
    //TODO añadir todas las opciones.
    switch((*Peticion).Action)
    {
        case(ActSleepMCU):
            CRM_Exec_SleepMCU(Peticion);
            break;
        case(ActResetMCU):
            CRM_Exec_ResetMCU(Peticion);
            break;
        case(ActChnHop):
            CRM_Exec_ChanHoping(Peticion);
            break;
        case(ActTxPwr):
            CRM_Exec_TxPower(Peticion);
            break;
        case(ActReset):
            CRM_Exec_Reset(Peticion);
            break;
        case(ActSleep):
            CRM_Exec_Sleep(Peticion);
            break;
        case(ActWake):
            CRM_Exec_Wake(Peticion);
            break;
        case(ActTurnOff):
            CRM_Exec_TurnOff(Peticion);
            break;
        default:
            break;
    }
    return TRUE;
}
/*Fin de las funciones de interfaz con messenger*/

/*Para el propio MCU*/
BOOL CRM_Exec_SleepMCU(EXEC_MSSG_RCVD *Peticion)
{
    //TODO dormir MCU. HAL?
    return TRUE;
}
BOOL CRM_Exec_ResetMCU(EXEC_MSSG_RCVD *Peticion)
{
    //TODO resetear MCU. HAL?
    return TRUE;
}
/*Fin de funciones para le propio MCU*/

/*De funcionalidad propia del modulo*/
BOOL CRM_Exec_ChanHoping(EXEC_MSSG_RCVD *Peticion)
{
    //TEST1-BORRABLE
    #if defined(TEST1)
        OUTPUT BYTE valor = 44;
        (*Peticion).Param4 = &valor;
    #endif
    //Fin de TEST1
    SetChannel(Peticion->Transceiver, Peticion->Param1);//CRM_HAL_ChanHop(Peticion->Param1);/*Esta es la custom que no avisa.*/
//    StartChannelHopping(Peticion->Param1);/*Este ya hace broadcast avisando del
//                                           cambio de canal y los que reciben el
//                                           paquete de notificacion de cambio de
//                                           canal se cambian automaticamente. Lo
//                                            que no siempre puede interesar.*/
    return TRUE;
}
BOOL CRM_Exec_TxPower(EXEC_MSSG_RCVD *Peticion)
{
    //TODO controlar la potencia de transmision. HAL
    return TRUE;
}
BOOL CRM_Exec_Reset(EXEC_MSSG_RCVD *Peticion)
{
    //TODO resetear transceiver. HAL
    return TRUE;
}
BOOL CRM_Exec_Sleep(EXEC_MSSG_RCVD *Peticion)
{
    ///TODO dormir transceiver. HAL
    return TRUE;
}
BOOL CRM_Exec_Wake(EXEC_MSSG_RCVD *Peticion)
{
    //TODO despertar transceiver. HAL
    return TRUE;
}
BOOL CRM_Exec_TurnOff(EXEC_MSSG_RCVD *Peticion)
{
    //TODO apagar transceptor. HAL
     return TRUE;
}
/*Fin de funciones de funcionalidad propia del sub-modulo*/

/*****************************************************************************/
/******************************FIN DE FUNCIONES*******************************/
/*****************************************************************************/
