/*****************************************************************************
 *
 *              AppDemo.c -- AppDemo v0.1
 *
 *****************************************************************************
 *
 * Author:          Jose Mª Bermudo Mera
 * FileName:        AppDemo.c
 * Dependencies:    AppDemo.h
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
 *  Rev   Date(m/d/y)   	Description
 *****************************************************************************/

#include "AppDemo.h"

BOOL HayDatosApp;
BOOL EnviandoMssgApp;
BOOL RecibiendoMssg;

extern RECEIVED_MESSAGE BufferRecepcionPrueba;

//Para el envio de datos de app y de control en paralelo.
RECEIVED_MESSAGE AppRXBuffer;
//    BYTE APPBufferRX[RX_BUFFER_SIZE];//Habia pensado en utilizar otro buffer
                            //distinto al bufferRX pero a priori diria que
                            //no existe conflicto.
BYTE AppDireccion[MY_ADDRESS_LENGTH];
BYTE BufferRx[RX_BUFFER_SIZE];
//XXX

extern radioInterface ri;

BYTE SMS1[7] = "OYEEE!!";
BYTE TestAddress[8];

void Rutina_Principal(void)
{
    BYTE i, j;
    BYTE TxSynCount = 0;
    BYTE RxNum = 0;
    BOOL bReceivedMessage = FALSE;

    //Jose//LimpiaBuffer();
    
    AppRXBuffer.Payload = BufferRx;
    AppRXBuffer.SourceAddress = AppDireccion;
    CtrlMssgFlag = FALSE;
    HayDatosApp = FALSE;
    EnviandoMssgApp = FALSE;
    RecibiendoMssg = FALSE;

    TestAddress[0] = 0x00;
    TestAddress[1] = 0x11;
    TestAddress[2] = 0x22;
    TestAddress[3] = 0x33;
    TestAddress[4] = 0x44;
    TestAddress[5] = 0x55;
    TestAddress[6] = 0x66;

    LED1 = 1;

    Printf("\r\n   Demo Instruction:");
    Printf("\r\n        Push Button 2 to unicast encrypted message. LED 2 will");
    Printf("\r\n                     be toggled upon receiving messages. ");
    Printf("\r\n\r\n");

    while(1)
    {
        //Jose//LimpiaBufferRx();
        VCCMSSGTYPE CabeceraRxMssg;
        RecibiendoMssg = TRUE;
        if(Rcvd_Buffer(&AppRXBuffer))
        {
            RecibiendoMssg = FALSE;
            CabeceraRxMssg = (VCCMSSGTYPE)(AppRXBuffer.Payload[CtrlMssgField]);
            switch(CabeceraRxMssg)
            {
                case 0x00:
                    //Los mensajes de control de MiWi tienen esta cabecera.
                    break;
                case VccCtrlMssg: /*Si es un mensaje de control que debe ir por
                                   el VCC. Lo procesamos siempre que no haya ya
                                   uno a la espera. El flag lo levantará la
                                   rutina de atencion a la interrupcion que
                                   genera el optimizer.*/
                    /*Como siempre recibimos en AppRXBuffer, si el mensaje es de
                     control lo copiamos a otro buffer para liberar AppRXBuffer
                     y poder seguir procesando otros.*/
                    if(!CtrlMssgFlag)
                    {
                        BYTE i;
                        /*Los flags del paquete recibido.*/
                        BufferRecepcionPrueba.flags.Val = AppRXBuffer.flags.Val;
                        /*El ID de la PAN a la que pertenece el nodo origen.*/
                        BufferRecepcionPrueba.SourcePANID = AppRXBuffer.SourcePANID;
                        /*La direccion del nodo origen.*/
                        for(i = 0; i < MY_ADDRESS_LENGTH; i++)
                        {
                            BufferRecepcionPrueba.SourceAddress[i] = AppRXBuffer.SourceAddress[i];
                        }
                        /*El tamaño del payload del paquete recibido*/
                        BufferRecepcionPrueba.PayloadSize = AppRXBuffer.PayloadSize;
                        /*El RSSI para el paquete recibido.*/
                        BufferRecepcionPrueba.PacketRSSI = AppRXBuffer.PacketRSSI;
                        /*El LQI para el paquete recibido.*/
                        BufferRecepcionPrueba.PacketLQI = AppRXBuffer.PacketLQI;

                        for(i=0; i < AppRXBuffer.PayloadSize; i++)
                        {
                            BufferRecepcionPrueba.Payload[i] = AppRXBuffer.Payload[i];
                        }
                        CtrlMssgFlag = TRUE;
                    }
                    break;
                default:
                    //TODO es un mensaje de datos.
                    Printf("\r\nMensaje recibido: ");
                    #if defined DATA_OVER_VCC && defined CRMODULE
                    for(i = 0; i < AppRXBuffer.PayloadSize; i++) {
                        ConsolePut(AppRXBuffer.Payload[i]);
                    }
                    #endif
                    break;
            }
        }
        RecibiendoMssg = FALSE;
            /*******************************************************************/
            // If no packet received, now we can check if we want to send out
            // any information.
            // Function ButtonPressed will return if any of the two buttons
            // has been pushed.
            /*******************************************************************/
        BYTE PressedButton = ButtonPressed();
        switch( PressedButton )
        {
            case 1:
                break;
            case 2:
                HayDatosApp = TRUE;
                break;
            default:
                break;
        }
        if(HayDatosApp && !CtrlMssgFlag)
        {
            Envio_Datos();
        }
    }
}

void Envio_Datos(void)
{
    #if defined NODE_1
    TestAddress[7] = 0x22;//Dirección del nodo 2
    #elif defined NODE_2
    TestAddress[7] = 0x11;//Dirección del nodo 1
    #endif
    BYTE i, j;
    i = 0;
    EnviandoMssgApp = TRUE;
    while(i < sizeof(SMS1)) {
        j = PutTXData(ri, SMS1[i]);
        if (j) {
            Printf("\r\nFallo al escribir en el buffer. Codigo de error: ");
            PrintChar(j);
        } else {
            i++;
        }
    }
    i = SendPckt(ri, LONG_MIWI_ADDRMODE, TestAddress);
    Printf("\r\nPaquete enviado: ");
    if (i == 0) {
        Printf(" => OK");
    } else {
        Printf(" => FALLO: ");
        PrintChar(i);
    }
    EnviandoMssgApp = FALSE;
    HayDatosApp = FALSE;//Revisar según lo que quiera hacer
    //SWDelay(2000);
}

/******************************************************************************/
/*
 * Nombre: BOOL Rcvd_Buffer(BYTE *Buffer)
 * Función: Llenar el buffer que le pasamos como parámetro con los elementos que
 * llegan.
 * Devuelve: TRUE o FALSE segun se haya recibido correctamente.
 * Parametros: El buffer en el que queremos escribir y su tamaño.
 *
 * NOTA:
 *
 */
BOOL Rcvd_Buffer(RECEIVED_MESSAGE *Buffer)
{
    BYTE i, err;
    if (!GetPayloadToRead(ri)) {
        return FALSE;
    }
    err = GetRXSourceAddr(ri, Buffer->SourceAddress);
    if (err & 0x80) {
        Printf("\r\nError al obtener la dirección: ");
        PrintChar(err);
        return FALSE;
    }
    for (i = 0; GetPayloadToRead(ri) > 0; i++) {
        BYTE * storeItHere = &(Buffer->Payload[i]);
        err = GetRXData(ri, storeItHere);
        if (err) {
            Printf("\r\nError al obtener la dirección: ");
            PrintChar(err);
            return FALSE;
        }
    }
    Buffer->PayloadSize = i;
    return TRUE;
}
