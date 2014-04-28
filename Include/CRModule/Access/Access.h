/*****************************************************************************
 *
 *              Access.h -- Access Control v0.1
 *
 *****************************************************************************
 *
 * Author:          Guillermo Jara Luengos
 * FileName:        Access.h
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
 * Este archivo implementa el control de acceso al repositorio.
 *
 * Change History:
 *  Rev   Date(m/d/y)   Description
 *  0.1   23/10/2012    Initial revision
 *****************************************************************************/
#ifndef ACCESS_H
#define ACCESS_H

#include "GenericTypeDefs.h"
#include "../CRModule.h"

#include "../NODE FW.X/CWSN LSI Node/Include/WirelessProtocols/ConfigApp.h"//"CRModule/HAL/ConfigHAL.h"
#include "CRModule/Access/ConfigAccess.h"

/*****************************************************************************/
/***************************DEFINICION DE TIPOS*******************************/
/*****************************************************************************/

#define Allowed 1
#define Nallowed 0

typedef enum _ACCCTRLACTION
{
    ActChckPerm = 0x01, ActAddEntry
} ACCCTRLACTION;

/*La estructura de los parametros del Mensaje con destino este modulo.*/
typedef struct _ACCCTRL_MSSG_RCVD
{
    INPUT BYTE OrgModule;
    INPUT ACCCTRLACTION Action;
    INPUT BYTE *DirecOrigen;
    INPUT SUB_MODULE Destino;
    INPUT BYTE Operation;
}ACCCTRL_MSSG_RCVD;

//Union de tamaño BYTE que puede definir los permisos para las acciones de uno
// de los sub-modulos cada vez. Si algún sub-modulo tuviera mas de ocho
//posibles acciones necesitariamos un WORD. Tambien podría separarlos para
//optimizar los tamaños de cada sub-modulo y que no haya sobredimensionamiento.
typedef union _PERMACT
{
    WORD Val;
    struct
    {
        WORD SignDetect:1;
        WORD ActvScan:1;
        WORD Services:1;
        WORD NtwksDiscovery:1;
    }DiscActionPerm;
    struct
    {
        WORD SleepMCU:1;
        WORD ResetMCU:1;
        WORD ChanHop:1;
        WORD AdjTrnscTxPowr:1;
        WORD ResetTrnsc:1;
        WORD PutTrnsc2Sleep:1;
        WORD WakeTrnsc:1;
        WORD TurnOffTrnsc:1;
    }ExecActionPerm;
    struct
    {
        WORD GameTheory:1;
        WORD ProccReqst:1;
    }OptActionPerm;
    struct
    {
        WORD StoreInRepo:1;
        WORD ReadFrmRepo:1;
    }RepActionPerm;
    struct
    {
        WORD ReadPermission:1;
        WORD WritePermission:1;
    }PolActionPerm;
    struct
    {
        WORD AddEntry:1;
        WORD CheckPermissions:1;
    }AccActionPerm;
}PERMACT;

//De esta forma definimos un tipo que es una estructura con los datos de los
//permisos para todas las acciones para cada uno de los modulos. Vamos que
//contiene la definicion de 5 elementos del tipo PERMACT, uno para cada
//sub-modulo.
typedef struct _ACCCTRL_PERM_REG
{
    PERMACT DiscPerm;
    PERMACT ExecPerm;
    PERMACT OptiPerm;
    PERMACT RepoPerm;
    PERMACT PoliPerm;
    PERMACT AccsPerm;
}ACCCTRL_PERM_REG;

//Y ahora el tipo a partir de estructura definitivo. Con campo para direccion
//y campo para un ACCCTRL_PERM_REG.
typedef struct _ENTRADA_PERM_REG
{
    BYTE Direc_EUI[MY_ADDRESS_LENGTH];
    ACCCTRL_PERM_REG Registro_Permisos;
}ENTRADA_PERM_REG;

//El array para los permisos de cada uno de los nodos.
ENTRADA_PERM_REG TABLA_PERM_ACC[MAX_NUMERO_PERM_NODOS];


/*****************************************************************************/
/*************************FIN DEFINICION DE TIPOS*****************************/
/*****************************************************************************/

/*****************************************************************************/
/*********************************VARIABLES***********************************/
/*****************************************************************************/

BOOL NodoCerrado; //Una variable para que el nodo niegue cualquier peticion
                    //del exterior por defecto.

/*****************************************************************************/
/********************************FIN VARIABLES********************************/
/*****************************************************************************/


/*****************************************************************************/
/*********************************FUNCIONES***********************************/
/*****************************************************************************/

/*De interfaz con messenger*/
BOOL CRM_AccCtrl_Mssg_Rcvd(ACCCTRL_MSSG_RCVD *Peticion);
/*Fin de funciones de interfaz con messenger*/

/*De funcionalidad del propio sub-modulo*/
BOOL CRM_AccCtrl_ChckPerm(ACCCTRL_MSSG_RCVD *Peticion);
BOOL CRM_AccCtrl_AddEntry(ACCCTRL_MSSG_RCVD *Peticion);
/*Fin de las funciones de funcionalidad propia*/

/*De inicialización del sub-modulo*/
BOOL CRM_AccCtrl_Init(void);
/*Fin de funciones de inicializacion del sub-modulo*/

/*****************************************************************************/
/******************************FIN DE FUNCIONES*******************************/
/*****************************************************************************/

#include "CRModule/Messenger/Messenger.h" //lo necesito porque cojo los nombres
                            // de las acciones de cada sub-modulo y como messng
                            //incluye todos pues terminamos antes.


#endif