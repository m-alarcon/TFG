/*****************************************************************************
 *
 *              CRModule.h -- CRModule v0.1
 *
 *****************************************************************************
 *
 * Author:          Guillermo Jara Luengos
 * FileName:        CRModule.h
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
 * Este archivo implementa el control de acceso al repositorio.
 *
 * Change History:
 *  Rev   Date(m/d/y)   	      Description
 *  0.1   November 23, 2012, 12:58 PM    Initial revision
 *****************************************************************************/

#ifndef CRMODULE_H
#define	CRMODULE_H

#define INPUT
#define OUTPUT
#define IOPUT

//Josembm: defino aquí los tipos necesarios para CRModule
/*CRMODULE*/
//Una definicion de un tipo para definir los sub-modulos cognitivos, le añadimos
//una que note que queremos comunicarnos con un nodo externo.
typedef enum _SUB_MODULE {SubM_AccCtrl = 0x01, SubM_Disc, SubM_Exec, SubM_Opt, SubM_Poli, SubM_Repo, SubM_Mssngr, SubM_Ext} SUB_MODULE;
//Para definir los dos procesadores de mensajes.
typedef enum _MSSGPR {VCC = 0x01, NMM} MSSGPR;
/*fin de CRMODULE*/

//#include "CRModule/Messenger/Messenger.h"

#endif	/* CRMODULE_H */

