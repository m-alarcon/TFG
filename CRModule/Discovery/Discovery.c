/*****************************************************************************
 *
 *              Discovery.c -- Discovery Module v0.1
 *
 *****************************************************************************
 *
 * Author:          Guillermo Jara Luengos
 * FileName:        Discovery.c
 * Dependencies:    Discovery.h, ConfigDiscovery.h
 * Processor:
 * BOARD:
 * Compiler:        C32 01.00.02 or higher
 * Linker:          MPLINK 03.40.00 or higher
 * Company:         B105 -FAILURE IS NOT AN OPTION-
 *
 *****************************************************************************
 * File Description:
 *
 * Este archivo se ocupa de las funciones propias del sensado del entorno.
 *
 * Change History:
 *  Rev   Date(m/d/y)   Description
 *  0.1   23/10/2012    Initial revision
 *****************************************************************************/
#include "CRModule/Discovery/Discovery.h"

/*****************************************************************************/
/*********************************VARIABLES***********************************/
/*****************************************************************************/
//Para configurar cada cuanto tiempo realiza el escaneo automatico el
//sub-modulo Discovery. Si vale 0 no realizara escaneo automatico (solo bajo
//demanda.
WORD AutoDisc; //En segundos. //TODO o podemos ponerlo tambien en decenas de
                //segundo y nos podria valer con un byte.

/*Variables para configurar las aplicaciones de MiWi*/
DWORD DiscChnMAP;
BYTE DiscScanDuration;
BYTE DiscDetctMode;
/*Fin de las variables para configurar MiWi.*/
/*****************************************************************************/
/******************************FIN DE VARIABLES*******************************/
/*****************************************************************************/


/*****************************************************************************/
/*********************************FUNCIONES***********************************/
/*****************************************************************************/

/*****************************************************************************/
/*De recepcion/interfaz con Messenger*/ /*En principio devolvía void* porque ahí
                                         almacenaba el canal del escaneo pero eso
                                         daba error en tiempo de ejecución y además
                                         tenía el param4 para almacenarlo.REVISAR*/
void* CRM_Disc_Mssg_Rcvd(DISC_MSSG_RCVD *Peticion)
{
    void* resultado;
    BYTE canal;
    switch((*Peticion).Action)
    {
        case(ActSignDetc):
            canal = CRM_Disc_SignalDetection(Peticion);
//            CRM_Disc_SignalDetection(Peticion);
            resultado = &canal;
            break;
        case(ActActvScn):
            CRM_Disc_ActiveScan(Peticion);
            break;
        case(ActServs):
            CRM_Disc_Services(Peticion);
            break;
        case(ActNwks):
            CRM_Disc_NwksDisc(Peticion);
            break;
        default:
            break;
    }
    return resultado;
}
/*Fin de funciones de recepcion/interfaz con Messenger*/
/*****************************************************************************/

/*Propias del modulo*/
BYTE CRM_Disc_SignalDetection(DISC_MSSG_RCVD *Peticion)
{
    #if defined(TEST1)
        OUTPUT BYTE valor = 44;
        (*Peticion).Param4 = &valor;
    #endif
    BYTE CanalMenosRuidoso;
    BYTE BackupCurrentChannel = GetOpChannel(Peticion->Transceiver);
    CanalMenosRuidoso = DoChannelScanning(Peticion->Transceiver, Peticion->Param4);//MiApp_NoiseDetection(*((DWORD*)(Peticion->Param1)), *((BYTE*)(Peticion->Param2)), *((BYTE*)(Peticion->Param3)), ((BYTE*)(Peticion->Param4)), *((miwi_band*)(banda_miwi)));
    SetChannel(Peticion->Transceiver, BackupCurrentChannel);
    return CanalMenosRuidoso;
}
BOOL CRM_Disc_ActiveScan(DISC_MSSG_RCVD *Peticion)
{
    //MiApp_SearchConnection(*((BYTE*)(Peticion->Param2)), *((DWORD*)(Peticion->Param1)));
    PerformActiveScan(Peticion->Transceiver, *((BYTE*)(Peticion->Param2)), *((DWORD*)(Peticion->Param1)));
    //TODO llamadas a funciones de la HAL.
    return TRUE;//TODO devolver apropiadamnete TRUE o FALSE.
}
BOOL CRM_Disc_ChckTrnsc(DISC_MSSG_RCVD *Peticion)//XXX-Willy.Al optimizer??
{
    //TODO llamadas a funciones de la HAL.
    return TRUE;//TODO devolver apropiadamnete TRUE o FALSE.
}
BOOL CRM_Disc_Services(DISC_MSSG_RCVD *Peticion)
{
    //TODO llamadas a funciones de la HAL.
    return TRUE;//TODO devolver apropiadamnete TRUE o FALSE.
}
BOOL CRM_Disc_NwksDisc(DISC_MSSG_RCVD *Peticion)
{
    //TODO llamadas a funciones de la HAL.
    return TRUE;//TODO devolver apropiadamnete TRUE o FALSE.
}
/*Fin de funciones propias del modulo*/

/*Funciones de inicializacion*/
BOOL CRM_Disc_Init(void)
{
    AutoDisc = DfaultAutoDiscTime;
    DiscChnMAP = DfaultDiscChnMap;
    DiscScanDuration = DfaultDiscScnDura;
    DiscDetctMode = DfaultDiscDetctMode;
    return TRUE;//TODO devolver apropiadamnete TRUE o FALSE.
}
/*Fin de las funciones de inicializacion*/

/*Funciones propias de ejecucion*/
BOOL CRM_Disc_Int(void)
{
    //TODO la rutina de ejecucion de sensado que se configura para que se
    //ejecute automaticamente.
    return TRUE;//TODO devolver apropiadamnete TRUE o FALSE.
}
/*Fin de funciones propias de ejecucion*/

/*****************************************************************************/
/******************************FIN DE FUNCIONES*******************************/
/*****************************************************************************/
