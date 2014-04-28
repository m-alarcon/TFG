/*****************************************************************************
 *
 *              ConfigDiscovery.c -- Discovery Module Configuration v0.1
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
 * Este archivo se ocupa de configurar el modulo discovery, entendiendo por
 * configuración funciones tales como activar la salida por el puerto serie
 * para debugeo.
 *
 * Change History:
 *  Rev   Date(m/d/y)   Description
 *  0.1   23/10/2012    Initial revision
 *****************************************************************************/
#ifndef  _CONFIGDISCOVERY_H_
#define  _CONFIGDISCOVERY_H_

/*******************PARAMETROS DE CONFIGURACION*******************************/

//CARRIER_SCAN. La comentamos de tal manera que lancemos un error en
//compilación si intentamos utilizar el escaneo de portadoras. Ya que
//por el momento no sabemos como utilizarlo en el MRF24J40 suponiendo
//que sea posible puesto que oficialmente no esta disponible.
//#define CARRIER_SCAN

/*Definiciones de parametros para las configuraciones por defecto*/
#define DfaultAutoDiscTime 0 //Por defecto no hara autodiscovery.
/*Fin de los parametros por defecto*/

/*Configuraciones por defecto para uso de MiWi*/
#define DfaultDiscChnMap 0xFFFFFFFF
#define DfaultDiscScnDura 10
#define DfaultDiscDetctMode NOISE_DETECT_ENERGY
/*Fin de configuracion por defecto de MiWi*/

/******************FIN DE PARAM DE CONFIGURACION******************************/
#endif
