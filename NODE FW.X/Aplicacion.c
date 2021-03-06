#include "Aplicacion.h"
#include "CRModule/Repository/Repository.h"
#include "NodeHAL.h"
BYTE i, j;
BYTE *beatles = "When I find myself in times of trouble, Mother Mary comes to me,"
            " speaking words of wisdom, let it be. And in my hour of darkness"
            " she is standing right in front of me, speaking words of wisdom,"
            "let it be";

BOOL HayDatosApp;
BOOL EnviandoMssgApp;
BOOL RecibiendoMssg;
extern RECEIVED_MESSAGE BufferRecepcionPrueba;
RECEIVED_MESSAGE AppRXBuffer;
BYTE AppDireccion[MY_ADDRESS_LENGTH];
BYTE BufferRx[RX_BUFFER_SIZE];
extern radioInterface riActual;
extern radioInterface riData;

void Enviar_Paquete_Datos_App(radioInterface ri, BYTE modo, BYTE *addr){

    if(!CHNG_MSSG_RCVD){
    BYTE n_rtx,c;
    n_rtx = 0;
    i = 0;
    EnviandoMssgApp = TRUE;
    c = GetOpChannel(ri);
    while(GetFreeTXBufSpace(ri) > 0){     //Hasta llenar el buffer de transmision
        j = PutTXData(ri, beatles[i]);
        if (j != NO_ERROR){
            Printf("\r\nFallo al escribir en el buffer: ");
            PrintChar(j);
        }
        else
            i++;
    }

    Printf("\r\nEnvio de mensaje de datos.");
    while(TRUE){
        //Enviar paquete con los datos que haya en el buffer de la interfaz
        i = SendPckt(ri, modo, addr);
        if (i != NO_ERROR){
            Printf(" => FALLO");
            //i = GetFreeTXBufSpace(ri);
            //Printf("\r\nCapacidad libre del buffer de TX: ");
            //PrintDec(i);
            n_rtx++;
            switch(ri){
                case MIWI_0434:
                    MIWI434_rtx[c] = n_rtx;
                    break;
                case MIWI_0868:
                    MIWI868_rtx[c] = n_rtx;
                    break;
                case MIWI_2400:
                    MIWI2400_rtx[c] = n_rtx;
                    break;
            }
            if(n_rtx >= maxRTxXDefecto){
                Printf("\r\nSe ha alcanzado el numero maximo de retransmisiones, se descarta el paquete.");
                switch(ri){
                    case MIWI_0434:
                        MIWI434_rtx[c] = maxRTxXDefecto;
                        break;
                    case MIWI_0868:
                        MIWI868_rtx[c] = maxRTxXDefecto;
                        break;
                    case MIWI_2400:
                        MIWI2400_rtx[c] = maxRTxXDefecto;
                        break;
                }
                DiscardTXData(ri);
                break;
            }
        }
        else{
            Printf(" => OK");
            break;  //TRANSMISION CORRECTA, SALE DEL BUCLE
        }
    }
    EnviandoMssgApp = FALSE;
    }
}

void limpiaBufferRX(radioInterface radio){

    while(NodeStatus.MIWI2400_RXbuf_isEmpty == FALSE){
        BYTE recibido = GetPayloadToRead(radio);
        BYTE i;
        BYTE info;
        BYTE *data = &info;
        if (recibido != 0){
            for (i = 0; i < recibido; i++){
                GetRXData(radio, data);
            }
        }
    }
}

BOOL Proc_Buff(RECEIVED_MESSAGE *Buffer)
{
    BYTE i, err;
    
    if (!GetPayloadToRead(MIWI_0434) && !GetPayloadToRead(riActual)) {
        return FALSE;
    }
    
    if(!GetPayloadToRead(riActual) && GetPayloadToRead(MIWI_0434)){
        riData = MIWI_0434;
        Printf("\r\n//////Se ha recibido mensaje por VCC.");
    } else if(!GetPayloadToRead(MIWI_0434) && GetPayloadToRead(riActual)){
        riData = riActual;
        if(MSSG_PROC_OPTM == 0 && CHNG_MSSG_RCVD == 0){
            return FALSE;
        }
    } else if(GetPayloadToRead(riActual) && GetPayloadToRead(MIWI_0434)){
        riData = MIWI_0434;
    } else {
        return FALSE;
    }
            
    
    err = GetRXSourceAddr(riData, Buffer->SourceAddress);
    if (err & 0x80) {
        Printf("\r\nError al obtener la direcci�n: ");
        PrintChar(err);
        return FALSE;
    }
    
#ifdef DATACLUSTERING
    BYTE inNet = 0;
    
    for (i = 0; i < CONNECTION_SIZE; i++){                    
        if(isSameAddress(Buffer->SourceAddress, ConnectionTable[i].Address)){
            inNet = 1;
            break;
        }
    }
    
    if (inNet == 0){
        limpiaBufferRX(riData);
        return FALSE;
    }
#endif
    
    for (i = 0; GetPayloadToRead(riData) > 0; i++) {
        BYTE * storeItHere = &(Buffer->Payload[i]);
        err = GetRXData(riData, storeItHere);
        if (err) {
            Printf("\r\nError al obtener la direcci�n: ");
            PrintChar(err);
            return FALSE;
        }
    }
    Buffer->PayloadSize = i;    
    return TRUE;
}

void Recibir_info(void){

    VCCMSSGTYPE CabeceraRxMssg;
    
    AppRXBuffer.Payload = BufferRx;
    AppRXBuffer.SourceAddress = AppDireccion;    
    
    RecibiendoMssg = TRUE;
        
        if(Proc_Buff(&AppRXBuffer))
        {
            RecibiendoMssg = FALSE;
            CabeceraRxMssg = (VCCMSSGTYPE)(AppRXBuffer.Payload[CtrlMssgField]);
            switch(CabeceraRxMssg)
            {
                case 0x00:
                    //Los mensajes de control de MiWi tienen esta cabecera.
                    break;
                case VccCtrlMssg:
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
                            *(BufferRecepcionPrueba.SourceAddress + i) = *(AppRXBuffer.SourceAddress + i);
                        }
                        /*El tama�o del payload del paquete recibido*/
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
                    //TODO es un mensaje normal.
                    Printf("\r\nMensaje de aplicacion recibido");
                    break;
            }
        }
        RecibiendoMssg = FALSE;
        MSSG_PROC_OPTM = 0;
}