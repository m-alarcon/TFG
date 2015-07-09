/*****************************************************************************
 *
 *              ConfigOptimizer.h -- Optimizer Module Configuration v0.1
 *
 *****************************************************************************
 *
 * Author:          Guillermo Jara Luengos
 * FileName:        ConfigOptimizer.h
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
 * Este archivo configura el modulo Optimizer (CR Module).
 *
 * Change History:
 *  Rev   Date(m/d/y)   Description
 *  0.1   23/10/2012    Initial revision
 *****************************************************************************/

#ifndef  _CONFIGOPTIMIZER_H_
#define  _CONFIGOPTIMIZER_H_

#if defined(TEST4)
//#define NodoReceptor
//#define NodoEmisor
#endif
#if defined TEST6
#define TIEMPODEESPERARESPUESTA 10000
#endif

//Cada cuanto queremos que se ejecute la rutina de optimizacion.
#define PeriodoXDefecto 50 //En mseg => Cada 10 milisegundos.

/*Parametros para la funcion de optimizacion de Elena.*/
#define CosteTxXDefecto 100
#define CosteRxXDefecto 70
#define CosteSensingXDefecto 180
#define numMsjXDefecto 100
#define AprendizajeMax 600
#define ReinicioMax 6000
#define PorcentajeMinimo 10
#if defined(MRF49XA) || defined(MRF49XA_alt)
    #define maxRTxXDefecto RETRANSMISSION_TIMES
#else
    #define maxRTxXDefecto 30 //TODO el MRF24J40 no tiene por defecto rtx, hay que
        //implementarlas en apliacion y definiriamos nosotros cuantas como
        //maximo queremos que se den.
#endif
#define probChngCompiXDefecto 200 //REVISAR
/*Fin de los parametros para la funcion de Elena*/

#endif
