/* 
 * File:   Aplicacion.h
 * Author: malarcon
 *
 * Created on 9 de junio de 2015, 18:11
 */
#include "NodeHAL.h"
#include "CWSN LSI Node/Include/WirelessProtocols/Console.h"
#include "CWSN LSI Node/Include/WirelessProtocols/ConfigApp.h"
#include "CRModule/CRModule.h"
#include "CRModule/VCC/VCC.h"
#include "CRModule/Optimizer/Optimizer.h"
#include "CRModule/Optimizer/ConfigOptimizer.h"
//#include "CRModule/Repository/Repository.h"

#ifndef APLICACION_H
#define	APLICACION_H

BYTE VCCmssg;
void limpiaBufferRX(radioInterface radio);

#endif	/* APLICACION_H */