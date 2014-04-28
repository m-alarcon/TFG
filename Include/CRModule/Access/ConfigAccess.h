/*****************************************************************************
 *
 *              ConfigAccess.h -- Access Module Configuration v0.1
 *
 *****************************************************************************
 *
 * Author:          Guillermo Jara Luengos
 * FileName:        ConfigAccess.h
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
 * Este archivo configura el modulo Access (CR Module).
 *
 * Change History:
 *  Rev   Date(m/d/y)   Description
 *  0.1   23/10/2012    Initial revision
 *****************************************************************************/

#ifndef  __CONFIGACCESS_H_
#define  __CONFIGACCESS_H_

/*************************VARIABLES DE CONFIGURACION**************************/


/********************************FIN DE VARIABLES*****************************/

/*************************PARAMETROS DE CONFIGURACION*************************/

//REVISAR.Diría que es imposible que connection size no esté definido. Lo pongo así por
// si acaso. Lo que desde luego no tiene sentido es que el registro del
//permiso de nodos sea mayor que connection size asi que cuidado con eso.
#if defined(CONNECTION_SIZE)
    #define MAX_NUMERO_PERM_NODOS CONNECTION_SIZE
#else
    #define MAX_NUMERO_PERM_NODOS 5
#endif
/********************************FIN DE PARAMETROS****************************/

#endif
