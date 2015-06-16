/*****************************************************************************
 *
 *              ConfigRepository.h -- Repository Module Configuration v0.1
 *
 *****************************************************************************
 *
 * Author:          Guillermo Jara Luengos
 * FileName:        ConfigRepository.h
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
 * Este archivo configura el modulo repository (CR Module).
 *
 * Change History:
 *  Rev   Date(m/d/y)   Description
 *  0.1   23/10/2012    Initial revision
 *****************************************************************************/

#ifndef  _CONFIGREPOSITORY_H_
#define  _CONFIGREPOSITORY_H_

//#define TamanoTablaConexiones CONNECTION_SIZE*2 //El doble que la de MiWi p.ej.
//#define TamanoTablActiveScn ACTIVE_SCAN_RESULT_SIZE*2 //Idem.

#define TamanoInfoEntorno 64 //por poner un numero para el tamaÃ±o de la info
        //sobre el "ruido" de los canales que guardamos.
#define NumeroDePeticionesInicial 100 /*El numero de peticiones incial que
                                consideramos para que las primeras veces la
                                contribucino no sea brutal y modifique todo el
                                proceso.*/
#define MAX_VECTOR_POTENCIA 5 /*Se usa para cambiar el tamaño del vector con el
                                que se hace la media para lanzar el algoritmo*/
#define MAX_PAQ_APRENDIZAJE 100
#define MAX_CLUSTERS        10


#endif
