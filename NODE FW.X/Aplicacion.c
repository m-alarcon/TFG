#include "Aplicacion.h"

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
            SWDelay(100);
            if(n_rtx >= maxRTxXDefecto){
                Printf("\r\nSe ha alcanzado el numero maximo de retransmisiones, se descarta el paquete.");
                switch(ri){
                    case MIWI_0434:
                        MIWI434_rtx[c] = maxRTxXDefecto;
                    case MIWI_0868:
                        MIWI868_rtx[c] = maxRTxXDefecto;
                    case MIWI_2400:
                        MIWI2400_rtx[c] = maxRTxXDefecto;
                }
                DiscardTXData(ri);
                break;
            }
        }
        else{
            Printf(" => OK");
            switch(ri){
                case MIWI_0434:
                    MIWI434_rtx[c] = n_rtx;
                case MIWI_0868:
                    MIWI868_rtx[c] = n_rtx;
                case MIWI_2400:
                    MIWI2400_rtx[c] = n_rtx;
            }
            break;  //TRANSMISION CORRECTA, SALE DEL BUCLE
        }
    }
    EnviandoMssgApp = FALSE;
    }
}

void limpiaBufferRX(void){

    BYTE recibido = GetPayloadToRead(riActual);
    BYTE i;
    BYTE info;
    BYTE *data = &info;
    if (recibido != 0){
        for (i = 0; i < recibido; i++){
            GetRXData(riActual, data);
        }
    }
}

BOOL Rcvd_Buffer1(RECEIVED_MESSAGE *Buffer)
{
    BYTE i, err;
    if (!GetPayloadToRead(MIWI_0434) && !GetPayloadToRead(riActual)) {
        return FALSE;
    }
    
    BYTE RI_MASK = WhichRIHasData();       
    
    switch(RI_MASK){
        case MIWI_0434_RI_MASK:
            riData = MIWI_0434;
            Printf("\r\nMensaje por 434 recibido.");
            break;
        default:
            riData = riActual;
            break;
    }
            
    
    err = GetRXSourceAddr(riData, Buffer->SourceAddress);
    if (err & 0x80) {
        Printf("\r\nError al obtener la dirección: ");
        PrintChar(err);
        return FALSE;
    }
    
    for(i = 0; i < CONNECTION_SIZE; i++){
        if(isSameAddress(Buffer->SourceAddress, ConnectionTable[i].Address)){
            NumMssgIntercambiados[i]++;
            break;
        }
    }
    
    for (i = 0; GetPayloadToRead(riData) > 0; i++) {
        BYTE * storeItHere = &(Buffer->Payload[i]);
        err = GetRXData(riData, storeItHere);
        if (err) {
            Printf("\r\nError al obtener la dirección: ");
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
        
        if(Rcvd_Buffer1(&AppRXBuffer))
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
                    //TODO es un mensaje normal.
                    Printf("\r\nMensaje de aplicacion recibido");
                    break;
            }
        }
        RecibiendoMssg = FALSE;
}