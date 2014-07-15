/*****************************************************************************
 *
 *              AppDemo.h -- AppDemo v0.1
 *
 *****************************************************************************
 *
 * Author:          Jose Mª Bermudo Mera
 * FileName:        AppDemo.h
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
 *****************************************************************************/

#ifndef APPDEMO_H
#define	APPDEMO_H

#include "CWSN LSI Node/Include/WirelessProtocols/ConfigApp.h"

#include "CWSN LSI Node/Include/NodeHAL.h"

#include "CRModule/CRModule.h"
#include "CRModule/VCC/VCC.h"

void Rutina_Principal(void);
void Envio_Datos(void);
BOOL Rcvd_Buffer(RECEIVED_MESSAGE *Buffer);

#endif
