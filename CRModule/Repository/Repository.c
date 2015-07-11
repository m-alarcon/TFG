/*****************************************************************************
 *
 *              Repository.c -- Repository Module v0.1
 *
 *****************************************************************************
 *
 * Author:          Guillermo Jara Luengos
 * FileName:        Repository.c
 * Dependencies:    Repository.h
 * Processor:
 * BOARD:
 * Compiler:        C32 01.00.02 or higher
 * Linker:          MPLINK 03.40.00 or higher
 * Company:         B105 -FAILURE IS NOT AN OPTION-
 *
 *****************************************************************************
 * File Description:
 *
 * Este archivo desarrolla las funciones y datos del modulo repository
 * (CR Module).
 *
 * Change History:
 *  Rev   Date(m/d/y)   Description
 *  0.1   23/10/2012    Initial revision
 *****************************************************************************/

#include "CRModule/Repository/Repository.h"
#include "CRModule/Repository/ConfigRepository.h"


/*****************************************************************************/
/*********************************VARIABLES***********************************/
/*****************************************************************************/

//Para el optimizer saber cuantas peticiones llevamos en total.
BYTE NumeroDePeticionesDeCambio;
extern radioInterface riActual;
int paquetesRecibidos;

int nClusters;
cl cluster;
/*****************************************************************************/
/******************************FIN DE VARIABLES*******************************/
/*****************************************************************************/


/*****************************************************************************/
/*********************************FUNCIONES***********************************/
/*****************************************************************************/

/*De recepcion/interfaz con Messenger*/
BOOL CRM_Repo_Mssg_Rcvd(REPO_MSSG_RCVD *Peticion)
{
     switch((*Peticion).Action)
    {
        case(ActStr):
            CRM_Repo_Store(Peticion);
            break;
        case(ActSndDta):
            CRM_Repo_SendDat(Peticion);
            break;
        default:
            break;
    }
    return TRUE;
}
/*Fin de funciones de recepcion/interfaz con Messenger*/

/*Funciones propias del sub-modulo de I/O de datos*/
BOOL CRM_Repo_Store(REPO_MSSG_RCVD *Peticion)
{
    switch((*Peticion).DataType)
    {
        case(OwnNode):
            CRM_Repo_NodoPropio(Peticion);
            break;
        case(NetNode):
            CRM_Repo_NodosRed(Peticion);
            break;
        case(EnvNode):
            CRM_Repo_NodosEnv();
            break;
        case(Enviro):
            CRM_Repo_Env(*(BYTE*)((*Peticion).Param1), *(BYTE*)((*Peticion).Param2));
            break;
        case(IncluirPotencia):
            CRM_Repo_NodoPropio(Peticion);
            break;
        case(AddMsg):
            CRM_Repo_Mensajes_Intercambiados((*Peticion).Param1);
            break;
        case (SaveRSSI):
            CRM_Repo_Str_RSSI(*(BYTE*)(Peticion->Param1));
            break;
        case (RstRSSI):
            CRM_Repo_Reiniciar_Potencias();
            break;
        case (RstRTx):
            CRM_Repo_Reiniciar_RTx();
            break;
        case (AddCoord):
            CRM_Repo_Guardar_Coordenadas(Peticion);
            break;         
        case (DetAtt):
            CRM_Repo_Proc_Mens_Att(Peticion);
            break;
        case (InitTAtt):
            inicializarTablaAtacantes();
            break;
        default:
            break;
    }
    //TODO.HECHO.Almacenar datos.
    return TRUE;
}
BOOL CRM_Repo_SendDat(REPO_MSSG_RCVD *Peticion)
{
    #if defined(TEST1)
        OUTPUT BYTE valor = 44;
        (*Peticion).Param1 = &valor;
    #endif
    BYTE i;
    //PRUEBA TODO en el final hay que hacerlo bien clasificando las acciones y tal.
    switch (Peticion->DataType)
    {
        case OwnNode:
            //Para solicitar info del nodo propio.
            break;
        case NetNode: //Para solicitar info de algun nodo de la red.
            switch(*((BYTE*) (Peticion->Param1)))
            {
                case AdditionalNetNode:
                    Peticion->Param3 = Repo_Conn_Table[*((BYTE*) (Peticion->Param2))].PeerInfo; /*TODO en realidad deberiamos copiarlo o
                                                                            cogerlo del repositorio permanente (cuando
                                                                            lo tengamos) porque este Repo_Conn podríamos
                                                                            borrarlo.*/ //Serán las peticiones enviadas.
                    Peticion->Param4 = Repo_Conn_Table[*((BYTE*) (Peticion->Param2))].PeerInfo + 1; /*Las peticiones aceptadas.*/
                    break;
                case AllNetNode:
                    break;
                default:
                    break;
            }
            break;
        case EnvNode:
            break;
        case Enviro:
            break;
        case AllInfo:
            break;
        case EnvPotencias:
            Peticion->Param1 = &vectorPotencias[0];
            break;
        case EnvRTx:
            switch(Peticion->Transceiver)
            {
                case MIWI_0434:
                    Peticion->Param2 = &MIWI434_rtx[*((BYTE*) (Peticion->Param1))];
                    break;
                case MIWI_0868:
                    Peticion->Param2 = &MIWI868_rtx[*((BYTE*) (Peticion->Param1))];
                    break;
                case MIWI_2400:
                    Peticion->Param2 = &MIWI2400_rtx[*((BYTE*) (Peticion->Param1))];
                    break;
            }
            break;
        case EnvNMsg:
            for(i = 0; i < CONNECTION_SIZE; i++){
                if(isSameAddress(Peticion->Param1, ConnectionTable[i].Address)){
                    Peticion->Param2 = &NumMssgIntercambiados[i];
                }
            }
            break;
        case EnvRSSI:
        {//CRM_Repo_Get_RSSI(*(radioInterface*)(Peticion->Param1), *(BYTE*)(Peticion->Param2), Peticion->Param3, Peticion->Param4);    
            BYTE i, j, n, swap, swapCh;
            BYTE array[MIWI2400NumChannels] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
            BYTE arrayCh[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
            switch(*(radioInterface*)(Peticion->Param1)){
                case MIWI_0868:
                    n = MIWI0868NumChannels;
                    for (i = 0; i < n; i++){
                        array[i] = MIWI868_RSSI_values[i];                
                    }
                    break;
                case MIWI_2400:
                    n = MIWI2400NumChannels;
                    for (i = 0; i < n; i++){            
                        array[i] = MIWI2400_RSSI_values[i];              
                    }
                    break;             
            }    
            for (i = 0 ; i < ( n - 1 ); i++){
                for (j = 0 ; j < n - i - 1; j++){
                    if (array[j] > array[j+1]){
                        swap         = array[j];
                        array[j]     = array[j+1];
                        array[j+1]   = swap;
                        swapCh       = arrayCh[j];
                        arrayCh[j]   = arrayCh[j+1];
                        arrayCh[j+1] = swapCh;                
                    }
                }
            }

            switch(ri){
                case MIWI_0868:
                    n = MIWI0868NumChannels;
                    for (i = 0; i < n; i++){
                        char trazaCanales[80];
                        sprintf(trazaCanales, "\r\nMIWI 868 Canal %d RSSI %d.\r\n", arrayCh[i], array[i]);
                        Printf(trazaCanales);                
                    }
                    break;
                case MIWI_2400:
                    n = MIWI2400NumChannels;
                    for (i = 0; i < n; i++){            
                        char trazaCanales[80];
                        sprintf(trazaCanales, "\r\nMIWI 2400 Canal %d RSSI %d.\r\n", arrayCh[i], array[i]);
                        Printf(trazaCanales);              
                    }
                    break;             
            }       


            char trazaCanales[80];
            sprintf(trazaCanales, "\r\nEl canal que se pide es el %d y su RSSI %d\r\n", arrayCh[*(BYTE*)(Peticion->Param2)], array[*(BYTE*)(Peticion->Param2)]);
            Printf(trazaCanales);
            Peticion->Param3 = &arrayCh[*(BYTE*)(Peticion->Param2)];
            Peticion->Param4 = &array[*(BYTE*)(Peticion->Param2)];
            break;
        }
        case EnvListaPaq:
            Peticion->Param1 = Lista_Paq_Rec_Aprendizaje;
            break;
        case EnvListaCl:
            Peticion->Param1 = Lista_Clusters;
            break;
        case EnvTablaAt:
            Peticion->Param1 = Tabla_Atacantes;
            break;
        default:
            break;
    }

    //TODO enviar datos. Guardar en el parametro los datos del repo que
    //queramos vamos.
    return TRUE;
}
/*Fin de funciones de I/O de datos*/

/*Funciones internas del sub-modulo*/
/*Para solicitar guardar informacion sobre el nodo propio.*/
void CRM_Repo_NodoPropio(REPO_MSSG_RCVD *Peticion)
{
    BYTE i;
    switch (*((BYTE*)(Peticion->Param1)))
    {
        case AllOwnNode:
            /*La info tipica de un active scan pero para el nodo propio.*/
            InfoPropioNodo.Channel = GetOpChannel(Peticion->Transceiver);//currentChannel;
            //InfoPropioNodo.Address = myLongAddress;
            for (i = 0; i < MY_ADDRESS_LENGTH; i++)
            {
                InfoPropioNodo.Address[i] = GetMyLongAddress(i);
            }
            InfoPropioNodo.PANID = GetPANID(Peticion->Transceiver);
            InfoPropioNodo.Capability.bits.Role = MiWiCapacityInfo.bits.Role;
            InfoPropioNodo.Capability.bits.Sleep = MiWiCapacityInfo.bits.Sleep;
            InfoPropioNodo.Capability.bits.SecurityEn = MiWiCapacityInfo.bits.Security;
            //InfoPropioNodo.Capability.bits.RepeatEn = //No lo utiliza la pila en
            //ningun sitio.
            if (MiWiCapacityInfo.bits.ConnMode <= ENABLE_PREV_CONN)
            {
                InfoPropioNodo.Capability.bits.AllowJoin = 1;
            }
            //InfoPropioNodo.Capability.bits.Direct = //REVISAR no tiene sentido en el
            //nodo propio.no?
            //InfoPropioNodo.Capability.bits.altSrcAddr = //REVISAR ya he puesto que
            //se guarde la larga al principio de este método.
            //no?
            for (i = 0; i < ADDITIONAL_NODE_ID_SIZE; i++)
            {
                InfoPropioNodo.PeerInfo[i] = AdditionalNodeID[i];
            }
            /*Fin de la info tipica de active scan.*/

            /*Info de los permisos del nodo*/
            for (i = 0; i < MAX_NUMERO_PERM_NODOS; i++)
            {
                Repo_Tabla_Permo_Acc[i] = TABLA_PERM_ACC[i];
            }
            /*Fin de la info de los permisos del nodo propio.*/

            /*Info de las politicas*/
            Repo_Poli_Weights = Reparto_Pesos;
            /*Fin de info de las políticas*/
            break;
        case ASOwnNode:
            /*La info tipica de un active scan pero para el nodo propio.*/
            InfoPropioNodo.Channel = GetOpChannel(Peticion->Transceiver);//currentChannel;
            //InfoPropioNodo.Address = myLongAddress;
            for (i = 0; i < MY_ADDRESS_LENGTH; i++)
            {
                InfoPropioNodo.Address[i] = GetMyLongAddress(i);
            }
            InfoPropioNodo.PANID = GetPANID(Peticion->Transceiver);
            InfoPropioNodo.Capability.bits.Role = MiWiCapacityInfo.bits.Role;
            InfoPropioNodo.Capability.bits.Sleep = MiWiCapacityInfo.bits.Sleep;
            InfoPropioNodo.Capability.bits.SecurityEn = MiWiCapacityInfo.bits.Security;
            //InfoPropioNodo.Capability.bits.RepeatEn = //No lo utiliza la pila en
            //ningun sitio.
            if (MiWiCapacityInfo.bits.ConnMode <= ENABLE_PREV_CONN)
            {
                InfoPropioNodo.Capability.bits.AllowJoin = 1;
            }
            //InfoPropioNodo.Capability.bits.Direct = //REVISAR no tiene sentido en el
            //nodo propio.no?
            //InfoPropioNodo.Capability.bits.altSrcAddr = //REVISAR ya he puesto que
            //se guarde la larga al principio de este método.
            //no?
            break;
        case AdditionalOwnNode:
            for (i = 0; i < ADDITIONAL_NODE_ID_SIZE; i++)
            {
                InfoPropioNodo.PeerInfo[i] = AdditionalNodeID[i];
            }
            /*Fin de la info tipica de active scan.*/
            break;
        case PermOwnNode:
            /*Info de los permisos del nodo*/
            for (i = 0; i < MAX_NUMERO_PERM_NODOS; i++)
            {
                Repo_Tabla_Permo_Acc[i] = TABLA_PERM_ACC[i];
            }
            /*Fin de la info de los permisos del nodo propio.*/
            break;
        case PolicOwnNode:
            /*Info de las politicas*/
            Repo_Poli_Weights = Reparto_Pesos;
            /*Fin de info de las políticas*/
            break;
        case 0x05:
            break;
        case IncluirPotencia:
            for(i = 0; i < MAX_VECTOR_POTENCIA; i++){
                if(i < (MAX_VECTOR_POTENCIA - 1)){
                    vectorPotencias[i] = vectorPotencias[i+1];
                } else {
                    vectorPotencias[i] = *((BYTE*)(Peticion->Param2));
                }
            }
            break;
        case EnvRTx:
            CRM_Repo_NRTx(*((BYTE*)(Peticion->Param1)),*((BYTE*)(Peticion->Param2)), *((BYTE*)(Peticion->Param3)));
            break;
        default:
            break;
    }

    /*Info de la hora*/
        RtccGetTimeDate(&RepoNodPropioTime, &RepoNodPropioDate);
    /*Fin info de la hora*/

}

BOOL CRM_Repo_NodosRed(REPO_MSSG_RCVD *Peticion)
{
    BYTE i;
    for (i = 0; i < CONNECTION_SIZE; i++)
    {
        if (isSameAddress(Peticion->EUINodo, ConnectionTable[i].Address))
        {
                switch (*((REPOSUBDATANETNODE*) (Peticion->Param1)))
                {
                    case AdditionalNetNode:
#if defined(TEST6)

                        Repo_Conn_Table[*((BYTE*) (Peticion->Param2))].PeerInfo[0] = *((BYTE*) (Peticion->Param3)); //Peticiones enviadas.
                        Repo_Conn_Table[*((BYTE*) (Peticion->Param2))].PeerInfo[1] = *((BYTE*) (Peticion->Param4)); //Peticiones aceptadas.

/*ESTO ERA CUANDO IBA A RECALCULAR LA PROBABILIDAD EN EL REPOSITORIO.*/
//                        if (*((BOOL*) (Peticion->Param2)) == TRUE)
//                        {
//                            //TODO actualizar la probabilidad de cambio que viene en param3
//                            /*TODO mejor no lo pasamos en param3 si no que recalculamos,
//                             ya que puede que la probabilidad de cambio de un nodo no sea
//                             absoluta si no que dependa de quien le pregunte.*/
//                            Repo_Conn_Table[i].PeerInfo[0] = (Repo_Conn_Table[i].PeerInfo[0] + 1)*100 / (NumeroDePeticionesDeCambio + 1);
//                                /*En una busqueda "rapida" no he encontrado que la tabla de
//                                 conexiones se actualice así que por eso la toco yo a piñon.*/
//                            //ConnectionTable[i].PeerInfo = (BYTE*) Peticion->Param3;
//                            //Repo_Conn_Table[i].PeerInfo = (BYTE*)Peticion->Param3;
//                        }
//                        else
//                        {
//                        //TODO eliminar la entrada de la tabla de conexiones.
//                            Repo_Conn_Table[i].PeerInfo[0] = (Repo_Conn_Table[i].PeerInfo[0])*100 / (NumeroDePeticionesDeCambio + 1);
//                            MiApp_RemoveConnection(i); /*La manera limpia de hacerlo.*/
////                            ConnectionTable[i].status.bits.isValid = 0;
//                        }
#endif
                        break;
                    case RSSINetNode:
                    {
                        //Le he pasado en Param2 el vector con todos los RSSI.
                        //Formato: RSSIResultado868, CanalOptimo868, RSSIResultado2400, CanalOptimo2400, CanalOptimo, ri
                        MIWI868_RSSI_optimo_ext[i] = *(BYTE*)(Peticion->Param2);
                        MIWI868_canal_optimo_ext[i] = *(BYTE*)(Peticion->Param2 + 1);
                        MIWI2400_RSSI_optimo_ext[i] = *(BYTE*)(Peticion->Param2 + 2);
                        MIWI2400_canal_optimo_ext[i] = *(BYTE*)(Peticion->Param2 + 3);
                        CanalOptimoExt[i] = *(BYTE*)(Peticion->Param2 + 4);
                        riCanalOptimoExt[i] = *(BYTE*)(Peticion->Param2 + 5);
                        BYTE RSSI_canal_optimo;
                        switch(riCanalOptimoExt[i]){
                            case MIWI_0868:
                                RSSI_canal_optimo = MIWI868_canal_optimo_ext[i];
                                break;
                            default:
                                RSSI_canal_optimo = MIWI2400_canal_optimo_ext[i];
                                break;
                        }
                        //Manda un mensaje a Optm diciendole que ha llegado un mensaje de cambio de canal.
                        BYTE InfoCambio[] = {3, *(BYTE*)(Peticion->Param2 + 4), *(BYTE*)(Peticion->Param2 + 5), RSSI_canal_optimo};
                        OPTM_MSSG_RCVD PeticionProcMensCambio;
                        BYTE ProcReq;
                        if(EstadoGT == EsperandoDecFinal){
                            ProcReq = ProcDecFinal;
                        } else if(EstadoGT == EsperandoDecisionRestoNodos){
                            Printf("\r\nDos nodos han decidido cambiar de canal al mismo tiempo. Se vuelve al principio.");
                            ProcReq = ProcRespCambio;
                            limpiaBufferRX();
                        } else {
                            ProcReq = ProcCambioCanal;
                        }                        
                        PeticionProcMensCambio.Action = ActProcRq;
                        PeticionProcMensCambio.Param1 = &ProcReq;
                        PeticionProcMensCambio.Param2 = &InfoCambio;
                        PeticionProcMensCambio.Transceiver = Peticion->Transceiver;
                        CRM_Message(NMM, SubM_Opt, &PeticionProcMensCambio);
                        
                        break;
                    }    
                    case AllNetNode:
                        break;
                    default:
                        break;
                }
        }
    }

    RtccGetTimeDate(&RepoNodRedTime, &RepoNodRedDate);
/*Fin info de la hora*/
    return TRUE;
}

void CRM_Repo_NodosEnv(void)
{
    BYTE i;
    /*Copia de la info del ActiveScan*/
    for( i=0; i < ActiveScanResultIndex; i++)
    {
        Repo_Activ_Scan[i] = ActiveScanResults[i];
    }
    /*Fin de la copia del ActiveScan.*/
    
    /*Info de la hora*/
    RtccGetTimeDate(&RepoNodEnvTime, &RepoNodEnvDate);
    /*Fin info de la hora*/
}

void CRM_Repo_Env(BYTE canal, BYTE InfoRSSI)
{
    //REVISAR. Esto solo vale para una entrada obviamente habrá que hacerlo
    //para un array.
    Repo_EvalCanl.CanalEval = canal;
    Repo_EvalCanl.EvalRSSI = InfoRSSI;
    /*Info de la hora*/
    RtccGetTimeDate(&Repo_EvalCanl.RepoEnvTime, &Repo_EvalCanl.RepoEnvDate);
    /*Fin info de la hora*/
}

void CRM_Repo_NRTx(BYTE n_rtx, BYTE canal, radioInterface ri){
    switch(ri){
        case MIWI_0434:
            MIWI434_rtx[canal] = n_rtx;
            break;
        case MIWI_0868:
            MIWI868_rtx[canal] = n_rtx;
            break;
        case MIWI_2400:
            MIWI2400_rtx[canal] = n_rtx;
            break;
    }
}

void CRM_Repo_Mensajes_Intercambiados(BYTE *Address)
{
    BYTE i;
    for(i = 0; i < CONNECTION_SIZE; i++){
        if(isSameAddress(Address, ConnectionTable[i].Address)){
            NumMssgIntercambiados[i]++;
            break;
        }
    }
}

void CRM_Repo_Str_RSSI(radioInterface ri){
    BYTE i;
    switch(ri){
        case MIWI_0434:
            for(i = 0; i < MIWI0434NumChannels; i++){
                GetScanResult(ri, i, &MIWI434_RSSI_values[i]);            
            }
            break;
        case MIWI_0868:
            for(i = 0; i < MIWI0868NumChannels; i++){
                GetScanResult(ri, i, &MIWI868_RSSI_values[i]);                
            }
            break;
        case MIWI_2400:
            for(i = 0; i < MIWI2400NumChannels; i++){
                GetScanResult(ri, i, &MIWI2400_RSSI_values[i]);
            }
            break;             
    }
}
//PRUEBA TEST5
void CRM_Repo_ProbCambio(BYTE Valor)
{
    Repo_Conn_Table[0].PeerInfo[0] = Valor;
}

/*Fin de las funciones internas*/

void inicializarTablaAtacantes(){
    BYTE i,j,k;
    BYTE TablaDirecciones[CONNECTION_SIZE+1][MY_ADDRESS_LENGTH];

    for (i = 0; i < CONNECTION_SIZE+1; i++){
        if (i < CONNECTION_SIZE && CONNECTION_SIZE != 0){
            for (k = 0; k < MY_ADDRESS_LENGTH; k++){
                TablaDirecciones[i][k] = ConnectionTable[i].Address[k];
            }
        } else {
            for (k = 0; k < MY_ADDRESS_LENGTH; k++){
                TablaDirecciones[i][k] = myLongAddress[k];
            }
        }

    }
    
    for (i = 0; i < CONNECTION_SIZE+1; i++){
        for (j = i*(CONNECTION_SIZE+1); j < (i*(CONNECTION_SIZE+1))+CONNECTION_SIZE+1; j++){
            for (k = 0; k < MY_ADDRESS_LENGTH; k++){
                Tabla_Atacantes[j].direccionAtacante[k] = TablaDirecciones[i][k];
            }
            Tabla_Atacantes[j].esAtacante = 0;
        }
    }

    j = 0;
    for (i = 0; i < (CONNECTION_SIZE+1)*(CONNECTION_SIZE+1); i++){
        if (j < CONNECTION_SIZE+1){
            for (k = 0; k < MY_ADDRESS_LENGTH; k++){
                Tabla_Atacantes[i].direccionDetector[k] = TablaDirecciones[j][k];
            }
            j++;
        } else if (j == CONNECTION_SIZE+1){
            j = 0;
            for (k = 0; k < MY_ADDRESS_LENGTH; k++){
                Tabla_Atacantes[i].direccionDetector[k] = TablaDirecciones[j][k];
            }
            j++;
        }
    }
}

BOOL CRM_Repo_Reiniciar_Potencias(void){
    BYTE i;
    for (i = 0; i < MAX_VECTOR_POTENCIA; i++){
        vectorPotencias[i] = 0xFF;
    }
}

BOOL CRM_Repo_Reiniciar_RTx(void){
    BYTE i;
    #ifdef MIWI_0868_RI
    for(i = 0; i < MIWI0868NumChannels; i++){
        MIWI868_rtx[i] = 0;
    }
    #endif
    #ifdef MIWI_2400_RI
    for(i = 0; i < MIWI2400NumChannels; i++){
        MIWI2400_rtx[i] = 0;
    }
    #endif
}

void CRM_Repo_Guardar_Coordenadas(REPO_MSSG_RCVD *Peticion){
    paquetesRecibidos = *(BYTE*)(Peticion->Param1);
    Lista_Paq_Rec_Aprendizaje[paquetesRecibidos].tiempo = *(BYTE*)(Peticion->Param2);
    Lista_Paq_Rec_Aprendizaje[paquetesRecibidos].RSSI = *(BYTE*)(Peticion->Param3);
}

BOOL CRM_Repo_Proc_Mens_Att(REPO_MSSG_RCVD *Peticion){
    Printf("\r\nRecibido mensaje de atacante.");
    BYTE dirAtt[MY_ADDRESS_LENGTH], dirDet[MY_ADDRESS_LENGTH], i;
    BYTE nDetectado = 0;
    for(i = 0; i < MY_ADDRESS_LENGTH; i++){
        dirAtt[i] = *(BYTE*)(Peticion->Param2 + i);
    }
    for(i = 0; i < MY_ADDRESS_LENGTH; i++){
        dirDet[i] = *(BYTE*)(Peticion->Param2 + i + MY_ADDRESS_LENGTH);
    }
    for (i = 0; i < (CONNECTION_SIZE+1)*(CONNECTION_SIZE+1); i++){
        if (isSameAddress(Tabla_Atacantes[i].direccionAtacante, dirAtt) && isSameAddress(Tabla_Atacantes[i].direccionDetector, dirDet)){
            if(Tabla_Atacantes[i].esAtacante == 1){
                Printf("\r\nYa habia recibido ese mensaje.");
                return FALSE;
            } else {            
                Tabla_Atacantes[i].esAtacante = 1;
            }
        }
        if (isSameAddress(Tabla_Atacantes[i].direccionAtacante, dirAtt) && Tabla_Atacantes[i].esAtacante == 1){
            nDetectado++;
        }        
    }

    if (nDetectado >= 1){
        Printf("\r\nVarios nodos han detectado al mismo atacante. Se desconecta de la red.");
        //Se desconecta a ese de la red.
        //Se puede hacer cualquier otra cosa.
        
        /////////////MANDAR MENSAJE A EXECUTION/////////////////
        BYTE index;
        for (i = 0; i < CONNECTION_SIZE; i++){
            if(isSameAddress(dirAtt, ConnectionTable[i].Address)){
                index = i;
            }
        }
        EXEC_MSSG_RCVD PeticionDisconn;
        PeticionDisconn.Action = ActDisconn;
        PeticionDisconn.Param1 = index;
        CRM_Message(NMM, SubM_Exec, &PeticionDisconn);
        
    }
    //Guardo en la tabla quién es el atacante
    
    //Mando un mensaje por VCC al resto de nodos con la informacion del atacante.
    Printf("\r\nMando información del atacante y el detector al resto de nodos.");
    BYTE MensajeVCC[] = {VccCtrlMssg, SubM_Repo, ActStr, DetAtt, 0, dirAtt[0], dirAtt[1], 
                        dirAtt[2], dirAtt[3], dirAtt[4], dirAtt[5], dirAtt[6], dirAtt[7], 
                        dirDet[0], dirDet[1], dirDet[2], dirDet[3], dirDet[4], dirDet[5], 
                        dirDet[6], dirDet[7], riActual};
    MSN_MSSG_RCVD PeticionAtacante;
    VCC_MSSG_RCVD PeticionVCCAtacante;

    PeticionVCCAtacante.AddrMode = BROADCAST_ADDRMODE;
    PeticionVCCAtacante.Action = ActSend;
    PeticionVCCAtacante.BufferVCC = MensajeVCC;
    PeticionVCCAtacante.Transceiver = MIWI_0434;
    BYTE sizeBufferVCC = 22;
    PeticionVCCAtacante.Param1 = &sizeBufferVCC;

    PeticionAtacante.Peticion_Destino.PeticionVCC = &PeticionVCCAtacante;

    CRM_Message(VCC, SubM_Ext, &PeticionAtacante);
}

/*Funciones de inicializacion*/
BOOL CRM_Repo_Init(void)
{
    //TODO inicializar el repo.
//    /*Inicializacion de info del nodo propio*/
//        AdditionalNodeID[0] = 50; /*Aqui guardamos la probablidad de cambio
//                                   inicial.*/
//    /*Fin de la inicializacion de info del nodo propio.*/
    nClusters = 0;
//Inicializar la info de los nodos de la red. Una vez inicializada la tocamos nosotros.
    /*Copia de la tabla de conexiones*/
    BYTE i;
        for( i=0; i < CONNECTION_SIZE; i++)
        {
            Repo_Conn_Table[i] = ConnectionTable[i];
        }
    /*Fin de la tabla de conexiones.*/

    NumeroDePeticionesDeCambio = NumeroDePeticionesInicial;


    /*Inicialización vector potencias*/
    /*
    for (i = 0; i < MAX_VECTOR_POTENCIA; i++){
        vectorPotencias[i] = 0xFF;
    }
*/
    /*Inicialización número mensajes intercambiados*/
    for(i = 0; i < CONNECTION_SIZE; i++){
        NumMssgIntercambiados[i] = 0;
    }
    
    /*Inicialización de la interfaz de cada uno de los nodos*/
    for(i = 0; i < CONNECTION_SIZE; i++){
        Repo_Conn_Table[i].PeerInfo[0] = riActual;
        Repo_Conn_Table[i].PeerInfo[1] = GetOpChannel(riActual);
    }    

    /*Inicialización tablas de retransmisiones*/
    #ifdef MIWI_0434_RI
    for(i = 0; i < MIWI0434NumChannels; i++){
        MIWI434_rtx[i] = 0;
    }
    #endif
    #ifdef MIWI_0868_RI
    for(i = 0; i < MIWI0868NumChannels; i++){
        MIWI868_rtx[i] = 0;
    }
    #endif
    #ifdef MIWI_2400_RI
    for(i = 0; i < MIWI2400NumChannels; i++){
        MIWI2400_rtx[i] = 0;
    }
    #endif

    return TRUE;
}
/*Fin de las funciones de inicializacion*/

/*****************************************************************************/
/******************************FIN DE FUNCIONES*******************************/
/*****************************************************************************/
