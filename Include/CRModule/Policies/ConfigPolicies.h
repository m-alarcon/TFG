/*****************************************************************************
 *
 *              ConfigPolicies.h -- Policies Module Configuration v0.1
 *
 *****************************************************************************
 *
 * Author:          Guillermo Jara Luengos
 * FileName:        ConfigPolicies.h
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
 * Este archivo configura el modulo Policies (CR Module).
 *
 * Change History:
 *  Rev   Date(m/d/y)   Description
 *  0.1   23/10/2012    Initial revision
 *****************************************************************************/

#ifndef  _CONFIGPOLICIES_H_
#define  _CONFIGPOLICIES_H_

#define PwrCnsmpInit       0   //Consumo.
#define NoiseInit          0   //Ruido del canal.
#define LQIInit            0   //Calidad del enlace.
#define SecurityInit       0   //Seguridad.
#define LatencyInit        0   //Latencia.
#define BWConsmptnInit     0   //Consumo de ancho de banda.
#define IntrfcChngInit     10   //Disposicion a cambiar de canal.
#define ThroughputInit     0   //Tasa de datos.


#endif
