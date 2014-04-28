/*****************************************************************************
 *
 *              Messenger.c -- Messenger Module v0.1
 *
 *****************************************************************************
 *
 * Author:          Guillermo Jara Luengos
 * FileName:        Messenger.c
 * Dependencies:    Messenger.h
 * Processor:
 * BOARD:
 * Compiler:        C32 01.00.02 or higher
 * Linker:          MPLINK 03.40.00 or higher
 * Company:         B105 -FAILURE IS NOT AN OPTION-
 *
 *****************************************************************************
 * File Description:
 *
 * El modulo Messenger encargado de gestionar las peticiones entre modulos.
 *
 * Change History:
 *  Rev   Date(m/d/y)   Description
 *  0.1   23/10/2012    Initial revision
 *****************************************************************************/
#include "CRModule/Messenger/Messenger.h"

/*****************************************************************************/
/*********************************FUNCIONES***********************************/
/*****************************************************************************/

/*De recepcion y procesado*/
void* CRM_Message(MSSGPR Procc, INPUT SUB_MODULE DestSubModule, void *Peticion)
{
    //TODO el parametro de sub-modulo destino lo podiamos meter en la peticion.
    //Habria que meterlo en cada uno de los posibles tipos de peticion claro.
    //Pero la verdad es que se ven mas rapido hacia donde van dirigidos los
    //messages si se deja fuera.
    void* Salida; //TODO implementacion para que las funciones devuelvan su
                  //resultado sobre *Salida. Para eso tenemos que pasarlo como
                  //parametro.
    switch (Procc)
    {
        case VCC:
//            *((BOOL*)Salida) = CRM_Message_VCC(DestSubModule, Peticion);
            CRM_Message_VCC(DestSubModule, Peticion);
            break;
        case NMM:
            Salida = CRM_Message_NMM(DestSubModule, Peticion);
            break;
        default:
            break;
    }
    return Salida;
}

BOOL CRM_Message_VCC(INPUT SUB_MODULE DestSubModule, MSN_MSSG_RCVD *Peticion)
{
    BOOL salida = FALSE;
    ACCCTRL_MSSG_RCVD ComprobarPermiso;
    ComprobarPermiso.DirecOrigen = Peticion->DireccionEUI;
    ComprobarPermiso.Destino = DestSubModule;
    ComprobarPermiso.Action = ActChckPerm;
 //TODO HECHO tengo que poner tambien el campo operation para el access control.
    switch(DestSubModule)
    {
        case SubM_Disc: /*Mensaje entrante para Discovery.*/
            ComprobarPermiso.Operation = Peticion->Peticion_Destino.PeticionDisc->Action;
            if (CRM_Mssg_Send_AccCtrl(&ComprobarPermiso))
            {
                CRM_Mssg_Send_Disc(Peticion->Peticion_Destino.PeticionDisc);
            }
            break;
        case SubM_Exec: /*Mensaje entrante para Executor.*/
            ComprobarPermiso.Operation = Peticion->Peticion_Destino.PeticionExec->Action;
            if (CRM_Mssg_Send_AccCtrl(&ComprobarPermiso))
            {
                CRM_Mssg_Send_Exec(Peticion->Peticion_Destino.PeticionExec);
            }
            break;
        case SubM_Opt: /*Mensaje entrante para Optimizer.*/
             ComprobarPermiso.Operation = Peticion->Peticion_Destino.PeticionOptm->Action;
//#if !defined(TEST4) //Para que en el TEST4 no realice la comprobacion.
#if defined(TEST4) //Para que en el TEST4 si realice la comprobacion.
            if (CRM_Mssg_Send_AccCtrl(&ComprobarPermiso))
#endif
            {
                CRM_Mssg_Send_Optm(Peticion->Peticion_Destino.PeticionOptm);
            }
            break;
        case SubM_Repo: /*Mensaje entrante para Repostory.*/
            ComprobarPermiso.Operation = Peticion->Peticion_Destino.PeticionRepo->Action;
            if (CRM_Mssg_Send_AccCtrl(&ComprobarPermiso))
            {
                CRM_Mssg_Send_Repo(Peticion->Peticion_Destino.PeticionRepo);
            }
            break;
        case SubM_Poli: /*Mensaje entrante para Policies.*/
            ComprobarPermiso.Operation = Peticion->Peticion_Destino.PeticionPoli->Action;
            if (CRM_Mssg_Send_AccCtrl(&ComprobarPermiso))
            {
                CRM_Mssg_Send_Poli(Peticion->Peticion_Destino.PeticionPoli);
            }
            break;
        case SubM_AccCtrl: /*Mensaje entrante para Access Control.*/
            ComprobarPermiso.Operation = Peticion->Peticion_Destino.PeticionAcc->Action;
            if (CRM_Mssg_Send_AccCtrl(&ComprobarPermiso))
            {
                CRM_Mssg_Send_AccCtrl(Peticion->Peticion_Destino.PeticionAcc);
            }
            break;
        case SubM_Ext: /*Mensaje SALIENTE para un nodo externo.*/
            salida = CRM_Mssg_Send_VCC(Peticion->Peticion_Destino.PeticionVCC);
            break;
        default:
            break;
    //TODO procesador de Messages que proviene/van de/a otros nodos.HECHO
    }
    return salida;
}
void* CRM_Message_NMM(INPUT SUB_MODULE DestSubModule, void *Peticion)
{
    void *Salida;
    switch(DestSubModule)
    {
        case SubM_Disc:
        {
//            BYTE canal;
            Salida = CRM_Mssg_Send_Disc(Peticion);
//            Salida = &canal;
            break;
        }
        case SubM_Exec:
            CRM_Mssg_Send_Exec(Peticion);
            break;
        case SubM_Opt:
            CRM_Mssg_Send_Optm(Peticion);
            break;
        case SubM_Repo:
            CRM_Mssg_Send_Repo(Peticion);
            break;
        case SubM_Poli:
            CRM_Mssg_Send_Poli(Peticion);
            break;
        case SubM_AccCtrl:
            CRM_Mssg_Send_AccCtrl(Peticion);
            break;
        default:
            break;
    }
    return Salida;
}
/*Fin de funciones de recepcion y procesado*/

/*****************************************************************************/
/******************************FIN DE FUNCIONES*******************************/
/*****************************************************************************/
