/*****************************************************************************
 *
 *              Discovery.h -- Discovery v0.1
 *
 *****************************************************************************
 *
 * Author:          Guillermo Jara Luengos
 * FileName:
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
 * El header del Discovery.c
 *
 * Change History:
 *  Rev   Date(m/d/y)   Description
 *  0.1   23/10/2012    Initial revision
 *****************************************************************************/
#ifndef  _DISCOVERY_H_
#define  _DISCOVERY_H_

#include "GenericTypeDefs.h"
#include "../CRModule.h"
#include "WirelessProtocols/MCHP_API.h"
#include "../../CWSN LSI Node/Include/NodeHAL.h"
#include "../NODE FW.X/CWSN LSI Node/Include/WirelessProtocols/Console.h"//"../Include/GuilJa Utils Heads/MiWi Utils/GuilJa_MiWi_Utils.h"

#include "ConfigDiscovery.h"



/*****************************************************************************/
/***************************DEFINICION DE TIPOS*******************************/

/*****************************************************************************/

typedef enum _DISCACTION
{
    ActSignDetc = 0x01, ActActvScn, ActServs, ActNwks
} DISCACTION;

/*La estructura de los parametros del Mensaje con destino este modulo.*/
typedef struct _DISC_MSSG_RCVD
{
    INPUT BYTE OrgModule;
    INPUT DISCACTION Action;
    INPUT radioInterface Transceiver;
    INPUT void *Param1;
    INPUT void *Param2;
    INPUT void *Param3;
    OUTPUT void *Param4;
}DISC_MSSG_RCVD;

/*****************************************************************************/
/*************************FIN DEFINICION DE TIPOS*****************************/
/*****************************************************************************/


/*****************************************************************************/
/*********************************FUNCIONES***********************************/
/*****************************************************************************/

/*De recepcion/interfaz con Messenger*/
void* CRM_Disc_Mssg_Rcvd(DISC_MSSG_RCVD *Peticion);
/*Fin de funciones de recepcion/interfaz con Messenger*/

/*Propias del modulo*/
BYTE CRM_Disc_SignalDetection(DISC_MSSG_RCVD *Peticion);
BOOL CRM_Disc_ActiveScan(DISC_MSSG_RCVD *Peticion);
BOOL CRM_Disc_ChckTrnsc(DISC_MSSG_RCVD *Peticion);//XXX-Willy.Al optimizer??
BOOL CRM_Disc_Services(DISC_MSSG_RCVD *Peticion);
BOOL CRM_Disc_NwksDisc(DISC_MSSG_RCVD *Peticion);
/*Fin de funciones propias del modulo*/

/*Funciones de inicializacion*/
BOOL CRM_Disc_Init(void);
/*Fin de las funciones de inicializacion*/

/*Funciones propias de ejecucion*/
BOOL CRM_Disc_Int(void);
/*Fin de funciones propias de ejecucion*/

/*****************************************************************************/
/******************************FIN DE FUNCIONES*******************************/
/*****************************************************************************/


#endif
