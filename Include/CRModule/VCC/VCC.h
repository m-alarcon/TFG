/*****************************************************************************
 *
 *              VCC.h -- VCC v0.1
 *
 *****************************************************************************
 *
 * Author:          Guillermo Jara Luengos
 * FileName:        VCC.h, GenericTypeDefs.h, GuilJaTypes.h,
 *                  GuilJa_MiWi_Utils.h, ConfigVCC.h.
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
 * El header del VCC.c
 *
 * Change History:
 *  Rev   Date(m/d/y)   	Description
 *  0.1   February 6, 2013, 12:34 PM    Initial revision
 *****************************************************************************/
#ifndef  _VCC_H_
#define  _VCC_H_

#include "GenericTypeDefs.h"
#include "../CRModule.h"
#include "../../CWSN LSI Node/Include/NodeHAL.h"
#include "ConfigVCC.h"

/*****************************************************************************/
/*******************************DEFINES UTILES********************************/
/*****************************************************************************/


/*****************************************************************************/
/*****************************FIN DEFINES UTILES******************************/
/*****************************************************************************/


/*****************************************************************************/
/***************************DEFINICION DE TIPOS*******************************/
/*****************************************************************************/

//Las acciones que puede hacer el VCC, enviar y recibir.
typedef enum _VCCACTION
{
    ActSend = 0x01, ActRecv
} VCCACTION;

//El tipo de mensaje que ha recibido/envia. Si es de control con cabecera 0x01
typedef enum _VCCMSSGTYPE
{
    VccCtrlMssg = 0x01
} VCCMSSGTYPE;

//El tipo de mensaje que ha recibido/envia. Si es de control con cabecera 0x01 /////MODIFICADO malarcon
typedef enum _VCCMSSGPARSING
{
    CtrlMssgField = 0x00, SubMDestField = 0x01, SubMDestActField, SubMDestParam1Field, SubMDestParam2Field, SubMDestParam3Field, SubMDestParam4Field, SubMDestParamTransceiver
} VCCMSSPARSING;


/*La estructura de los parametros del Mensaje con destino este modulo.*/
typedef struct _VCC_MSSG_RCVD
{
    INPUT BYTE *DirNodDest;
    INPUT VCCACTION Action;
    INPUT BYTE AddrMode;
    INPUT void *BufferVCC;
    INPUT radioInterface Transceiver;
    INPUT void *Param1;/*Por ejemplo para decirle el tamaño del envio o de la
                        recepcion. O indicarle la estructura del mensaje que
                        queremos enviar... lo que necesitemos.*/
}VCC_MSSG_RCVD;

#include "CRModule/Optimizer/Optimizer.h"

/*****************************************************************************/
/*************************FIN DEFINICION DE TIPOS*****************************/
/*****************************************************************************/


/*****************************************************************************/
/*********************************VARIABLES***********************************/
/*****************************************************************************/

BOOL CtrlMssgFlag;
BOOL PendingMssg4Send;

/*****************************************************************************/
/*******************************FIN VARIBALES*********************************/
/*****************************************************************************/


/*****************************************************************************/
/*********************************FUNCIONES***********************************/
/*****************************************************************************/

/*De recepcion/interfaz con Messenger*/
BOOL CRM_VCC_Mssg_Rcvd(VCC_MSSG_RCVD *Peticion);
/*Fin de funciones de recepcion/interfaz con Messenger*/

/*De funcionalidad propia del sub-modulo*/
BOOL CRM_VCC_Reciever(VCC_MSSG_RCVD *Peticion);
BOOL CRM_VCC_Sender(VCC_MSSG_RCVD *Peticion);
/*Fin de funciones de funcionalidad propia del sub-modulo*/

//Funciones de funcionalidad interna del sub-modulo.*/
//BOOL CRM_VCC_MssgCreator(INPUT BYTE *Buffer);
BOOL CRM_VCC_MssgCreator(INPUT RECEIVED_MESSAGE *Buffer);
//Fin de funciones de funcionalidad interna del sub-modulo.*/


/*****************************************************************************/
/******************************FIN DE FUNCIONES*******************************/
/*****************************************************************************/


#endif