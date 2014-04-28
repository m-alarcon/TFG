/*****************************************************************************
 *
 *              Messenger.h -- Messenger Module v0.1
 *
 *****************************************************************************
 *
 * Author:          Guillermo Jara Luengos
 * FileName:        Messenger.h
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
 * Este archivo desarrolla las funciones y datos del modulo Messenger
 * (CR Module).
 *
 * Change History:
 *  Rev   Date(m/d/y)   Description
 *  0.1   23/10/2012    Initial revision
 *****************************************************************************/

#ifndef  _MESSENGER_H_
#define  _MESSENGER_H_

#include "CRModule/Discovery/Discovery.h"
#include "CRModule/Execution/Execution.h"
#include "CRModule/Optimizer/Optimizer.h"
#include "CRModule/Policies/Policies.h"
#include "CRModule/Repository/Repository.h"
#include "CRModule/Access/Access.h"
#include "CRModule/VCC/VCC.h"

/*****************************************************************************/
/***************************DEFINICION DE TIPOS*******************************/
/*****************************************************************************/

//Algunas estructuras de datos para los mensajes que van dirigidos hacia los
//diferentes módulos. En función del destino la estructura de datos contendrá
//diferentes parámetros. Lo dejamos aqui comentado para verlo aqui rapidamente
//pero esta definido en .h del modulo correspondiente.

    //typedef struct _DISC_MSSG_RCVD
    //{
    //    INPUT BYTE OrgModule;
    //    INPUT BYTE Action;
    //    INPUT BYTE Transceiver;
    //    INPUT BYTE Param1;
    //    INPUT void *Param2;
    //    OUTPUT void *Param3;
    //}DISC_MSSG_RCVD;

    //typedef struct _EXEC_MSSG_RCVD
    //{
    //    INPUT BYTE OrgModule;
    //    INPUT BYTE Action;
    //    INPUT BYTE Transceiver;
    //    INPUT BYTE Action2;
    //    INPUT BYTE Param1;
    //    INPUT void *Param2;
    //    OUTPUT void *Param3;
    //}EXEC_MSSG_RCVD;

    //typedef struct _OPTM_MSSG_RCVD
    //{
    //    INPUT BYTE OrgModule;
    //    INPUT BYTE Action;
    //    INPUT BYTE *Nodo;
    //}OPTM_MSSG_RCVD;

    //typedef struct _REPO_MSSG_RCVD
    //{
    //    INPUT BYTE OrgModule;
    //    INPUT BYTE Action;
    //    INPUT BYTE DataType;
    //    IOPUT void *Param1;
    //    IOPUT void *Param2;
    //}REPO_MSSG_RCVD;

    //typedef struct _POLI_MSSG_RCVD
    //{
    //    INPUT BYTE OrgModule;
    //    INPUT BYTE Action;
    //    INPUT BYTE DataType;
    //    INPUT void *Param1;
    //}POLI_MSSG_RCVD;

    //typedef struct _ACCCTRL_MSSG_RCVD
    //{
    //    INPUT BYTE OrgModule;
    //    INPUT BYTE Action;
    //    INPUT BYTE *DirecOrigen;
    //    INPUT BYTE Destino;
    //    INPUT BYTE Operation;
    //}ACCCTRL_MSSG_RCVD;


////Para definir los dos posibles origenes de mensajes, VCC y otro submodulo.
//typedef enum _MSSGPR {VCC, NMM} MSSGPR;

//Necesitamos tambien un estandar para los mensajes de messenger, si es
//mensaje entre sub-modulos no hace falta, directamente metemos como parametro
//la peticion con el formato del destino. Pero para los que vienen del exterior
//, es decir de otro nodo, necesitamos una coherencia y por eso definimos el
//tipo de dato para el messenger, pero ojo, repito, solo se tiene en cuenta
//para los mensajes que vienen de fuera.
/*La estructura de los parametros del Mensaje.*/
typedef struct _MSN_MSSG_RCVD
{
    INPUT BYTE *DireccionEUI; //REVISAR para todos transceiv.
    union _Peticion_Destino
    {
        DISC_MSSG_RCVD *PeticionDisc;
        EXEC_MSSG_RCVD *PeticionExec;
        OPTM_MSSG_RCVD *PeticionOptm;
        REPO_MSSG_RCVD *PeticionRepo;
        POLI_MSSG_RCVD *PeticionPoli;
        ACCCTRL_MSSG_RCVD *PeticionAcc;
        VCC_MSSG_RCVD *PeticionVCC;
    }Peticion_Destino;
}MSN_MSSG_RCVD;


/*****************************************************************************/
/*************************FIN DEFINICION DE TIPOS*****************************/
/*****************************************************************************/

/*****************************************************************************/
/*********************************FUNCIONES***********************************/
/*****************************************************************************/

/*De recepcion y procesado*/
void* CRM_Message(INPUT MSSGPR Origen, INPUT SUB_MODULE DestSubModule, void *Peticion);
BOOL CRM_Message_VCC(INPUT SUB_MODULE DestSubModule, MSN_MSSG_RCVD *Peticion);
void* CRM_Message_NMM(INPUT SUB_MODULE DestSubModule, void *Peticion);
/*Fin de funciones de recepcion y procesado*/

/*Definiciones de las funciones de envio que se corresponden con las propias
  de recepcion de cada modulo.*/
#define CRM_Mssg_Send_Disc(Peticion) CRM_Disc_Mssg_Rcvd(Peticion)
#define CRM_Mssg_Send_Exec(Peticion) CRM_Exec_Mssg_Rcvd(Peticion)
#define CRM_Mssg_Send_Optm(Peticion) CRM_Optm_Mssg_Rcvd(Peticion)
#define CRM_Mssg_Send_Repo(Peticion) CRM_Repo_Mssg_Rcvd(Peticion)
#define CRM_Mssg_Send_Poli(Peticion) CRM_Poli_Mssg_Rcvd(Peticion)
#define CRM_Mssg_Send_AccCtrl(Peticion) CRM_AccCtrl_Mssg_Rcvd(Peticion)
#define CRM_Mssg_Send_VCC(Peticion) CRM_VCC_Mssg_Rcvd(Peticion)
/*Fin de la defincion de las equivalencias de las funciones de envio*/

/*****************************************************************************/
/******************************FIN DE FUNCIONES*******************************/
/*****************************************************************************/

#endif

