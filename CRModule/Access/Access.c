/*****************************************************************************
 *
 *              Access.c -- Access Control v0.1
 *
 *****************************************************************************
 *
 * Author:          Guillermo Jara Luengos
 * FileName:        Access.c
 * Dependencies:    Access.h, ConfigAccess.h
 * Processor:
 * BOARD:
 * Compiler:        C32 01.00.02 or higher
 * Linker:          MPLINK 03.40.00 or higher
 * Company:         B105 -FAILURE IS NOT AN OPTION-
 *
 *****************************************************************************
 * File Description:
 *
 * Este archivo implementa el control de acceso.
 *
 * Change History:
 *  Rev   Date(m/d/y)   Description
 *  0.1   23/10/2012    Initial revision
 *****************************************************************************/
#include "CRModule/Access/Access.h"

//#include "CRModule/HAL/HAL.h"

/*****************************************************************************/
/*********************************VARIABLES***********************************/
/*****************************************************************************/

BYTE IndiceTablaPermisAcc; //Un indice para controlar la posicion de la tabla
                           //donde gardamos los permisos que tienen los nodos
                           //conocidos.

ACCCTRL_PERM_REG PermisosDefecto; //Para tener una configuracion por defecto
                                  //de registro de permisos.

/*****************************************************************************/
/********************************FIN VARIABLES********************************/
/*****************************************************************************/


/*****************************************************************************/
/*********************************FUNCIONES***********************************/
/*****************************************************************************/

/*De interfaz con messenger*/
BOOL CRM_AccCtrl_Mssg_Rcvd(ACCCTRL_MSSG_RCVD *Peticion)
{
    BOOL salida;
    switch(Peticion->Action)
    {
        case(ActChckPerm):
            salida = CRM_AccCtrl_ChckPerm(Peticion);
            break;
        case(ActAddEntry):
            CRM_AccCtrl_AddEntry(Peticion);
            break;
        default:
            break;
    }
    return salida;
}
/*Fin de funciones de interfaz con messenger*/

/*De funcionalidad del propio sub-modulo*/
BOOL CRM_AccCtrl_ChckPerm(ACCCTRL_MSSG_RCVD *Peticion)
{
    //TODO chequear permisos. Podemos buscarlo por el indice de la tabla
    //haciendo qeu sea el mismo que el de la tabla de conexiones, pero aun asi
    //comprobamos que la direccion larga coincide por si la tabla de conexiones
    // ha cambiado.
    if(NodoCerrado == TRUE)
        return Nallowed;
    else
    {
        BYTE i;
        for(i=0; i<IndiceTablaPermisAcc;i++)
        {
            if(isSameAddress(Peticion->DirecOrigen, TABLA_PERM_ACC[i].Direc_EUI))
            {
                switch (Peticion->Destino)
                {
                    case SubM_Disc:
                        switch(Peticion->Operation)
                        {
                            case ActSignDetc:
                                return TABLA_PERM_ACC[i].Registro_Permisos.DiscPerm.DiscActionPerm.SignDetect;
                                break;
                            case ActActvScn:
                                return TABLA_PERM_ACC[i].Registro_Permisos.DiscPerm.DiscActionPerm.ActvScan;
                                break;
                            case ActServs:
                                return TABLA_PERM_ACC[i].Registro_Permisos.DiscPerm.DiscActionPerm.Services;
                                break;
                            case ActNwks:
                                return TABLA_PERM_ACC[i].Registro_Permisos.DiscPerm.DiscActionPerm.NtwksDiscovery;
                                break;
                            default:
                                return Nallowed;
                                break;
                        }
                        break;
                    case SubM_Exec:
                        switch(Peticion->Operation)
                        {
                            case ActSleepMCU:
                                return TABLA_PERM_ACC[i].Registro_Permisos.ExecPerm.ExecActionPerm.SleepMCU;
                                break;
                            case ActResetMCU:
                                return TABLA_PERM_ACC[i].Registro_Permisos.ExecPerm.ExecActionPerm.ResetMCU;
                                break;
                            case ActChnHop:
                                return TABLA_PERM_ACC[i].Registro_Permisos.ExecPerm.ExecActionPerm.ChanHop;
                                break;
                            case ActTxPwr:
                                return TABLA_PERM_ACC[i].Registro_Permisos.ExecPerm.ExecActionPerm.AdjTrnscTxPowr;
                                break;
                            case ActTurnOn://ActReset:
                                return TABLA_PERM_ACC[i].Registro_Permisos.ExecPerm.ExecActionPerm.TurnOnTrnsc;//ResetTrnsc;
                                break;
                            case ActSleep:
                                return TABLA_PERM_ACC[i].Registro_Permisos.ExecPerm.ExecActionPerm.PutTrnsc2Sleep;
                                break;
                            case ActWake:
                                return TABLA_PERM_ACC[i].Registro_Permisos.ExecPerm.ExecActionPerm.WakeTrnsc;
                                break;
                            case ActTurnOff:
                                return TABLA_PERM_ACC[i].Registro_Permisos.ExecPerm.ExecActionPerm.TurnOffTrnsc;
                                break;
                            default:
                                return Nallowed;
                                break;
                        }
                        break;
                    case SubM_Opt:
                        switch(Peticion->Operation)
                        {
                            case ActGameTh:
                                return TABLA_PERM_ACC[i].Registro_Permisos.OptiPerm.OptActionPerm.GameTheory;
                                break;
                            case ActProcRq:
                                return TABLA_PERM_ACC[i].Registro_Permisos.OptiPerm.OptActionPerm.ProccReqst;
                            default:
                                return Nallowed;
                                break;
                        }
                        break;
                    case SubM_Poli:
                        switch(Peticion->Operation)
                        {
                            case ActRead:
                                return TABLA_PERM_ACC[i].Registro_Permisos.PoliPerm.PolActionPerm.ReadPermission;
                                break;
                            case ActWrite:
                                return TABLA_PERM_ACC[i].Registro_Permisos.PoliPerm.PolActionPerm.WritePermission;
                                break;
                            default:
                                return Nallowed;
                                break;
                        }
                        break;
                    case SubM_Repo:
                        switch(Peticion->Operation)
                        {
                            case ActStr:
                                return TABLA_PERM_ACC[i].Registro_Permisos.RepoPerm.RepActionPerm.StoreInRepo;
                                break;
                            case ActSndDta:
                                return TABLA_PERM_ACC[i].Registro_Permisos.RepoPerm.RepActionPerm.ReadFrmRepo;
                                break;
                            default:
                                return Nallowed;
                                break;
                        }
                        break;
                    case SubM_AccCtrl:
                        switch(Peticion->Operation)
                        {
                            case ActAddEntry:
                                return TABLA_PERM_ACC[i].Registro_Permisos.AccsPerm.AccActionPerm.AddEntry;
                                break;
                            case ActChckPerm:
                                return TABLA_PERM_ACC[i].Registro_Permisos.AccsPerm.AccActionPerm.CheckPermissions;
                                break;
                            default:
                                return Nallowed;
                                break;
                        }
                    default:
                        return Nallowed;
                        break;
                }
            }
            else
                return Nallowed;//No está en la lista
                //TODO podríamos añadirle a una blacklist o algo así por intetar
                //acceder sin estar en la lista.
        }
    }
    //TODO Comprobar si el nodo que solicita la peticion esta en nuestro
    //registro de permisos. Si esta buscar en el si tiene permitido
    //realizar la accion que solicita.
    return Nallowed;
}

BOOL CRM_AccCtrl_AddEntry(ACCCTRL_MSSG_RCVD *Peticion)
{
    BYTE i;
    for(i=0; i<IndiceTablaPermisAcc;i++)
    {
        if (isSameAddress(Peticion->DirecOrigen, TABLA_PERM_ACC[i].Direc_EUI))
        {
            //Ya está en la tabla.
            //TODO podiamos implementar una forma de cambiar los permisos.
            return FALSE;
        }
    }
    //Si no esta lo añadimos a la tabla de permisos.
    for(i = 0; i < MY_ADDRESS_LENGTH; i++)
    {
        TABLA_PERM_ACC[IndiceTablaPermisAcc].Direc_EUI[i] = *(Peticion->DirecOrigen + i);
    }
    TABLA_PERM_ACC[IndiceTablaPermisAcc].Registro_Permisos = PermisosDefecto;
    IndiceTablaPermisAcc += 1;
    return TRUE;
}
/*Fin de las funciones de funcionalidad propia*/

/*De inicialización del sub-modulo*/
BOOL CRM_AccCtrl_Init(void)
{
    NodoCerrado = TRUE; //Inicialmente negamos el acceso a cualquier nodo.
    IndiceTablaPermisAcc = 0; //Situamos el "puntero" de la tabla en 0.

    PermisosDefecto.DiscPerm.Val = 0xFFFF;
    PermisosDefecto.ExecPerm.Val = 0xFFFF;
    PermisosDefecto.OptiPerm.Val = 0xFFFF;
    PermisosDefecto.PoliPerm.Val = 0xFFFF;
    PermisosDefecto.RepoPerm.Val = 0xFFFF;
    PermisosDefecto.AccsPerm.Val = 0xFFFF;
    //TODO rutina de inicializacion.
    return TRUE;
}
/*Fin de funciones de inicializacion del sub-modulo*/

/*****************************************************************************/
/******************************FIN DE FUNCIONES*******************************/
/*****************************************************************************/
