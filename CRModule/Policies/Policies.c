/*****************************************************************************
 *
 *              Policies.c -- Policies Module v0.1
 *
 *****************************************************************************
 *
 * Author:          Guillermo Jara Luengos
 * FileName:        Policies.c
 * Dependencies:    Policies.h
 * Processor:       
 * BOARD:           
 * Compiler:        C32 01.00.02 or higher
 * Linker:          MPLINK 03.40.00 or higher
 * Company:         B105 -FAILURE IS NOT AN OPTION-
 *
 *****************************************************************************
 * File Description:
 *
 * Este archivo implementa el modulo correspondiente a las politicas del
 * modulo cognitivo (CR Module).
 *
 * Change History:
 *  Rev   Date(m/d/y)   Description
 *  0.1   23/10/2012    Initial revision
 *****************************************************************************/
#include "CRModule/Policies/Policies.h"
#include "CRModule/Policies/ConfigPolicies.h"

/*****************************************************************************/
/*********************************VARIABLES***********************************/
/*****************************************************************************/

POLI_WEIGHTS Reparto_Pesos;

/*****************************************************************************/
/******************************FIN DE VARIABLES*******************************/
/*****************************************************************************/


/*****************************************************************************/
/*********************************FUNCIONES***********************************/
/*****************************************************************************/

/*De interfaz con messenger*/
BOOL CRM_Poli_Mssg_Rcvd(POLI_MSSG_RCVD *Peticion)
{
    switch((*Peticion).Action)
    {
        case(ActRead):
            CRM_Poli_Read(Peticion);
            break;
        case(ActWrite):
            CRM_Poli_Write(Peticion);
            break;
        default:
            break;
    }
    return TRUE;
}
/*Fin de funciones de interfaz con messenger*/

/*Propias del sub-modulo*/
BOOL CRM_Poli_Read(POLI_MSSG_RCVD *Peticion)
{
    switch(Peticion->DataType)
    {
        case TypePwrCnsm:
            *((BYTE*)(Peticion->Param1)) = Reparto_Pesos.PowerConsm;
            break;
        case TypeNoise:
            *((BYTE*)(Peticion->Param1)) = Reparto_Pesos.Noise;
            break;
        case TypeLQI:
            *((BYTE*)(Peticion->Param1)) = Reparto_Pesos.LQI;
            break;
        case TypeSec:
            *((BYTE*)(Peticion->Param1)) = Reparto_Pesos.Security;
            break;
        case TypeLaten:
            *((BYTE*)(Peticion->Param1)) = Reparto_Pesos.Latency;
            break;
        case TypeBWCsnm:
            *((BYTE*)(Peticion->Param1)) = Reparto_Pesos.BWConsmptn;
            break;
        case TypeIntrChng:
            *((BYTE*)(Peticion->Param1)) = Reparto_Pesos.IntrfcChng;
            break;
        case TypeThrgh:
            *((BYTE*)(Peticion->Param1)) = Reparto_Pesos.Throughput;
            break;
        case AllData:
            *((POLI_WEIGHTS*)(Peticion->Param1)) = Reparto_Pesos;
            break;
        default:
            break;
    }
    //TODO la lectura de la politica.
    return TRUE;
}
BOOL CRM_Poli_Write(POLI_MSSG_RCVD *Peticion)
{
    switch(Peticion->DataType)
    {
        case TypePwrCnsm:
            Reparto_Pesos.PowerConsm = *((BYTE*)(Peticion->Param1));
            break;
        case TypeNoise:
            Reparto_Pesos.Noise = *((BYTE*)(Peticion->Param1));
            break;
        case TypeLQI:
             Reparto_Pesos.LQI = *((BYTE*)(Peticion->Param1));
            break;
        case TypeSec:
             Reparto_Pesos.Security = *((BYTE*)(Peticion->Param1));
            break;
        case TypeLaten:
             Reparto_Pesos.Latency = *((BYTE*)(Peticion->Param1));
            break;
        case TypeBWCsnm:
             Reparto_Pesos.BWConsmptn = *((BYTE*)(Peticion->Param1));
            break;
        case TypeIntrChng:
             Reparto_Pesos.IntrfcChng = *((BYTE*)(Peticion->Param1));
            break;
        case TypeThrgh:
             Reparto_Pesos.Throughput = *((BYTE*)(Peticion->Param1));
            break;
        case AllData:
             Reparto_Pesos = *((POLI_WEIGHTS*)(Peticion->Param1));
            break;
        default:
            break;
    }
    //TODO la escritura de la politica.
    return TRUE;
}
/*Fin de funciones propias del sub-modulo*/

/*De incializacion del sub-modulo*/
BOOL CRM_Poli_Init(void)
{
    Reparto_Pesos.PowerConsm = PwrCnsmpInit;
    Reparto_Pesos.Noise = NoiseInit;
    Reparto_Pesos.LQI = LQIInit;
    Reparto_Pesos.Security = SecurityInit;
    Reparto_Pesos.Latency = LatencyInit;
    Reparto_Pesos.BWConsmptn = BWConsmptnInit;
    Reparto_Pesos.IntrfcChng = IntrfcChngInit;
    Reparto_Pesos.Throughput = ThroughputInit;

    //TODO la inicializacion de las politicas.
    return TRUE;
}
/*Fin de las funciones de inicializacion del sub-modulo*/

/*****************************************************************************/
/******************************FIN DE FUNCIONES*******************************/
/*****************************************************************************/
