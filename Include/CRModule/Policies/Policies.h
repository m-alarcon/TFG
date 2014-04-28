/*****************************************************************************
 *
 *              Policies.h -- Policies Module v0.1
 *
 *****************************************************************************
 *
 * Author:          Guillermo Jara Luengos
 * FileName:        Policies.h
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
 * Este archivo implementa el modulo correspondiente a las politicas del
 * modulo cognitivo (CR Module).
 *
 * Change History:
 *  Rev   Date(m/d/y)   Description
 *  0.1   23/10/2012    Initial revision
 *****************************************************************************/
#ifndef __POLICIES_H_
#define __POLICIES_H_

#include "GenericTypeDefs.h"
#include "../CRModule.h"

#include "ConfigPolicies.h"

/*****************************************************************************/
/***************************DEFINICION DE TIPOS*******************************/

/*****************************************************************************/

typedef enum _POLIACTION
{
    ActRead = 0x01, ActWrite
} POLIACTION;

typedef enum _POLIDATATYPE
{
    TypePwrCnsm = 0x01, TypeNoise, TypeLQI, TypeSec, TypeLaten, TypeBWCsnm, TypeIntrChng, TypeThrgh, AllData =0x10
} POLIDATATYPE;

/*La estructura de los parametros del Mensaje con destino este modulo.*/
typedef struct _POLI_MSSG_RCVD
{
    INPUT BYTE OrgModule;
    INPUT POLIACTION Action;
    IOPUT BYTE DataType; //El parametro de la politica que queremos W/R.
    INPUT void *Param1; //En principio era para poner el valor que queremos
                        //escribir o el puntero a la salida para alojar la
                        //lectura pero no hace falta no? Lo utilizamos como
                        //hemos hecho en TEST1 y esta bien verdad?.
}POLI_MSSG_RCVD;

/******************************************************************************/
/*
 * Nombre: typedef Policies
 * Función: Definir un tipo de elemento, que además es una estructura, que
 *          especifique los pesos que tienen diferentes parametros como consumo,
 *           seguridad, etc. y de esta forma tomar decisiones en cada nodo y en
 *          la red de acuerdo a los pesos de las politicas.
 */
typedef struct _POLI_WEIGHTS
{
    BYTE PowerConsm; //Consumo.
    BYTE Noise; //Ruido del canal.
    BYTE LQI; //Calidad del enlace.
    BYTE Security; //Seguridad.
    BYTE Latency; //Latencia.
    BYTE BWConsmptn; //Consumo de ancho de banda.
    BYTE IntrfcChng; //Disposicion a cambiar de canal.
    BYTE Throughput; //Tasa de datos.
    BYTE KeepConect; //Predisposicion a mantener la conexi�n.
    //TODO añadir mas politicas.
}POLI_WEIGHTS;

/*****************************************************************************/
/*************************FIN DEFINICION DE TIPOS*****************************/
/*****************************************************************************/


/*****************************************************************************/
/*********************************FUNCIONES***********************************/
/*****************************************************************************/

/*De interfaz con messenger*/
BOOL CRM_Poli_Mssg_Rcvd(POLI_MSSG_RCVD *Peticion);
/*Fin de funciones de interfaz con messenger*/

/*Propias del sub-modulo*/
BOOL CRM_Poli_Read(POLI_MSSG_RCVD *Peticion);
BOOL CRM_Poli_Write(POLI_MSSG_RCVD *Peticion);
/*Fin de funciones propias del sub-modulo*/

/*Funcion de inicializacion*/
BOOL CRM_Poli_Init(void);
/*Fin de funciones de inicializacion*/

/*****************************************************************************/
/******************************FIN DE FUNCIONES*******************************/
/*****************************************************************************/


#endif
