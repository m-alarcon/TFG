/*****************************************************************************
 *
 *              Execution.h -- Execution Module v0.1
 *
 *****************************************************************************
 *
 * Author:          Guillermo Jara Luengos
 * FileName:        Execution.h
 * Dependencies:
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
#ifndef __EXECUTION_H_
#define __EXECUTION_H_

#include "GenericTypeDefs.h"
#include "../CRModule.h"

#include "../../CWSN LSI Node/Include/NodeHAL.h"
//#include "WirelessProtocols/MiWi/MiWi.h"
//#include "WirelessProtocols/MCHP_API.h"

#include "ConfigExecution.h"

/*****************************************************************************/
/***************************DEFINICION DE TIPOS*******************************/

/*****************************************************************************/

typedef enum _EXECACTION
{
    ActSleepMCU = 0x01, ActResetMCU, ActChnHop, ActTxPwr, ActTurnOn, ActSleep, ActWake, ActTurnOff, ActDisconn
} EXECACTION;//quito ActReset y pongo ActTurnOn

/*La estructura de los parametros del Mensaje con destino este modulo.*/
typedef struct _EXEC_MSSG_RCVD
{
    INPUT BYTE OrgModule;
    INPUT EXECACTION Action;
    INPUT radioInterface Transceiver;
    INPUT BYTE Action2; // Por si la accion es pedirle algo a discovery para
                        //indicar que accion queremos que haga Discovery.
    INPUT BYTE Param1;
    INPUT void *Param2;
    INPUT void *Param3;
    OUTPUT void *Param4;
}EXEC_MSSG_RCVD;

/*****************************************************************************/
/*************************FIN DEFINICION DE TIPOS*****************************/
/*****************************************************************************/


/*****************************************************************************/
/*********************************FUNCIONES***********************************/
/*****************************************************************************/

/*De interfaz con messenger*/
BOOL CRM_Exec_Mssg_Rcvd(EXEC_MSSG_RCVD *Peticion);
/*Fin de las funciones de interfaz con messenger*/

/*Para el propio MCU*/
BOOL CRM_Exec_SleepMCU(EXEC_MSSG_RCVD *Peticion);
BOOL CRM_Exec_ResetMCU(EXEC_MSSG_RCVD *Peticion);
/*Fin de funciones para le propio MCU*/

/*De funcionalidad propia del modulo*/
BOOL CRM_Exec_ChanHoping(EXEC_MSSG_RCVD *Peticion);
BOOL CRM_Exec_TxPower(EXEC_MSSG_RCVD *Peticion);
//BOOL CRM_Exec_Reset(EXEC_MSSG_RCVD *Peticion);
BOOL CRM_Exec_Sleep(EXEC_MSSG_RCVD *Peticion);
BOOL CRM_Exec_Wake(EXEC_MSSG_RCVD *Peticion);
BOOL CRM_Exec_TurnOff(EXEC_MSSG_RCVD *Peticion);
BOOL CRM_Exec_TurnOn(EXEC_MSSG_RCVD *Peticion);
BOOL CRM_Exec_DisconNode(EXEC_MSSG_RCVD *Peticion);
/*Fin de las funciones funcionalidad propia*/

/*****************************************************************************/
/******************************FIN DE FUNCIONES*******************************/
/*****************************************************************************/


#endif
