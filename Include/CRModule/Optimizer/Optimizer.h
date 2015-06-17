/*****************************************************************************
 *
 *              Optimizer.h -- Optimizer Module v0.1
 *
 *****************************************************************************
 *
 * Author:          Guillermo Jara Luengos
 * FileName:        Optimizer.h
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
 * Este archivo desarrolla las funciones y datos del modulo Optimizer
 * (CR Module).
 *
 * Change History:
 *  Rev   Date(m/d/y)   Description
 *  0.1   23/10/2012    Initial revision
 *****************************************************************************/

#ifndef  _OPTIMIZER_H_
#define  _OPTIMIZER_H_

#include "GenericTypeDefs.h"
#include "../CRModule.h"

#include "WirelessProtocols/MCHP_API.h" //Para disponer de las funciones y tal.
#include "../NODE FW.X/CWSN LSI Node/Include/NodeHAL.h"

#include "CRModule/Discovery/Discovery.h"
#include "CRModule/Execution/Execution.h"

#include "ConfigOptimizer.h"

#include "../../Aplicacion.h"

/*****************************************************************************/
/***************************DEFINICION DE TIPOS*******************************/
/*****************************************************************************/

typedef enum _OPTACTION
{
    ActGameTh = 0x01, ActCons = 0x02, ActProcRq
} OPTACTION;

typedef enum _OPTSUBACTION
{
    SubActChngCost = 0x01, SubActCambio = 0x02, SubActProcInfo
} OPTSUBACTION;

typedef enum _OPTPROCACTION
{
    ProcAsk4Chng = 0x01, ProcChangAnsw, ProcCambioCanal
} OPTPROCACTION;


typedef enum _TASKPARAM
{
    TaskParamTx = 0x01, Update
} TASKPARAM; /*La funcion de
                     CRM_Optm_Task puede hacer varias cosas y le pasaremos un
                     parametro para que realice operaciones concretas que nos
                     interesen.*/

typedef enum _OPTSTATMACH
{
    Idle = 0x01, WaitinAnsw4ChngChn
} OPTSTATMACH;

typedef enum _OPTGTSTATE
{
    EsperandoDecisionRestoNodos = 0x01, ComunicarDecFinal
} OPTGTSTATE;

/*La estructura de los parametros del Mensaje con destino este modulo.*/
typedef struct _OPTM_MSSG_RCVD
{
    INPUT BYTE OrgModule;
    INPUT OPTACTION Action;
    IOPUT void *Param1;
    IOPUT void *Param2;
    INPUT radioInterface Transceiver;
    INPUT BYTE *EUINodo; //XXX-Willy. Esta bien para identificarlo no?

}OPTM_MSSG_RCVD;

#include "CRModule/Messenger/Messenger.h"

/*****************************************************************************/
/*************************FIN DEFINICION DE TIPOS*****************************/
/*****************************************************************************/

/*****************************************************************************/
/*****************DEFINICION DE CONSTANTES Y VARIABLES************************/
/*****************************************************************************/
#define MAX_VECTOR_POTENCIA     5
#define UMBRAL_POTENCIA         11

BYTE MSSG_PROC_OPTM;

/*****************************************************************************/
/****************FIN DEFINICION DE CONSTANTES Y VARIABLES*********************/
/*****************************************************************************/

/*****************************************************************************/
/*********************************FUNCIONES***********************************/
/*****************************************************************************/

/*De recepcion/interfaz con Messenger*/
BOOL CRM_Optm_Mssg_Rcvd(OPTM_MSSG_RCVD *Peticion);
/*Fin de funciones de recepcion/interfaz con Messenger*/

/*De funcionalidad propia del sub-modulo*/
BOOL CRM_Optm_Processor(OPTM_MSSG_RCVD *Peticion);
BOOL CRM_Optm_GameTheory(OPTM_MSSG_RCVD *Peticion);
/*Fin de funciones de funcionalidad propia del sub-modulo*/

/*Sub-metodos de funciones*/
BOOL CRM_Optm_CostAlgorithm(INPUT BYTE CosteNotChanging, INPUT BYTE CosteOcup, INPUT BYTE CosteChngChann, INPUT WORD ProbCambioY);
BOOL CRM_Optm_Config(INPUT OPTACTION SubAccion, INPUT BYTE ConfigCosteTx, INPUT BYTE ConfigCosteRx,
        INPUT BYTE ConfigCosteSensing,INPUT BYTE ConfigmaxRTx, INPUT BYTE ConfignumMsj, INPUT WORD ConfigProbY);
/*Fin de sub-metodos de funciones*/

/*Funciones de inicializacion*/
BOOL CRM_Optm_Init(void);
/*Fin de las funciones de inicializacion*/

BOOL CRM_Optm_Calcular_Costes(BYTE n_rtx);
BOOL CRM_Optm_Cons(OPTM_MSSG_RCVD *Peticion);

/*Rutina de ejecucion del optimizer*/
BOOL CRM_Optm_Int(void);
/*Fin de rutina de ejecucion*/

/*Rutina para facilitar la realizacion de TESTs*/
//BORRABLE
BYTE Test2(void);
BYTE Test4(void);
/*Fin de la rutina para los tests*/

/*****************************************************************************/
/******************************FIN DE FUNCIONES*******************************/
/*****************************************************************************/



#endif

