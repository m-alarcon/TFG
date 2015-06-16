/*****************************************************************************
 *
 *              DataClustering.h -- DataClustering v0.1
 *
 *****************************************************************************
 *
 * Author:          malarcon
 * FileName:        Consumo.h
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
 *
 *
 * Change History:
 *  Rev   Date(m/d/y)   	      Description
 *  0.1   03/11/2015                  Primera versión
 *****************************************************************************/

#include "CWSN LSI Node/Include/NodeHAL.h"
#include "CWSN LSI Node/Include/WirelessProtocols/Console.h"
#include "CWSN LSI Node/Include/WirelessProtocols/ConfigApp.h"
#include "CRModule/CRModule.h"
#include "CRModule/VCC/VCC.h"

#ifndef CONSUMO_H
#define	CONSUMO_H

//Cambiar según necesidades




typedef struct mensCambio {
    BYTE chOpt;
    BYTE potCanales[MIWI2400NumChannels];
} msgChng;

BYTE canal;
BOOL recPaqCambioCanal;
msgChng paqRec;

void Consumo(void);
BYTE CalcularMedia(BYTE *pVector);
BOOL CalcularCostes();

#endif	/* CONSUMO_H */

