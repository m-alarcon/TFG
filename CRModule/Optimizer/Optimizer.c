/*****************************************************************************
 *
 *              Optimizer.c -- Optimizer Module v0.1
 *
 *****************************************************************************
 *
 * Author:          Guillermo Jara Luengos
 * FileName:        Optimizer.c
 * Dependencies:    Optimizer.h
 * Processor:
 * BOARD:
 * Compiler:        C32 01.00.02 or higher
 * Linker:          MPLINK 03.40.00 or higher
 * Company:         B105 -FAILURE IS NOT AN OPTION-
 *
 *****************************************************************************
 * File Description:
 *
 * El modulo Messenger encargado de gestionar las peticiones entre modulos.
 *
 * Change History:
 *  Rev   Date(m/d/y)   Description
 *  0.1   23/10/2012    Initial revision
 *****************************************************************************/
#include "CRModule/Optimizer/Optimizer.h"
#include "CRModule/Optimizer/ConfigOptimizer.h"
#include "../NODE FW.X/Aplicacion.h"

/*****************************************************************************/
/*********************************VARIABLES***********************************/
/*****************************************************************************/

//Para configurar cada cuanto tiempo realiza su rutina de ejecucion Optimizer,
//que recordemos que es la rutina de ejecucion periodica y principal del modulo
//cognitivo.
WORD Periodomseg;
WORD Cuentamseg; //Para la cuenta de los mseg que han pasado.

WORD mseg;  //Para la interrupcion del timer 5.
WORD Periodo;

WORD tiempoCambio;
WORD tiempoCambioPotTX;
WORD tiempoCambioFrecPaquetes;

extern BOOL EnviandoMssgApp;/*Para  la preuba de que mientras se envían datos
                             /*de appliacion no se puede realizar la estrategia
                             de optimizacion. Me refiero a justo en el momento
                             en el que se tocan funciones de miapp que pueden
                             tocar registros conflictivos. Ver enviodatos.c*/
extern BOOL RecibiendoMssg;/*PArecido a lo de arriba.*/
extern BYTE TablaConexionesInicial[MIWI_CONN_ENTRY_SIZE*CONNECTION_SIZE];

//#if defined(TEST4)
BYTE BufferPrueba[RX_BUFFER_SIZE];
BYTE Direccion[MY_ADDRESS_LENGTH];
RECEIVED_MESSAGE BufferRecepcionPrueba;
VCC_MSSG_RCVD PeticionRecepcion; //La del VCC
OPTM_MSSG_RCVD PeticionTest2; //La del Optmizer
//#endif

#if defined TEST5
    MIWI_TICK t1G, t2G;
    DWORD_VAL tTG;
#endif

//#if defined TEST6
    OPTSTATMACH OptmEstado;
    WORD MilisDeTimeOut;
    BYTE PeticionesEnviadas;
    BYTE PeticionesAceptadas;
    WORD ProbabilidadDeCambio;
//#endif

extern radioInterface ri;
extern radioInterface riActual;
extern BYTE canalCambio;
extern BYTE BackupCanal;
extern radioInterface riData;
BYTE num_rtx;
BYTE numNodosRed;
//extern BYTE VCCmssg;

#if defined NODE_1
BYTE EUINodoExt1[] = {EUI_0, EUI_1, EUI_2, EUI_3, EUI_4, EUI_5, EUI_6, 0x22};
BYTE EUINodoExt2[] = {EUI_0, EUI_1, EUI_2, EUI_3, EUI_4, EUI_5, EUI_6, 0x33};
#elif defined NODE_2
BYTE EUINodoExt1[] = {EUI_0, EUI_1, EUI_2, EUI_3, EUI_4, EUI_5, EUI_6, 0x11};
BYTE EUINodoExt2[] = {EUI_0, EUI_1, EUI_2, EUI_3, EUI_4, EUI_5, EUI_6, 0x33};
#elif defined NODE_3
BYTE EUINodoExt1[] = {EUI_0, EUI_1, EUI_2, EUI_3, EUI_4, EUI_5, EUI_6, 0x11};
BYTE EUINodoExt2[] = {EUI_0, EUI_1, EUI_2, EUI_3, EUI_4, EUI_5, EUI_6, 0x22};
#endif


//Variables para las diferentes estrategias de optimizacion.
    //Optimzier de Elena.
    BYTE CosteTx;
    BYTE CosteRx;
    BYTE CosteSensing;

    BYTE NumMsj = numMsjXDefecto;
    BYTE MaxRTx;
    BYTE SumRTx;
    BYTE MssgRTx[numMsjXDefecto]; //TODO molaría crear el tamaño dinamicamente con
        //malloc y free en función de NumMsj, de momento lo ponemos fijo.
    BYTE MssgIndex = 0;

    BYTE CosteCh;
    BYTE CosteO;//Coste de tx en el actual que se supone un poco saturado.
    BYTE CosteN;//Coste de no cambiar de canal a pesar de que el compañero lo
                //haga.

    WORD ProbChngCompi;
    WORD ProbChngPropia;
    //Fin de optimizer de Elena.

    //Teoria de juegos    
    BYTE nRespuestas;
    BYTE nRespuestasAfirmativas;
    BYTE nRespuestasNegativas;
    BYTE n_msg = 3; /*Numero de mensajes necesarios para cambiar de canal. Cambiar con el número que sea realmente*/
    WORD CosteCambio, CosteOcupado, CosteNoCambio;
    
    //Data Clustering
    double learningTimeMax;
    double learningTime;
    double reinicioAtacantesTimeMax;//Para reiniciar cada 5 minutos
    BYTE aprendizaje;
    BYTE normalizado;
    BYTE clustersDone;
    
    MIWI_TICK TiempoAnterior;
    double initRad = 0.3;
    double tiempoMax;
    double potenciaMax;
    int paquetesRecibidos;
    int nClusters = 0;
    cl cluster;
    
    LONG tiempo;

/*****************************************************************************/
/******************************FIN DE VARIABLES*******************************/
/*****************************************************************************/

/*****************************************************************************/
/*********************************FUNCIONES***********************************/
/*****************************************************************************/

/*De recepcion/interfaz con Messenger*/
BOOL CRM_Optm_Mssg_Rcvd(OPTM_MSSG_RCVD *Peticion)
{
    switch(Peticion->Action)
    {
        case(ActGameTh):
            CRM_Optm_GameTheory(Peticion);
            break;
        case(ActProcRq):
            CRM_Optm_Processor(Peticion);
            break;
        case(ActCons):
            CRM_Optm_Cons(Peticion);
            break;
        default:
            break;
    }
    return TRUE;
}
/*Fin de funciones de recepcion/interfaz con Messenger*/

/*De funcionalidad propia del sub-modulo*/
//Es el procesador de mensajes y peticiones que proceden de otros nodos. Es
//decir, es el encargado de decidir si cambiar o no de canal cuando otro nodo se
// lo solicita. Tambien debe por ejemplo decidir que hacer si recibe un mensaje
//de otro nodo diciendole que aquel no va a cambiar de canal (respondiendo a una
// peticion que hayamos hecho.
BOOL CRM_Optm_Processor(OPTM_MSSG_RCVD *Peticion)
{
    switch(*((BYTE*)(Peticion->Param1)))
    {
        case 0x00:
            break;
        case ProcAsk4Chng: //Aqui procesamos la peticion de cambio de canal.
        {
//#if defined(TEST6)
            /***************************VARIABLES******************************/
            //Un par de tipos de mensaje que utilizaremos para responder.
            MSN_MSSG_RCVD PeticionNodoExt;
            VCC_MSSG_RCVD PeticionNodoExtVCC;
            //Y un tipo para el executor.
            EXEC_MSSG_RCVD PeticionCambio;
            /***************************FIN VARIABLES**************************/

            /*******************************CUERPO*****************************/
            if(OptmEstado == WaitinAnsw4ChngChn)
            {
                //TODO implementar estrategia para resolver el conflicto.
                BYTE i;
                for (i=0; i < MY_ADDRESS_LENGTH; i++)
                {
                    if(Peticion->EUINodo[MY_ADDRESS_LENGTH - i - 1] < GetMyLongAddress(MY_ADDRESS_LENGTH - i -1))
                    {
                        break;
                    }
                }
                OptmEstado = Idle;
                MilisDeTimeOut = 0;
            }
            /*Comprobamos en nuestras politicas si realizamos el cambio.*/
                /*********MESSAGE CREATION*******/
                POLI_MSSG_RCVD PeticionPolitica;
                BYTE PoliOutputIntrChng;
                PeticionPolitica.Action = ActRead;
                PeticionPolitica.DataType = TypeIntrChng;
                PeticionPolitica.Param1 = &PoliOutputIntrChng;/*Chequeamos por ejemplo
                                                               la propension al cambio
                                                               de canal.*/
                /*********FIN MESSAGE CREATION*******/
            CRM_Message(NMM, SubM_Poli, &PeticionPolitica);
            /*Fin de la comprobacion en las politicas.*/
            //Y ahora comprobamos el resultado.
            if(*((BYTE*)PeticionPolitica.Param1) < 10) /*Si es menor que 10 la predisposicion
                                                 al cambio de canal no lo hacemos.*/
NOACEPTA: //Si no queremos notifcar el no cambio comentariamos y dejaríamos solo el break.
            {
                /*Creamos la respuesta para el nodo externo y la enviamos.*/
                    /*********MESSAGE CREATION*******/
                    BYTE PeticionNodoExtBuff[] = {VccCtrlMssg, SubM_Opt, ActProcRq, ProcChangAnsw, FALSE, FALSE, FALSE, ri};
                    PeticionNodoExtVCC.DirNodDest = Peticion->EUINodo;
                    PeticionNodoExtVCC.Action = ActSend;
                    PeticionNodoExtVCC.BufferVCC = PeticionNodoExtBuff;
                    PeticionNodoExtVCC.Transceiver = ri;//Peticion->Transceiver;
                    BYTE sizeOfBufferVCC = 6;
                    PeticionNodoExtVCC.Param1 = &sizeOfBufferVCC;
                    PeticionNodoExt.Peticion_Destino.PeticionVCC = &PeticionNodoExtVCC;
                    /*********FIN MESSAGE CREATION*******/
                CRM_Message(VCC, SubM_Ext, &PeticionNodoExt);
                /*Fin de la respuesta para el nodo externo.*/
                break;
            }
            //Si la comprobacion es mayor que 10 hacemos todo el resto.
            else if(!CRM_Optm_CostAlgorithm(CosteN, CosteO, CosteCh, 1)) /*Como el que nos lo ha pedido
                                                                     sabemos que cambia su prob es
                                                                     1.*/
            {
             goto NOACEPTA;
            }
            else
            {
                /*Creamos un Message para executor para cambiar de canal.*/
                    /*********MESSAGE CREATION*******/
                    PeticionCambio.Action = ActChnHop;
                    PeticionCambio.Param1 = *((BYTE*)(Peticion->Param2));
                    PeticionCambio.Transceiver = Peticion->Transceiver;
                    ri = Peticion->Transceiver;//XXX
                    /*********FIN MESSAGE CREATION*******/
                CRM_Message(NMM, SubM_Exec, &PeticionCambio);
                /*Fin del Message para executor.*/
                WORD riname;
                switch (ri) {
                    case MIWI_0434: riname = 434; break;
                    case MIWI_0868: riname = 868; break;
                    case MIWI_2400: riname = 2400; break;
                }
                char traza[80];
                sprintf(traza, "\r\n\r\nCambiado al canal %d de la interfaz %d.",
                        GetOpChannel(ri), riname);
                Printf(traza);
                /*Creamos la respuesta para el nodo externo y la enviamos.*/
                    /*********MESSAGE CREATION*******/
                    BYTE PeticionNodoExtBuff[] = {VccCtrlMssg, SubM_Opt, ActProcRq, ProcChangAnsw, TRUE, TRUE, TRUE, ri};
                    PeticionNodoExtVCC.DirNodDest = Peticion->EUINodo;
                    PeticionNodoExtVCC.Action = ActSend;
                    PeticionNodoExtVCC.BufferVCC = PeticionNodoExtBuff;
                    PeticionNodoExtVCC.Transceiver = ri;//Peticion->Transceiver;
                    BYTE sizeOfBufferVCC = 6;
                    PeticionNodoExtVCC.Param1 = &sizeOfBufferVCC;
                    PeticionNodoExt.Peticion_Destino.PeticionVCC = &PeticionNodoExtVCC;
                    /*********FIN MESSAGE CREATION*******/
                    CRM_Message(VCC, SubM_Ext, &PeticionNodoExt);
                /*Fin de la respuesta para el nodo externo.*/

#if defined(TEST5)
                t2G = MiWi_TickGet();
                tTG.Val = MiWi_TickGetDiff(t2G, t1G);
#endif
                /******************************FIN CUERPO**************************/
                break;
            }
//#endif
        }
        case ProcChangAnsw: //Aqui procesamos la respuesta que recibimos aceptando o negando el cambio de canal.
        {
            //TODO debemos empaquetar la respuesta Podemos haberla puesto en el siguiente byte.
//#if defined(TEST6)
            /*Creamos la peticion para saber el numero de veces que le hemos preguntado y
             cuantas ha aceptado el nodo.*/
            /***************************VARIABLES******************************/
//                REPO_MSSG_RCVD PeticionProb;
            BYTE IndiceTabla;
            REPO_MSSG_RCVD PeticionActualProb;
            REPO_MSSG_RCVD PeticionProb;
            REPOSUBDATANETNODE TipoDeSubDato;
            BYTE PeticionesEnviadasProv;
            BYTE PeticionesAceptadasProv;

            /***************************FIN VARIABLES**************************/

            Printf("\r\nRespuesta recibida al cambio de canal ...");
                /*********MESSAGE CREATION*******/
                TipoDeSubDato = AdditionalNetNode;
                IndiceTabla = MiWi_Search4ShortAddress(Peticion->Transceiver, Peticion->EUINodo, Repo_Conn_Table); /*XXX-GuilJa REVISAR
                                                                                                   //porque la función busca
                                                                                                   //direcciones cortas.*/

                PeticionProb.Action = ActSndDta;
                PeticionProb.DataType = NetNode;
                PeticionProb.EUINodo = Peticion->EUINodo;
                PeticionProb.Param1 = &TipoDeSubDato;
                PeticionProb.Param2 = &IndiceTabla;//XXX
                PeticionProb.Param3 = &PeticionesEnviadas;
                PeticionProb.Param4 = &PeticionesAceptadas;
                PeticionProb.Transceiver = Peticion->Transceiver;//XXX
//                *((REPOSUBDATANETNODE*)(PeticionProb.Param2)) = AdditionalNetNode;
                /*********FIN MESSAGE CREATION*******/
            CRM_Message(NMM, SubM_Repo, &PeticionProb);

//            PeticionesEnviadas = *((BYTE*) (PeticionProb.Param3));
//            PeticionesAceptadas = *((BYTE*) (PeticionProb.Param4));

            if (PeticionesEnviadas == 255)
            {
                PeticionesEnviadas /= 10;
                PeticionesAceptadas = (PeticionesAceptadas / 10) == 0 ? 1 : (PeticionesAceptadas/10); /*XXX-GuilJa para que
                                                                                                       la probabilidad no se
                                                                                                       haga 0.*/
            }
            if(*(BYTE*)(Peticion->Param2) == TRUE)
            {
                Printf("AFIRMATIVO.\r\n");
                char Print[80];
                /*Realizamos el calculo con el param4 que es donde tenemos los datos necesarios.*/
                PeticionesEnviadas = PeticionesEnviadas + 1;
                sprintf(Print, "PeticionesEnviadas %d.", PeticionesEnviadas);
                Printf(Print);
                PeticionesAceptadas = PeticionesAceptadas + 1;
                ProbabilidadDeCambio = ((PeticionesAceptadas)*100)/(PeticionesEnviadas);

            }
            else
            {
                /*Realizamos el calculo con el param4 que es donde tenemos los datos necesarios.*/
                PeticionesEnviadas += 1;
                ProbabilidadDeCambio = (PeticionesAceptadas)*100/(PeticionesEnviadas);

                Printf("NEGATIVO.\r\n");
                //Pero RemoveConnection manda un mensaje y como ya hemos cambiado de canal y el otro no, no le
                //va a llegar, asi que esto habra que cambiarlo.
//                MiApp_RemoveConnection(0);
//                            ConnectionTable[i].status.bits.isValid = 0;
            }

                /*********MESSAGE CREATION*******/
                TipoDeSubDato = AdditionalNetNode;
                PeticionActualProb.Action = ActStr;
                PeticionActualProb.EUINodo = Peticion->EUINodo;
                PeticionActualProb.DataType = NetNode;
                PeticionActualProb.Transceiver = Peticion->Transceiver;//XXX
                PeticionActualProb.Param1 = &TipoDeSubDato;
                PeticionActualProb.Param2 = &IndiceTabla; //XXX
                PeticionActualProb.Param3 = &PeticionesEnviadas;
                PeticionActualProb.Param4 = &PeticionesAceptadas;
                /*********FIN MESSAGE CREATION*******/
            CRM_Message(NMM, SubM_Repo, &PeticionActualProb);

            //TODO actualizar el valor que se utiliza para la estrategia de optimizacion.

            //Ya no esperamos respuesta asi que..
            OptmEstado = Idle;            
            MilisDeTimeOut = 0;
//#endif
            break;            
        }
//#endif
        case ProcCambioCanal:
        {
            CHNG_MSSG_RCVD = 1;
            if(EstadoGT == Clear){                
                Printf("\r\nSe ha recibido peticion de cambio de canal del otro nodo.");
                nRespuestasAfirmativas++;
                REPO_MSSG_RCVD PeticionRepoRTx;
                BYTE canal = GetOpChannel(riActual);
                PeticionRepoRTx.Action = ActSndDta;
                PeticionRepoRTx.DataType = EnvRTx;
                PeticionRepoRTx.Param1 = &canal;
                PeticionRepoRTx.Transceiver = riActual;

                CRM_Message(NMM, SubM_Repo, &PeticionRepoRTx);
                
                num_rtx = *(BYTE*)(PeticionRepoRTx.Param2);

                char traza[80];
                sprintf(traza, "\r\nHa habido %d retransmisiones en el canal actual.\r\n", num_rtx);
                Printf(traza);
                
                if(*(BOOL*)(Peticion->Param2) == FALSE){
                    Printf("\r\nEra una respuesta que ya se habia procesado.");

                    EstadoGT = Clear;
                    CHNG_MSSG_RCVD = 0;
                    inicioCambio = FALSE;
                    nRespuestas = 0;
                    nRespuestasAfirmativas = 0;
                    nRespuestasNegativas = 0;           
                    return FALSE;
                }
                
                BOOL cambio = CRM_Optm_Calcular_Costes(num_rtx);
                //Reinicio el numero de retransmisiones
                REPO_MSSG_RCVD PeticionRepoResetRTx;
                PeticionRepoResetRTx.Action = ActStr;
                PeticionRepoResetRTx.DataType = RstRTx;
                CRM_Message(NMM, SubM_Repo, &PeticionRepoResetRTx);
                if(cambio == TRUE){
                    Printf("\r\nNotifico al resto de nodos que quiero cambiar.");
                    //Notifico a los otros nodos que quiero cambiar de canal y hago 4)
                    //Informa al resto de nodos que se quiere cambiar de canal.
                    MSN_MSSG_RCVD PeticionCambioCanal;
                    VCC_MSSG_RCVD PeticionVCCCambioCanal;
                    BYTE MensajeVCC[] = {VccCtrlMssg, SubM_Opt, ActProcRq, ProcCambioCanal, TRUE, Peticion->Transceiver};
                    PeticionVCCCambioCanal.AddrMode = BROADCAST_ADDRMODE;
                    PeticionVCCCambioCanal.Action = ActSend;
                    PeticionVCCCambioCanal.BufferVCC = MensajeVCC;
                    PeticionVCCCambioCanal.Transceiver = MIWI_0434;
                    BYTE sizeBufferVCC = 6;
                    PeticionVCCCambioCanal.Param1 = &sizeBufferVCC;

                    PeticionCambioCanal.Peticion_Destino.PeticionVCC = &PeticionVCCCambioCanal;

                    CRM_Message(VCC, SubM_Ext, &PeticionCambioCanal);

                    //Mando mensaje a optimizer para ejecutar GT
                    OPTM_MSSG_RCVD PeticionCambio;
                    PeticionCambio.Action = SubActRespCambio;
                    PeticionCambio.Transceiver = Peticion->Transceiver;
                    PeticionCambio.Param1 = Peticion->Param2;
                    CRM_Optm_Cons(&PeticionCambio);

                    Printf("\r\nTimeout a 0");
                    MilisDeTimeOut = 0;
            
                    EstadoGT = EsperandoDecisionRestoNodos;

                } else {
                    Printf("\r\nNotifico al resto de nodos que no quiero cambiar.");
                    MSN_MSSG_RCVD PeticionCambioCanal;
                    VCC_MSSG_RCVD PeticionVCCCambioCanal;
                    BYTE MensajeVCC[] = {VccCtrlMssg, SubM_Opt, ActProcRq, ProcCambioCanal, FALSE, Peticion->Transceiver};

                    PeticionVCCCambioCanal.Action = ActSend;
                    PeticionVCCCambioCanal.BufferVCC = MensajeVCC;
                    PeticionVCCCambioCanal.AddrMode = BROADCAST_ADDRMODE;
                    PeticionVCCCambioCanal.Transceiver = MIWI_0434;
                    BYTE sizeBufferVCC = 6;
                    PeticionVCCCambioCanal.Param1 = &sizeBufferVCC;

                    PeticionCambioCanal.Peticion_Destino.PeticionVCC = &PeticionVCCCambioCanal;

                    CRM_Message(VCC, SubM_Ext, &PeticionCambioCanal);

                    //Se devuelven todas las variables usadas al estado inicial
                    EstadoGT = Clear;
                    CHNG_MSSG_RCVD = 0;
                    inicioCambio = FALSE;
                    nRespuestas = 0;
                    nRespuestasAfirmativas = 0;
                    nRespuestasNegativas = 0;
                }
            } else if (EstadoGT == EsperandoDecisionRestoNodos){
                nRespuestas++;
                BOOL cambio = *((BOOL*)(Peticion->Param2));
                if(cambio == FALSE){
                    nRespuestasNegativas++;                    
                    REPO_MSSG_RCVD PeticionRepoMssg;
                    PeticionRepoMssg.Action = ActSndDta;
                    PeticionRepoMssg.DataType = EnvNMsg;
                    PeticionRepoMssg.Param1 = Peticion->EUINodo;                

                    CRM_Message(NMM, SubM_Repo, &PeticionRepoMssg);
                    
                    Printf("\r\nEl numero de mensajes intercambiados con ese nodo es: ");
                    double nMensIntercambiados = *(BYTE*)(PeticionRepoMssg.Param2);
                    PrintChar(nMensIntercambiados);                    
                    double MensajesTotales = GetProcPckts(Peticion->Transceiver);
                    Printf("\r\nEl numero de mensajes totales en esta interfaz es: ");
                    PrintChar(MensajesTotales);
                    Printf("\r\nEl porcentaje de mensajes con ese nodo es: ");
                    double porcentaje = ((nMensIntercambiados/MensajesTotales)*100);
                    PrintChar(porcentaje); 
                    
                    
                    
                    if(porcentaje >= PorcentajeMinimo){//Si el numero de mensajes que me intercambio con este es muy grande no me cambio
                        Printf("\r\nUn nodo con el que me comunico mucho no quiere cambiar.");
                        Printf("\r\nNotifico al resto de nodos que no quiero cambiar.");
                        MSN_MSSG_RCVD PeticionCambioCanal;
                        VCC_MSSG_RCVD PeticionVCCCambioCanal;
                        BYTE MensajeVCC[] = {VccCtrlMssg, SubM_Opt, ActProcRq, ProcCambioCanal, FALSE, Peticion->Transceiver};

                        PeticionVCCCambioCanal.Action = ActSend;
                        PeticionVCCCambioCanal.BufferVCC = MensajeVCC;
                        PeticionVCCCambioCanal.AddrMode = BROADCAST_ADDRMODE;
                        PeticionVCCCambioCanal.Transceiver = MIWI_0434;
                        BYTE sizeBufferVCC = 6;
                        PeticionVCCCambioCanal.Param1 = &sizeBufferVCC;

                        PeticionCambioCanal.Peticion_Destino.PeticionVCC = &PeticionVCCCambioCanal;

                        CRM_Message(VCC, SubM_Ext, &PeticionCambioCanal);
                                            
                        EstadoGT = Clear;
                        CHNG_MSSG_RCVD = 0;
                        inicioCambio = FALSE;
                        nRespuestas = 0;
                        nRespuestasAfirmativas = 0;
                        nRespuestasNegativas = 0;
                    } else {
                        Printf("\r\nRespuesta negativa de un nodo con el que no me comunico mucho.");
                        MilisDeTimeOut = 0;
                        if(nRespuestasNegativas == CONNECTION_SIZE){                        
                            Printf("\r\nTodos los nodos han rechazado cambiarse de canal. Me cambio solo."); 
                            
                            EXEC_MSSG_RCVD PeticionExec;
                            PeticionExec.OrgModule = SubM_Opt;
                            PeticionExec.Action = ActChnHop;
                            PeticionExec.Param1 = canalCambio;
                            PeticionExec.Transceiver = ri;

                            riActual = ri;

                            CRM_Message(NMM, SubM_Exec, &PeticionExec);//envio del messg.
                            /*Fin del Message para Executor.*/
                            WORD riname;
                            switch (ri) {
                                case MIWI_0434: riname = 434; break;
                                case MIWI_0868: riname = 868; break;
                                case MIWI_2400: riname = 2400; break;
                            }
                            char traza[80];
                            sprintf(traza, "\r\nCambiado al canal %d de la interfaz %d\r\n", GetOpChannel(ri), riname);
                            Printf(traza);                             
                            
                            EstadoGT = Clear;
                            CHNG_MSSG_RCVD = 0;
                            inicioCambio = FALSE;
                            nRespuestas = 0;
                            nRespuestasAfirmativas = 0;  
                            nRespuestasNegativas = 0;
                        }
                    }
                } else {
                    Printf("\r\nSe ha recibido una respuesta positiva.");
                    nRespuestasAfirmativas++;
                    MilisDeTimeOut = 0;
                }
                if (nRespuestasAfirmativas == CONNECTION_SIZE){
                    Printf("\r\nTodos los nodos con los que me comunico mucho quieren cambiar, pasamos a elegir el canal.");
                    Printf("\r\nTimeout a 0");
                    MilisDeTimeOut = 0;
                    EstadoGT = EsperandoDecFinal;
                    nRespuestas = 0;
                }
            }
            break;
        }
        case ProcRespCambio:
        {//Aquí voy a ver si el nodo ha aceptado la petición de cambio de canal y va a esperar a que le envíe el mensaje con el canal al que cambiar
            Printf("\r\nSe ha recibido la respuesta al cambio de canal.");//Puede ser un canal y el RSSI o TRUE que
                                                                          //significa que quiere cambiarse al mismo canal.
            if(EstadoGT == EsperandoDecFinal || EstadoGT == EsperandoDecisionRestoNodos){
                nRespuestas++;
                if(*((BOOL*)(Peticion->Param2)) == TRUE){
                    Printf("\r\nOtro nodo ha aceptado cambiar al canal propuesto.");
                    if(nRespuestasAfirmativas == nRespuestas){
                        BYTE MensajeVCC[] = {VccCtrlMssg, SubM_Opt, ActProcRq, ProcRespCambio, TRUE, Peticion->Transceiver};

                        MSN_MSSG_RCVD PeticionCambioCanal;
                        VCC_MSSG_RCVD PeticionVCCCambioCanal;

                        PeticionVCCCambioCanal.AddrMode = BROADCAST_ADDRMODE;
                        PeticionVCCCambioCanal.Action = ActSend;
                        PeticionVCCCambioCanal.BufferVCC = MensajeVCC;
                        PeticionVCCCambioCanal.Transceiver = MIWI_0434;
                        BYTE sizeBufferVCC = 6;
                        PeticionVCCCambioCanal.Param1 = &sizeBufferVCC;

                        PeticionCambioCanal.Peticion_Destino.PeticionVCC = &PeticionVCCCambioCanal;

                        CRM_Message(VCC, SubM_Ext, &PeticionCambioCanal);

                        Printf("\r\nTodos los nodos conectados y que quieren cambiar van a cambiar al canal propuesto, se cambia de canal.");
                        EXEC_MSSG_RCVD PeticionExec;
                        PeticionExec.OrgModule = SubM_Opt;
                        PeticionExec.Action = ActChnHop;
                        PeticionExec.Param1 = canalCambio;
                        PeticionExec.Transceiver = ri;
                        
                        riActual = ri;

                        CRM_Message(NMM, SubM_Exec, &PeticionExec);//envio del messg.
                        /*Fin del Message para Executor.*/
                        WORD riname;
                        switch (ri) {
                            case MIWI_0434: riname = 434; break;
                            case MIWI_0868: riname = 868; break;
                            case MIWI_2400: riname = 2400; break;
                        }
                        char traza[80];
                        sprintf(traza, "\r\nCambiado al canal %d de la interfaz %d y enviada "
                                "la peticion al otro nodo.\r\n", GetOpChannel(ri), riname);
                        Printf(traza);                                 
                        
                        SWDelay(1000);
                        //Se devuelven todas las variables usadas al estado inicial
                        EstadoGT = Clear;
                        CHNG_MSSG_RCVD = 0;
                        inicioCambio = FALSE;
                        nRespuestas = 0;
                        nRespuestasAfirmativas = 0;
                        nRespuestasNegativas = 0;
                    }
                } else {
                    //Se reinicia todo.      
                    EstadoGT = Clear;
                    CHNG_MSSG_RCVD = 0;
                    inicioCambio = FALSE;
                    nRespuestas = 0;
                    nRespuestasAfirmativas = 0;
                    nRespuestasNegativas = 0;                   
                }
            }
        }
        case ProcDecFinal:
            //Aqui va a coger el mensaje que le llega y va a decidir si se cambia al canal que le han mandado o al que habia decidido él.
            if(EstadoGT == EsperandoDecFinal){
                nRespuestas++;
                if(*((BYTE*)(Peticion->Param2)) == 3){//Compruebo que es un mensaje con info de cambio de canal.
                    Printf("\r\nOtro nodo ha aceptado cambiar de canal pero a otro."); 
                    
                    BOOL cambio = FALSE;
                    ri = *(BYTE*)(Peticion->Param2 + 2);
                    BYTE CanalOtroNodo = (*(BYTE*)(Peticion->Param2 + 1));
                    BYTE RSSICanalOtroNodo = (*(BYTE*)(Peticion->Param1 + 3));
                    WORD riname;
                    switch (ri) {
                        case MIWI_0434: riname = 434; break;
                        case MIWI_0868: riname = 868; break;
                        case MIWI_2400: riname = 2400; break;
                    }
                    char traza[80];
                    sprintf(traza, "\r\nQuiere cambiar al canal %d de la interfaz %d.\r\n", CanalOtroNodo, riname);
                    Printf(traza);
                    
                    //Guardo los RSSI propios de la interfaz que se quiere cambiar
                    REPO_MSSG_RCVD PeticionRepoRSSI;                
                    PeticionRepoRSSI.Action = ActStr;
                    PeticionRepoRSSI.DataType = SaveRSSI;
                    PeticionRepoRSSI.Param1 = &ri;

                    CRM_Message(NMM, SubM_Repo, &PeticionRepoRSSI);
                    
                    //Pido a repo el canal Optimo que he sacado antes de la ri que quiere cambiarse el otro nodo.
                    //Pido a repo el 2º mejor canal y el 3º.
                    BYTE MejorCanal, SegundoCanal, TercerCanal, CuartoCanal, position;
                    REPO_MSSG_RCVD PeticionRepoCanales;
                    PeticionRepoCanales.Action = ActSndDta;
                    PeticionRepoCanales.Transceiver = ri;
                    PeticionRepoCanales.DataType = EnvRSSI;
                    PeticionRepoCanales.Param1 = &ri;
                    //Se podría hacer un método en repo que directamente te devolviese los 4 primeros pero 
                    //entonces no sería tan ajustable como haciendolo así, si quieres comparar con más canales solo se lo pides
                    //y si quieres menos los borras.
                    position = 0;
                    PeticionRepoCanales.Param2 = &position;
                    CRM_Message(NMM, SubM_Repo, &PeticionRepoCanales);//Peticion mejor canal.
                    MejorCanal  = *(BYTE*)(PeticionRepoCanales.Param3);
                    position = 1;
                    PeticionRepoCanales.Param2 = &position;
                    CRM_Message(NMM, SubM_Repo, &PeticionRepoCanales);//Peticion segundo mejor canal.
                    SegundoCanal  = *(BYTE*)(PeticionRepoCanales.Param3);
                    position = 2;
                    PeticionRepoCanales.Param2 = &position;        
                    CRM_Message(NMM, SubM_Repo, &PeticionRepoCanales);//Peticion tercer mejor canal. 
                    TercerCanal  = *(BYTE*)(PeticionRepoCanales.Param3);
                    position = 3;
                    PeticionRepoCanales.Param2 = &position;            
                    CRM_Message(NMM, SubM_Repo, &PeticionRepoCanales);//Peticion cuarto mejor canal.
                    CuartoCanal = *(BYTE*)(PeticionRepoCanales.Param3);
                    BYTE RSSIcuarto = *(BYTE*)(PeticionRepoCanales.Param4);
                    
                    char trazaCanales[80];
                    sprintf(trazaCanales, "\r\nLos canales mejores para mi son: el %d, el %d, el %d y el %d.\r\n", MejorCanal, SegundoCanal, TercerCanal, CuartoCanal);
                    Printf(trazaCanales);
                    
                    if(MejorCanal == CanalOtroNodo || SegundoCanal == CanalOtroNodo || TercerCanal == CanalOtroNodo || CuartoCanal == CanalOtroNodo){
                        cambio = TRUE;
                    } else {
                        Printf("\r\nEl canal del otro nodo no esta entre mis mejores. Comparo el RSSI del canal al que quiere cambiar con el de mi ultimo mejor.");
                        REPO_MSSG_RCVD PeticionRepoRSSICanal;
                        PeticionRepoRSSICanal.Action = ActSndDta;
                        PeticionRepoRSSICanal.Transceiver = ri;
                        PeticionRepoRSSICanal.DataType = EnvRSSICh;
                        PeticionRepoRSSICanal.Param1 = &ri;                      
                        PeticionRepoRSSICanal.Param2 = &CanalOtroNodo;            
                        CRM_Message(NMM, SubM_Repo, &PeticionRepoCanales);//Peticion cuarto mejor canal.
                        BYTE RSSICanalOtroNodo = *(BYTE*)(PeticionRepoCanales.Param3);                        
                        BYTE diff = (RSSICanalOtroNodo - RSSIcuarto);
                        char trazaCanales[80];
                        sprintf(trazaCanales, "\r\nEl RSSI del canal del otro nodo y el del ultimo mejor mio es: %d, %d.\r\n", RSSICanalOtroNodo, *(BYTE*)(PeticionRepoCanales.Param4));
                        Printf(trazaCanales);
                        if(diff < 0x05){
                            Printf("\r\nEstan muy proximos se acepta el cambio.");
                            cambio = TRUE;
                        } else {
                            Printf("\r\nNo estan proximos se rechaza el canal.");
                            cambio = FALSE;
                        }
                    } 
                    
                    if (cambio){
                        //Mandar TRUE al resto de nodos
                        canalCambio = CanalOtroNodo;                
                        BYTE MensajeVCC[] = {VccCtrlMssg, SubM_Opt, ActProcRq, ProcRespCambio, TRUE, Peticion->Transceiver};
                        MSN_MSSG_RCVD PeticionCambioCanal;
                        VCC_MSSG_RCVD PeticionVCCCambioCanal;

                        PeticionVCCCambioCanal.AddrMode = BROADCAST_ADDRMODE;
                        PeticionVCCCambioCanal.Action = ActSend;
                        PeticionVCCCambioCanal.BufferVCC = MensajeVCC;
                        PeticionVCCCambioCanal.Transceiver = MIWI_0434;
                        BYTE sizeBufferVCC = 6;
                        PeticionVCCCambioCanal.Param1 = &sizeBufferVCC;

                        PeticionCambioCanal.Peticion_Destino.PeticionVCC = &PeticionVCCCambioCanal;

                        CRM_Message(VCC, SubM_Ext, &PeticionCambioCanal);

                        Printf("\r\nTimeout a 0");
                        MilisDeTimeOut = 0;
                        
                        EstadoGT = EsperandoDecFinal;
                    } else {
                        //Cambio de canal, aviso al resto.
                        Printf("\r\nNo quiero cambiar al canal que me dice. Le digo que no me quiero cambiar a ese y si es el ultimo nodo por contestar me cambio.");
                        
                        BYTE MensajeVCC[] = {VccCtrlMssg, SubM_Opt, ActProcRq, ProcDecFinal, FALSE, Peticion->Transceiver};
                        MSN_MSSG_RCVD PeticionCambioCanal;
                        VCC_MSSG_RCVD PeticionVCCCambioCanal;

                        PeticionVCCCambioCanal.AddrMode = BROADCAST_ADDRMODE;
                        PeticionVCCCambioCanal.Action = ActSend;
                        PeticionVCCCambioCanal.BufferVCC = MensajeVCC;
                        PeticionVCCCambioCanal.Transceiver = MIWI_0434;
                        BYTE sizeBufferVCC = 6;
                        PeticionVCCCambioCanal.Param1 = &sizeBufferVCC;

                        PeticionCambioCanal.Peticion_Destino.PeticionVCC = &PeticionVCCCambioCanal;
                        
                        CRM_Message(VCC, SubM_Ext, &PeticionCambioCanal);
                        
                        if(nRespuestas == nRespuestasAfirmativas){
                            BYTE MensajeVCC[] = {VccCtrlMssg, SubM_Opt, ActProcRq, ProcRespCambio, TRUE, Peticion->Transceiver};

                            MSN_MSSG_RCVD PeticionCambioCanal;
                            VCC_MSSG_RCVD PeticionVCCCambioCanal;

                            PeticionVCCCambioCanal.AddrMode = BROADCAST_ADDRMODE;
                            PeticionVCCCambioCanal.Action = ActSend;
                            PeticionVCCCambioCanal.BufferVCC = MensajeVCC;
                            PeticionVCCCambioCanal.Transceiver = MIWI_0434;
                            BYTE sizeBufferVCC = 6;
                            PeticionVCCCambioCanal.Param1 = &sizeBufferVCC;

                            PeticionCambioCanal.Peticion_Destino.PeticionVCC = &PeticionVCCCambioCanal;

                            CRM_Message(VCC, SubM_Ext, &PeticionCambioCanal);
                            
                            Printf("\r\nTodos los nodos conectados  y que quieren cambiar van a cambiar al canal propuesto, se cambia de canal.");
                            EXEC_MSSG_RCVD PeticionExec;
                            PeticionExec.OrgModule = SubM_Opt;
                            PeticionExec.Action = ActChnHop;
                            PeticionExec.Param1 = canalCambio;
                            PeticionExec.Transceiver = ri;

                            riActual = ri;

                            CRM_Message(NMM, SubM_Exec, &PeticionExec);//envio del messg.
                            /*Fin del Message para Executor.*/
                            WORD riname;
                            switch (ri) {
                                case MIWI_0434: riname = 434; break;
                                case MIWI_0868: riname = 868; break;
                                case MIWI_2400: riname = 2400; break;
                            }
                            char traza[80];
                            sprintf(traza, "\r\nCambiado al canal %d de la interfaz %d y enviada "
                                    "la peticion al otro nodo.\r\n", GetOpChannel(ri), riname);
                            Printf(traza);                       

                            //Se devuelven todas las variables usadas al estado inicial
                            EstadoGT = Clear;
                            CHNG_MSSG_RCVD = 0;
                            inicioCambio = FALSE;
                            nRespuestas = 0;
                            nRespuestasAfirmativas = 0;   
                            nRespuestasNegativas = 0;
                        } else {
                            
                            Printf("Cambio de canal aunque alguno no quiera el mismo que yo.");
                            EXEC_MSSG_RCVD PeticionExec;
                            PeticionExec.OrgModule = SubM_Opt;
                            PeticionExec.Action = ActChnHop;
                            PeticionExec.Param1 = canalCambio;
                            PeticionExec.Transceiver = ri;

                            riActual = ri;

                            CRM_Message(NMM, SubM_Exec, &PeticionExec);//envio del messg.
                            /*Fin del Message para Executor.*/
                            WORD riname;
                            switch (ri) {
                                case MIWI_0434: riname = 434; break;
                                case MIWI_0868: riname = 868; break;
                                case MIWI_2400: riname = 2400; break;
                            }
                            char traza[80];
                            sprintf(traza, "\r\nCambiado al canal %d de la interfaz %d y enviada "
                                    "la peticion al otro nodo.\r\n", GetOpChannel(ri), riname);
                            Printf(traza);                       

                            //Se devuelven todas las variables usadas al estado inicial
                            EstadoGT = Clear;
                            CHNG_MSSG_RCVD = 0;
                            inicioCambio = FALSE;
                            nRespuestas = 0;
                            nRespuestasAfirmativas = 0;   
                            nRespuestasNegativas = 0; 
                            
                        }
                    }                    
                } else {
                    if(nRespuestas == nRespuestasAfirmativas){
                        BYTE MensajeVCC[] = {VccCtrlMssg, SubM_Opt, ActProcRq, ProcRespCambio, TRUE, Peticion->Transceiver};

                        MSN_MSSG_RCVD PeticionCambioCanal;
                        VCC_MSSG_RCVD PeticionVCCCambioCanal;

                        PeticionVCCCambioCanal.AddrMode = BROADCAST_ADDRMODE;
                        PeticionVCCCambioCanal.Action = ActSend;
                        PeticionVCCCambioCanal.BufferVCC = MensajeVCC;
                        PeticionVCCCambioCanal.Transceiver = MIWI_0434;
                        BYTE sizeBufferVCC = 6;
                        PeticionVCCCambioCanal.Param1 = &sizeBufferVCC;

                        PeticionCambioCanal.Peticion_Destino.PeticionVCC = &PeticionVCCCambioCanal;

                        CRM_Message(VCC, SubM_Ext, &PeticionCambioCanal);
                        
                        Printf("\r\nYa se han recibido todas las respuestas y hay alguno que no quiere cambiar. Me cambio de todas formas.");
                        EXEC_MSSG_RCVD PeticionExec;
                        PeticionExec.OrgModule = SubM_Opt;
                        PeticionExec.Action = ActChnHop;
                        PeticionExec.Param1 = canalCambio;
                        PeticionExec.Transceiver = ri;

                        riActual = ri;

                        CRM_Message(NMM, SubM_Exec, &PeticionExec);//envio del messg.
                        /*Fin del Message para Executor.*/
                        WORD riname;
                        switch (ri) {
                            case MIWI_0434: riname = 434; break;
                            case MIWI_0868: riname = 868; break;
                            case MIWI_2400: riname = 2400; break;
                        }
                        char traza[80];
                        sprintf(traza, "\r\nCambiado al canal %d de la interfaz %d y enviada "
                                "la peticion al otro nodo.\r\n", GetOpChannel(ri), riname);
                        Printf(traza);                       

                        //Se devuelven todas las variables usadas al estado inicial
                        EstadoGT = Clear;
                        CHNG_MSSG_RCVD = 0;
                        inicioCambio = FALSE;
                        nRespuestas = 0;
                        nRespuestasAfirmativas = 0;   
                        nRespuestasNegativas = 0;  
                    }
                }      
            }
            break;
        default:
            break;
    }
    return FALSE;
}

BOOL CRM_Optm_GameTheory(OPTM_MSSG_RCVD *Peticion)
{
    BOOL chngChnn = FALSE;
    //TODO la teoria de juegos.
    switch (Peticion->Action)
    {
        case(SubActChngCost):
            /*Realiza el calculo de la estrategia de optimizacion.*/
            chngChnn = CRM_Optm_CostAlgorithm(CosteN, CosteO, CosteCh, ProbChngCompi);
            if(chngChnn)
            {
                /*Primero calculamos el canal optimo para el cambio.*/
                    //Creamos unos parametros para un mensaje para Discovery.
                    //Para cada una de las interfaces:
                    BYTE CanalOptimo, CanalOptimo434, CanalOptimo868, CanalOptimo2400;
                    DWORD TodosCanales = 0xFFFFFFFF;
                    BYTE TiempoScan = 5;
                    BYTE TipoScan = NOISE_DETECT_ENERGY;
                    //Para cada una de las interfaces:
                    BYTE RSSIResultado434 = 255;
                    BYTE RSSIResultado868 = 255;
                    BYTE RSSIResultado2400 = 255;
                    //Creacion de un mensaje de discovery.
                    DISC_MSSG_RCVD PeticionDiscTest2;
                    PeticionDiscTest2.OrgModule = SubM_Opt;
                    PeticionDiscTest2.Action = ActSignDetc;
                    PeticionDiscTest2.Param1 = &TodosCanales;
                    PeticionDiscTest2.Param2 = &TiempoScan;
                    PeticionDiscTest2.Param3 = &TipoScan; //NOISE DETECT ENERGY.

                    //Ahora calculo el canal óptimo de cada interfaz y el canal más óptimo de todos:
                    PeticionDiscTest2.Transceiver = MIWI_0434;
                    PeticionDiscTest2.Param4 = &RSSIResultado434;
                    CanalOptimo434 = *(BYTE*)CRM_Message(NMM, SubM_Disc, &PeticionDiscTest2);
                    PeticionDiscTest2.Transceiver = MIWI_0868;
                    PeticionDiscTest2.Param4 = &RSSIResultado868;
                    CanalOptimo868 = *(BYTE*)CRM_Message(NMM, SubM_Disc, &PeticionDiscTest2);
                    PeticionDiscTest2.Transceiver = MIWI_2400;
                    PeticionDiscTest2.Param4 = &RSSIResultado2400;
                    CanalOptimo2400 = *(BYTE*)CRM_Message(NMM, SubM_Disc, &PeticionDiscTest2);
                    if ((RSSIResultado434 <= RSSIResultado868) && (RSSIResultado434 <= RSSIResultado2400)) {
                        CanalOptimo = CanalOptimo434;
                        ri = MIWI_0434;
                    } else if ((RSSIResultado868 < RSSIResultado434) && (RSSIResultado868 <= RSSIResultado2400)) {
                        CanalOptimo = CanalOptimo868;
                        ri = MIWI_0868;
                    } else {
                        CanalOptimo = CanalOptimo2400;
                        ri = MIWI_2400;
                    }
                    //Jose//CanalOptimo = *(BYTE*)CRM_Message(NMM, SubM_Disc, &PeticionDiscTest2);//envio del messge.
//                  CRM_Message(NMM, SubM_Disc, &PeticionDiscTest2);//envio del messge.
//                    CanalOptimo = *((BYTE*)(PeticionDiscTest2.Param4));
                /*Fin del calculo del canal optimo.*/
#if defined(TEST4)
                /*Creamos la peticion para el nodo externo y la enviamos.*/
                    MSN_MSSG_RCVD PeticionNodoExt;
                    VCC_MSSG_RCVD PeticionNodoExtVCC;

                    //BYTE EUI_Dest[] = {0x01, EUI_1, EUI_2, EUI_3, EUI_4, EUI_5, EUI_6, EUI_7};
                    BYTE EUI_Dest[] = {EUI_0, EUI_1, EUI_2, EUI_3, EUI_4, EUI_5, EUI_6, 0x22};
                    BYTE PeticionNodoExtBuff[] = {VccCtrlMssg, SubM_Opt, ActProcRq, ProcAsk4Chng, CanalOptimo, ri};
                    PeticionNodoExtVCC.DirNodDest = EUI_Dest;
                    PeticionNodoExtVCC.Action = ActSend;
                    PeticionNodoExtVCC.BufferVCC = PeticionNodoExtBuff;
                    PeticionNodoExtVCC.Transceiver = Peticion->Transceiver;//XXX
                    BYTE sizeOfBufferVCC = 6;
                    PeticionNodoExtVCC.Param1 = &sizeOfBufferVCC;

                    PeticionNodoExt.Peticion_Destino.PeticionVCC = &PeticionNodoExtVCC;

                    CRM_Message(VCC, SubM_Ext, &PeticionNodoExt);
                /*Fin de enviar la peticion al nodo externo.*/
#endif
#if defined TEST6
                //Iniciar el contador de espera de respuesta
                MilisDeTimeOut = 0;
                OptmEstado = WaitinAnsw4ChngChn;
#endif
                /*Creamos y enviamos un Message a Executor para cambiar el canal.*/
                    EXEC_MSSG_RCVD PeticionExecTest2;
                    PeticionExecTest2.OrgModule = SubM_Opt;
                    PeticionExecTest2.Action = ActChnHop;
                    PeticionExecTest2.Param1 = CanalOptimo;
                    PeticionExecTest2.Transceiver = ri;//Peticion->Transceiver;

                    CRM_Message(NMM, SubM_Exec, &PeticionExecTest2);//envio del messg.
                /*Fin del Message para Executor.*/
                WORD riname;
                switch (ri) {
                    case MIWI_0434: riname = 434; break;
                    case MIWI_0868: riname = 868; break;
                    case MIWI_2400: riname = 2400; break;
                }
                char traza[80];
                sprintf(traza, "\r\nCambiado al canal %d de la interfaz %d y enviada "
                        "la peticion al otro nodo.\r\n", GetOpChannel(Peticion->Transceiver), riname);
                Printf(traza);

                    //TODO. Actualizar la probabilidad de cambio.
#if defined(TEST5)
                //Para medir los tiempos de ejecucion.
            t2G = MiWi_TickGet();
            tTG.Val = MiWi_TickGetDiff(t2G, t1G);
#endif
            }
            //TODO HECHO llamar a la funcion del coste de cambio con los parametros que toquen.
            break;
        default:
            break;
    }
    return TRUE;
}
/*Fin de funciones de funcionalidad propia del sub-modulo*/

/*Sub-metodos de funciones*/
//El de coste de cambio de canal de Elena.
BOOL CRM_Optm_CostAlgorithm(INPUT BYTE CosteNotChanging , INPUT BYTE CosteOcup, INPUT BYTE CosteChngChann, INPUT WORD ProbCambioY)
{
    BOOL change;
    //Función de optimización//XXX-GuilJa.Los dos últimos terminos no sobrarian
    // dado que son constantes que se le suman a los dos terminos y por tanto no
    //variará el resultado de la comparacion.
    //XXX-GuilJa lo simplifico más abajo  WORD ganancia0 = (2*CosteNotChanging-CosteOcup)*0*ProbCambioY + (CosteOcup-CosteChngChann-CosteNotChanging)*0 + (CosteOcup-CosteNotChanging)*ProbCambioY - CosteOcup;
    //XXX-GuilJa lo simplifico más abajo.  WORD ganancia1 = (2*CosteNotChanging-CosteOcup)*1*ProbCambioY + (CosteOcup-CosteChngChann-CosteNotChanging)*1 + (CosteOcup-CosteNotChanging)*ProbCambioY - CosteOcup;

    //  WORD ganancia0 = 0;//XXX-GuilJa. Como queda 0 lo quitamos y cambiamos la proxima comprobacion.
    WORD ganancia1 = (2*CosteNotChanging-CosteOcup)*1*ProbCambioY + (CosteOcup-CosteChngChann-CosteNotChanging)*1;

    //XXX-GuilJa por la simplificacion que hemos hecho más arriba.  if(ganancia0>ganancia1)
    if(0>=ganancia1)
    {
        change = FALSE; //no cambio canal
    }
    else
    {
        change = TRUE; //cambio canal
    }
    return change;
    //TODO REVISAR no actualiza la probabilidad de cambio de el nodo propio no?
}
/*Fin de sub-metodos de funciones*/

/*Funciones de inicializacion*/
BOOL CRM_Optm_Init(void)
{
    SetTXPower(riActual, 0x00);
    
    numNodosRed = CONNECTION_SIZE;
    num_rtx = 0;
    BYTE i;
    Cuentamseg = 0;//Ponemos el contador a 0.
    Periodomseg = PeriodoXDefecto;
    mseg = 0;
    Periodo = 1000;
    tiempoCambio = 0;
    tiempoCambioPotTX = 50000;
    tiempoCambioFrecPaquetes = 60000;
    learningTimeMax = AprendizajeMax;
    reinicioAtacantesTimeMax = ReinicioMax;
    
    TiempoAnterior = MiWi_TickGet();
    paquetesRecibidos = 0;
    
    tiempo = 0;
    //TODO la inicializacion.
    //Ini de las estrategias de optimizacion.
    //La de coste de cambio de canal.
        CosteTx = CosteTxXDefecto;
        CosteRx = CosteRxXDefecto;
        CosteSensing = CosteSensingXDefecto;
        EstadoGT = Clear;
        CtrlMssgFlag = FALSE;
        nRespuestas = 0;
        nRespuestasAfirmativas = 0;
        nRespuestasNegativas = 0;
        primera = 0;
        flagPrimeraEjecucion = 0;
        inicioCambio = FALSE;
        learningTime = 0;
        aprendizaje = 0;
        normalizado = 0;
        clustersDone = 0;
        MSSG_PROC_OPTM = 0;
        
       // NumMsj = numMsjXDefecto; //Este es mi "espacio muestral" de los mensajes
            //que tengo en cuenta para realizar los calculos.
        MaxRTx = maxRTxXDefecto;
        //TODO lo relacionado con las retransmisiones hay que corregirlos.
        //TODO el lastTRX
        #if defined(MRF24J40)
            //TODO el MssgRTx//Hay rtx a nivel mac, buscar "retry" o "retries".
        #else
            //TODO el MssgRTx
        #endif

        //definicion de los costes
        CosteCh = CosteSensing + CosteTx + CosteRx;
        CosteO = CosteTx;//XXX-GuilJa. Lo inicializamos al coste de tx.
        CosteN = CosteTx * MaxRTx;


        ProbChngCompi = probChngCompiXDefecto;
    //Fin de la de coste de cambio

//#if defined(TEST4)
    PeticionRecepcion.Action = ActRecv;
    BufferRecepcionPrueba.SourceAddress = Direccion;
    BufferRecepcionPrueba.Payload = BufferPrueba;
    PeticionRecepcion.BufferVCC = &BufferRecepcionPrueba;
    BYTE TamanoBuffer = RX_BUFFER_SIZE;
    PeticionRecepcion.Param1 = &TamanoBuffer;

    //PeticionTest2.Action = SubActChngCost;

//#endif
#if defined TEST6
//TODO inicializar lo que haga falta como por ejemplo el timeout.
//    MilisDeTimeOut = TIEMPODEESPERARESPUESTA;
    MilisDeTimeOut = 0;
    OptmEstado = Idle;
#endif
    return TRUE;
}
/*Fin de las funciones de inicializacion*/

/*Función de configuracion*/
BOOL CRM_Optm_Config(INPUT OPTACTION SubAccion, INPUT BYTE ConfigCosteTx, INPUT BYTE ConfigCosteRx,
        INPUT BYTE ConfigCosteSensing,INPUT BYTE ConfigmaxRTx, INPUT BYTE ConfignumMsj, INPUT WORD ConfigProbY)
{
    if(ConfigCosteTx != 0)
    {
        CosteTx = ConfigCosteTx;
        CosteCh = CosteSensing + CosteTx + CosteRx;
        CosteO = CosteTx * (SumRTx / MssgIndex);
        CosteN = CosteTx * MaxRTx;
    }
    if(ConfigCosteRx != 0)
    {
        CosteRx = ConfigCosteRx;
        CosteCh = CosteSensing + CosteTx + CosteRx;
    }
    if(ConfigCosteSensing != 0)
    {
        CosteSensing = ConfigCosteSensing;
        CosteCh = CosteSensing + CosteTx + CosteRx;
    }
    if(ConfigmaxRTx != 0)
    {
        MaxRTx = ConfigmaxRTx;
//        SumRTx = MaxRTx - MssgRTx;//Para nuestro sumRtx no influye por el tema
            //de que contamos directamente el num de rtx y no como castalia que
            //cuenta las que faltan para llegar al maximo.
        CosteN = CosteTx * MaxRTx;
//        CosteO = CosteTx * (SumRTx / NumMsj);//XXX-GuilJa. Puesto que ya no
        //influye directamente en SumRTx
    }
    if(ConfignumMsj != 0) //TODO //REVISAR
    {
        NumMsj = ConfignumMsj;
//        CosteO = CosteTx * (SumRTx / NumMsj);//XXX-GuilJa
    }
    if(ConfigProbY != 0)
    {
        ProbChngCompi = ConfigProbY;
    }
}
/*Fin de la funcion de configuracion*/

/*Rutina de tareas generales del sub-modulo*/
BOOL CRM_Optm_Task(TASKPARAM Opcion)
{
    switch(Opcion)
    {
        case TaskParamTx:
            /*Con esta opcion deberiamos llamar cada vez que
                   hayamos enviado un mensaje y de esta forma
                   actualizar el numero de retransmisiones que se
                   hayan producido para dicho envio.*/
            MssgIndex += 1;
            #if defined(MRF24J40)
                //TODO el MssgRTx//Hay rtx a nivel mac, buscar retries".
                //TODO creo que lo mejor sería que llamaramos a esta funcion desde la
                //interrupcion del MRF24J40 que es donde estamos seguro que ha enviado
                //un paquete.
            MssgRTx[MssgIndex] = 2;//XXX-GuilJa para prueba.
            #else
                //TODO el MssgRTx
                //Se llama reTry y esta en el TxPacket del MRF49XA.
            #endif
            //lo que puedo hacer aqui es emular unos cambios en el MssgRtx.
            if(MssgIndex < NumMsj)
            {
                SumRTx += MssgRTx[MssgIndex];//Esto es lo que en Castalia está como una resta
                    //porque ellos tienen el numero de rtx que les queda para llegar
                    //al maximo en vez del numero de rtx tal cual del ultimo
                    //mensaje.
            }
            CosteO = CosteTx * (SumRTx / MssgIndex);
            break;
        default:
            break;
    }
}
/*Fin de rutina de tareas generales del sub-modulo*/

BOOL CRM_Optm_Incluir_Potencia(){
    
    BYTE RSSI;
    BYTE *pRSSI = &RSSI;

    if(GetRSSI(riData,pRSSI) == 0){        
        //Creo el mensaje para repository y se lo mando
        REPO_MSSG_RCVD PeticionRepoPotencias;
        REPODATATYPE PetPotencia = IncluirPotencia;
        PeticionRepoPotencias.Action = ActStr;
        PeticionRepoPotencias.DataType = PetPotencia;
        PeticionRepoPotencias.Param1 = &PetPotencia;
        PeticionRepoPotencias.Param2 = pRSSI;

        CRM_Message(NMM, SubM_Repo, &PeticionRepoPotencias);
        return TRUE;
    } else {
        return FALSE;
    }
    
}

BOOL CRM_Optm_Inicio_GT(BYTE *pVector){

    BOOL inicio;
    BYTE i, media;
    LONG s;
    s = 0;
    for(i = 0; i < MAX_VECTOR_POTENCIA; i++){
        s += *(pVector + i);
    }
    media = s/MAX_VECTOR_POTENCIA;
    if(media < UMBRAL_POTENCIA){
        inicio = TRUE;
    } else {
        inicio = FALSE;
    }
    return inicio;

}

/*Devuelve si se decide cambiar de canal o quedarse en el que está*/
BOOL CRM_Optm_Calcular_Costes(BYTE n_rtx){
    //El CosteNoCambio siempre va a ser mayor o igual que el CosteOcupado. Lo calculo pero al decidir
    //si cambio o no, no lo tengo en cuenta.
    BOOL cambio;
    CosteCambio = CosteSensing + ((CosteTx + CosteRx) * n_msg);
    CosteOcupado = CosteTx * n_rtx;
    CosteNoCambio = CosteTx * MaxRTx;
    if (CosteCambio < CosteOcupado){
        cambio = TRUE;
    } else {
        cambio = FALSE;
    }
    return cambio;

}

BOOL CRM_Optm_Cons(OPTM_MSSG_RCVD *Peticion){

    BackupCanal = GetOpChannel(Peticion->Transceiver);
    
    switch (Peticion->Action)
    {
        case(SubActCambio):

                CHNG_MSSG_RCVD = 1;
                /*Primero calculamos el canal optimo para el cambio.*/
                //Creamos unos parametros para un mensaje para Discovery.
                //Para cada una de las interfaces:
                BYTE CanalOptimo, CanalOptimo868, CanalOptimo2400;
                CanalOptimo868 = 0;
                CanalOptimo2400 = 0;
                DWORD TodosCanales = 0xFFFFFFFF;
                BYTE TiempoScan = 5;                                               
                BYTE TipoScan = NOISE_DETECT_ENERGY;
                //Para cada una de las interfaces:
                BYTE RSSIResultado868 = 255;
                BYTE RSSIResultado2400 = 255;
                //Creacion de un mensaje de discovery.
                DISC_MSSG_RCVD PeticionDiscCOpt;
                PeticionDiscCOpt.OrgModule = SubM_Opt;
                PeticionDiscCOpt.Action = ActSignDetc;
                PeticionDiscCOpt.Param1 = &TodosCanales;
                PeticionDiscCOpt.Param2 = &TiempoScan;
                PeticionDiscCOpt.Param3 = &TipoScan; //NOISE DETECT ENERGY.

                //Ahora calculo el canal óptimo de cada interfaz y el canal más óptimo de todos:
                #ifdef MIWI_0868_RI
                PeticionDiscCOpt.Transceiver = MIWI_0868;
                PeticionDiscCOpt.Param4 = &RSSIResultado868;
                CanalOptimo868 = *(BYTE*)CRM_Message(NMM, SubM_Disc, &PeticionDiscCOpt);
                #endif
                #ifdef MIWI_2400_RI
                PeticionDiscCOpt.Transceiver = MIWI_2400;
                PeticionDiscCOpt.Param4 = &RSSIResultado2400;
                CanalOptimo2400 = *(BYTE*)CRM_Message(NMM, SubM_Disc, &PeticionDiscCOpt);
                #endif
                if (RSSIResultado868 < RSSIResultado2400) {
                    CanalOptimo = CanalOptimo868;
                    ri = MIWI_0868;
                } else {
                    CanalOptimo = CanalOptimo2400;
                    ri = MIWI_2400;
                }
                
                canalCambio = CanalOptimo;
                
                //Mandar la información del cambio de canal al resto de nodos.
                MSN_MSSG_RCVD PeticionCambioCanal;
                VCC_MSSG_RCVD PeticionVCCCambioCanal;
                BYTE MensajeVCC[] = {VccCtrlMssg, SubM_Repo, ActStr, NetNode, RSSINetNode, RSSIResultado868, CanalOptimo868, RSSIResultado2400, CanalOptimo2400, CanalOptimo, ri, Peticion->Transceiver};

                PeticionVCCCambioCanal.AddrMode = BROADCAST_ADDRMODE;
                PeticionVCCCambioCanal.Action = ActSend;
                PeticionVCCCambioCanal.BufferVCC = MensajeVCC;
                PeticionVCCCambioCanal.Transceiver = MIWI_0434;
                BYTE sizeBufferVCC = 12;
                PeticionVCCCambioCanal.Param1 = &sizeBufferVCC;

                PeticionCambioCanal.Peticion_Destino.PeticionVCC = &PeticionVCCCambioCanal;
                Printf("\r\nSe manda peticion de cambio de canal al otro nodo.");
                CRM_Message(VCC, SubM_Ext, &PeticionCambioCanal);

                //Guardo el resto de RSSI propios
                REPO_MSSG_RCVD PeticionRepoRSSI;                
                PeticionRepoRSSI.Action = ActStr;
                PeticionRepoRSSI.DataType = SaveRSSI;
                PeticionRepoRSSI.Param1 = &ri;

                CRM_Message(NMM, SubM_Repo, &PeticionRepoRSSI);
                
                Printf("\r\nTimeout a 0");
                MilisDeTimeOut = 0;
                
                EstadoGT = EsperandoDecisionRestoNodos;

            break;
        case(SubActRespCambio):
            {
            //Ya se ha mandado la contestación al otro nodo, se comprueba si los canales óptimos concuerdan
            //si no concuerdan , se manda la información sensada.
                
            BackupCanal = GetOpChannel(riActual);

            /*Primero calculamos el canal optimo para el cambio.*/
            //Creamos unos parametros para un mensaje para Discovery.
            //Para cada una de las interfaces:
            BYTE CanalOptimo, CanalOptimo868, CanalOptimo2400;
            CanalOptimo868 = 0;
            CanalOptimo2400 = 0;
            DWORD TodosCanales = 0xFFFFFFFF;
            BYTE TiempoScan = 5;
            BYTE TipoScan = NOISE_DETECT_ENERGY;
            //Para cada una de las interfaces:
            BYTE RSSIResultado868 = 255;
            BYTE RSSIResultado2400 = 255;
            //Creacion de un mensaje de discovery.
            DISC_MSSG_RCVD PeticionDiscCOpt;
            PeticionDiscCOpt.OrgModule = SubM_Opt;
            PeticionDiscCOpt.Action = ActSignDetc;
            PeticionDiscCOpt.Param1 = &TodosCanales;
            PeticionDiscCOpt.Param2 = &TiempoScan;
            PeticionDiscCOpt.Param3 = &TipoScan; //NOISE DETECT ENERGY.

            //Ahora calculo el canal óptimo de cada interfaz y el canal más óptimo de todos:
            #ifdef MIWI_0868_RI
            PeticionDiscCOpt.Transceiver = MIWI_0868;
            PeticionDiscCOpt.Param4 = &RSSIResultado868;
            CanalOptimo868 = *(BYTE*)CRM_Message(NMM, SubM_Disc, &PeticionDiscCOpt);
            #endif
            #ifdef MIWI_2400_RI
            PeticionDiscCOpt.Transceiver = MIWI_2400;
            PeticionDiscCOpt.Param4 = &RSSIResultado2400;
            CanalOptimo2400 = *(BYTE*)CRM_Message(NMM, SubM_Disc, &PeticionDiscCOpt);
            #endif
            if (RSSIResultado868 < RSSIResultado2400) {
                CanalOptimo = CanalOptimo868;
                ri = MIWI_0868;
            } else {
                CanalOptimo = CanalOptimo2400;
                ri = MIWI_2400;
            }           

            BOOL cambio = FALSE;
            ri = *(BYTE*)(Peticion->Param1 + 2);            
            BYTE CanalOtroNodo = (*(BYTE*)(Peticion->Param1 + 1));
            BYTE RSSICanalOtroNodo = (*(BYTE*)(Peticion->Param1 + 3));
            switch(ri){
                case MIWI_0868:
                    CanalOptimo = CanalOptimo868;
                    break;
                case MIWI_2400:
                    CanalOptimo = CanalOptimo2400;
                    break;
            }
            
            WORD riname;
            switch (ri) {
                case MIWI_0868: riname = 868; break;
                case MIWI_2400: riname = 2400; break;
            }
            char traza[80];
            sprintf(traza, "\r\nQuiere cambiar al canal %d de la interfaz %d.\r\n", CanalOtroNodo, riname);
            Printf(traza);
            
            //Guardo el resto de RSSI propios
            REPO_MSSG_RCVD PeticionRepoRSSI;                
            PeticionRepoRSSI.Action = ActStr;
            PeticionRepoRSSI.DataType = SaveRSSI;
            PeticionRepoRSSI.Param1 = &ri;
            CRM_Message(NMM, SubM_Repo, &PeticionRepoRSSI);
            //Pido a repo el canal Optimo que he sacado antes de la ri que quiere cambiarse el otro nodo.
            //Pido a repo el 2º mejor canal, el 3º y el 4º.
            BYTE MejorCanal, SegundoCanal, TercerCanal, CuartoCanal, position;
            REPO_MSSG_RCVD PeticionRepoCanales;
            PeticionRepoCanales.Action = ActSndDta;
            PeticionRepoCanales.Transceiver = ri;
            PeticionRepoCanales.DataType = EnvRSSI;
            PeticionRepoCanales.Param1 = &ri;
            //Se podría hacer un método en repo que directamente te devolviese los 4 primeros pero 
            //entonces no sería tan ajustable como haciendolo así, si quieres comparar con más canales solo se lo pides
            //y si quieres menos los borras.
            position = 0;
            PeticionRepoCanales.Param2 = &position;
            CRM_Message(NMM, SubM_Repo, &PeticionRepoCanales);//Peticion mejor canal.
            MejorCanal  = *(BYTE*)(PeticionRepoCanales.Param3);
            position = 1;
            PeticionRepoCanales.Param2 = &position;
            CRM_Message(NMM, SubM_Repo, &PeticionRepoCanales);//Peticion segundo mejor canal.
            SegundoCanal  = *(BYTE*)(PeticionRepoCanales.Param3);
            position = 2;
            PeticionRepoCanales.Param2 = &position;        
            CRM_Message(NMM, SubM_Repo, &PeticionRepoCanales);//Peticion tercer mejor canal. 
            TercerCanal  = *(BYTE*)(PeticionRepoCanales.Param3);
            position = 3;
            PeticionRepoCanales.Param2 = &position;            
            CRM_Message(NMM, SubM_Repo, &PeticionRepoCanales);//Peticion cuarto mejor canal.
            CuartoCanal = *(BYTE*)(PeticionRepoCanales.Param3);
            
            char trazaCanales[80];
            sprintf(trazaCanales, "\r\nLos canales mejores para mi son: el %d, el %d, el %d y el %d.\r\n", MejorCanal, SegundoCanal, TercerCanal, CuartoCanal);
            Printf(trazaCanales);            
            
            if(CanalOptimo == CanalOtroNodo || SegundoCanal == CanalOtroNodo || TercerCanal == CanalOtroNodo || CuartoCanal == CanalOtroNodo){
                cambio = TRUE;
            } else {
                BYTE diff;
                if (*(BYTE*)(PeticionRepoCanales.Param4) > RSSICanalOtroNodo){
                    diff = (*(BYTE*)(PeticionRepoCanales.Param4) - RSSICanalOtroNodo);
                } else { 
                    diff = (RSSICanalOtroNodo - *(BYTE*)(PeticionRepoCanales.Param4)); 
                }
                if(diff < 0x05){
                    cambio = TRUE;
                } else {
                    cambio = FALSE;
                }
            }          
            //Aqui comprueba si el canal optimo y la ri concuerdan
            if (cambio){
                //Mandar TRUE al resto de nodos
                Printf("\r\nAcepto el canal que me han ofrecido.");
                canalCambio = CanalOtroNodo;                
                BYTE MensajeVCC[] = {VccCtrlMssg, SubM_Opt, ActProcRq, ProcRespCambio, TRUE, Peticion->Transceiver};
                MSN_MSSG_RCVD PeticionCambioCanal;
                VCC_MSSG_RCVD PeticionVCCCambioCanal;

                PeticionVCCCambioCanal.AddrMode = BROADCAST_ADDRMODE;
                PeticionVCCCambioCanal.Action = ActSend;
                PeticionVCCCambioCanal.BufferVCC = MensajeVCC;
                PeticionVCCCambioCanal.Transceiver = MIWI_0434;
                BYTE sizeBufferVCC = 6;
                PeticionVCCCambioCanal.Param1 = &sizeBufferVCC;

                PeticionCambioCanal.Peticion_Destino.PeticionVCC = &PeticionVCCCambioCanal;

                CRM_Message(VCC, SubM_Ext, &PeticionCambioCanal);

                Printf("\r\nTimeout a 0");
                MilisDeTimeOut = 0;                
                
                EstadoGT = EsperandoDecFinal;
            } else {
                Printf("\r\nNo acepto el canal que me han ofrecido. Mando la informacion de sensado.");
                //Mandar la información del cambio de canal al resto de nodos.
                BYTE MensajeVCC[] = {VccCtrlMssg, SubM_Repo, ActStr, NetNode, RSSINetNode, RSSIResultado868, CanalOptimo868, RSSIResultado2400, CanalOptimo2400, CanalOptimo, ri, Peticion->Transceiver};
                MSN_MSSG_RCVD PeticionCambioCanal;
                VCC_MSSG_RCVD PeticionVCCCambioCanal;

                PeticionVCCCambioCanal.AddrMode = BROADCAST_ADDRMODE;
                PeticionVCCCambioCanal.Action = ActSend;
                PeticionVCCCambioCanal.BufferVCC = MensajeVCC;
                PeticionVCCCambioCanal.Transceiver = MIWI_0434;
                BYTE sizeBufferVCC = 12;
                PeticionVCCCambioCanal.Param1 = &sizeBufferVCC;

                PeticionCambioCanal.Peticion_Destino.PeticionVCC = &PeticionVCCCambioCanal;

                CRM_Message(VCC, SubM_Ext, &PeticionCambioCanal);

                Printf("\r\nTimeout a 0");
                MilisDeTimeOut = 0;                
                
                EstadoGT = EsperandoDecFinal;
            }
            break;
            }
    }
    return FALSE;
}

BOOL CRM_Optm_Calculo_Coordenadas(){

    BYTE dato;
    BYTE *RSSI = &dato;
    MIWI_TICK TiempoActual;
    MIWI_TICK TiempoPaquete;
    TiempoActual = MiWi_TickGet();
    if ( paquetesRecibidos == 0 && GetRSSI(riActual,RSSI) == 0x00 ){  //Si es correcto el valor de RSSI y es el primer dato de un nodo
        Printf("\r\nSe ha recibido el primer paquete\r\n");
        inicializarTablaAtacantes();
        TiempoPaquete = TiempoActual;
        TiempoAnterior = TiempoActual; //Para el tiempo del proximo paquete
        tiempoMax = (double) TiempoPaquete.Val;
        potenciaMax = *RSSI;
        double tiempoCalculado = (double) TiempoPaquete.Val;
        
        REPO_MSSG_RCVD PeticionRepoInclCoord;
        PeticionRepoInclCoord.Action = ActStr;
        PeticionRepoInclCoord.DataType = AddCoord;
        PeticionRepoInclCoord.Param1 = &paquetesRecibidos;
        PeticionRepoInclCoord.Param2 = &tiempoCalculado;
        PeticionRepoInclCoord.Param3 = RSSI;
        
        CRM_Message(NMM, SubM_Repo, &PeticionRepoInclCoord);
        paquetesRecibidos++;
        return TRUE;
    } else {
        if (paquetesRecibidos < MAX_PAQ_APRENDIZAJE && GetRSSI(riActual,RSSI) == 0x00){
            Printf("\r\nSe han recibido mas paquetes\r\n");
            TiempoActual = MiWi_TickGet();
            TiempoPaquete.Val = MiWi_TickGetDiff(TiempoActual, TiempoAnterior);
            if((double) TiempoPaquete.Val > tiempoMax) tiempoMax = (double) TiempoPaquete.Val;
            if(*RSSI > potenciaMax) potenciaMax = *RSSI;
            TiempoAnterior = TiempoActual;
            double tiempoCalculado = (double) TiempoPaquete.Val;

            REPO_MSSG_RCVD PeticionRepoInclCoord;
            PeticionRepoInclCoord.Action = ActStr;
            PeticionRepoInclCoord.DataType = AddCoord;
            PeticionRepoInclCoord.Param1 = &paquetesRecibidos;
            PeticionRepoInclCoord.Param2 = &tiempoCalculado;
            PeticionRepoInclCoord.Param3 = RSSI;

            CRM_Message(NMM, SubM_Repo, &PeticionRepoInclCoord);                        
            paquetesRecibidos++;
            return TRUE;
        }
    }
    return FALSE;
}

void CRM_Optm_Normalizar_Coordenadas(){
    
    REPO_MSSG_RCVD PeticionRepoLista;
    PeticionRepoLista.Action = ActSndDta;
    PeticionRepoLista.DataType = EnvListaPaq;

    Printf("\r\nSe van a normalizar las coordenadas.");
    int i;
    for(i = 0; i < paquetesRecibidos; i++){
        PeticionRepoLista.Param1 = &i;
        CRM_Message(NMM, SubM_Repo, &PeticionRepoLista);
        (*(coord*)PeticionRepoLista.Param2).RSSI = (*(coord*)PeticionRepoLista.Param2).RSSI/potenciaMax;
        (*(coord*)PeticionRepoLista.Param2).tiempo = (*(coord*)PeticionRepoLista.Param2).tiempo/tiempoMax;
    }
    Printf("\r\nSe han normalizado las coordenadas.");
    
}

double CRM_Optm_Calculo_Distancia(coord pto1, coord pto2){
    double distancia;
    double pto1Pr = pto1.RSSI;
    double pto1t = pto1.tiempo;
    double pto2Pr = pto2.RSSI;
    double pto2t = pto2.tiempo;
    distancia = sqrt (pow(pto2Pr-pto1Pr, 2) + pow(pto2t-pto1t, 2));
    return distancia;
}

void CRM_Optm_Calculo_Clusters(){
    Printf("\r\nSe van a calcular los clusters.");
    int i,j;        
    REPO_MSSG_RCVD PeticionRepoListaClusters;
    PeticionRepoListaClusters.Action = ActSndDta;
    PeticionRepoListaClusters.DataType = EnvListaCl;
    BYTE PrimerElemento = 0;
    PeticionRepoListaClusters.Param1 = &PrimerElemento;

    CRM_Message(NMM, SubM_Repo, &PeticionRepoListaClusters);    

    REPO_MSSG_RCVD PeticionRepoLista;
    PeticionRepoLista.Action = ActSndDta;
    PeticionRepoLista.DataType = EnvListaPaq;
    PeticionRepoLista.Param1 = &PrimerElemento;

    CRM_Message(NMM, SubM_Repo, &PeticionRepoLista);
            
    for(i = 0; i < paquetesRecibidos; i++){
        if(nClusters == 0){
            (*(cl*)(PeticionRepoListaClusters.Param2)).centro = *(coord*)PeticionRepoLista.Param2;
            (*(cl*)(PeticionRepoListaClusters.Param2)).radio = initRad;
            (*(cl*)(PeticionRepoListaClusters.Param2)).nMuestras = 1;
            nClusters++;
        } else {
            int clusterActualizado = 0;
            for(j = 0; j < nClusters; j++){
                PeticionRepoListaClusters.Param1 = &j;
                CRM_Message(NMM, SubM_Repo, &PeticionRepoListaClusters);                
                PeticionRepoLista.Param1 = &i;
                CRM_Message(NMM, SubM_Repo, &PeticionRepoLista);
                double distancia = CRM_Optm_Calculo_Distancia((*(cl*)(PeticionRepoListaClusters.Param2)).centro,*(coord*)(PeticionRepoLista.Param2));
                if (distancia <= (*(cl*)(PeticionRepoListaClusters.Param2)).radio){
                    (*(cl*)(PeticionRepoListaClusters.Param2)).radio += distancia;
                    (*(cl*)(PeticionRepoListaClusters.Param2)).centro.RSSI = (((*(cl*)(PeticionRepoListaClusters.Param2)).centro.RSSI * (*(cl*)(PeticionRepoListaClusters.Param2)).nMuestras) + (*(coord*)(PeticionRepoLista.Param2)).RSSI)/((*(cl*)(PeticionRepoListaClusters.Param2)).nMuestras + 1);
                    (*(cl*)(PeticionRepoListaClusters.Param2)).centro.tiempo = (((*(cl*)(PeticionRepoListaClusters.Param2)).centro.tiempo * (*(cl*)(PeticionRepoListaClusters.Param2)).nMuestras) + (*(coord*)(PeticionRepoLista.Param2)).tiempo)/((*(cl*)(PeticionRepoListaClusters.Param2)).nMuestras + 1);
                    (*(cl*)(PeticionRepoListaClusters.Param2)).nMuestras++;
                    clusterActualizado = 1;
                }
            }
            if (clusterActualizado == 0) {
                PeticionRepoListaClusters.Param1 = &nClusters;
                CRM_Message(NMM, SubM_Repo, &PeticionRepoListaClusters);                
                (*(cl*)(PeticionRepoListaClusters.Param2)).centro = *(coord*)(PeticionRepoLista.Param2);
                (*(cl*)(PeticionRepoListaClusters.Param2)).radio = initRad;
                (*(cl*)(PeticionRepoListaClusters.Param2)).nMuestras = 1;
                nClusters++;
            }
        }
    }    
    Printf("\r\nSe han calculado los clusters.");
}

BOOL CRM_Optm_Detectar_Atacante(){
    BYTE RSSI;
    BYTE *pRSSI = &RSSI;
    BYTE *attacker;
    BYTE txAddr = 0;
    BYTE nDetectado = 0;
    attacker = &txAddr;
    MIWI_TICK TiempoActual;
    MIWI_TICK TiempoPaquete;
    coord paquete;
    BYTE i, inclCluster;
    inclCluster = 0;
    
    REPO_MSSG_RCVD PeticionRepoListaClusters;
    PeticionRepoListaClusters.Action = ActSndDta;
    PeticionRepoListaClusters.DataType = EnvListaCl;
    BYTE PrimerElemento = 0;
    PeticionRepoListaClusters.Param1 = &PrimerElemento;

    CRM_Message(NMM, SubM_Repo, &PeticionRepoListaClusters);       
    
    if ( GetRSSI(riActual,pRSSI) == 0x00 ){  //Se ha recibido un paquete
        Printf("\r\nSe ha recibido un paquete\r\n");
        TiempoActual = MiWi_TickGet();
        TiempoPaquete.Val = MiWi_TickGetDiff(TiempoActual, TiempoAnterior);
        TiempoAnterior = TiempoActual;        

        if (TiempoPaquete.Val > 0){ //Se van a descartar los paquetes que se reciban cuando desborde el contador.
            paquete.tiempo = (double) TiempoPaquete.Val/tiempoMax;
            paquete.RSSI = (double) *pRSSI/potenciaMax;

            //Recorrer el array de clusters y comprobar si el paquete pertenece a algun cluster
            for(i = 0; i < nClusters; i++){
                PeticionRepoListaClusters.Param1 = &i;
                CRM_Message(NMM, SubM_Repo, &PeticionRepoListaClusters);                 
                double distancia = CRM_Optm_Calculo_Distancia((*(cl*)(PeticionRepoListaClusters.Param2)).centro,paquete);
                if (distancia < (*(cl*)(PeticionRepoListaClusters.Param2)).radio && inclCluster == 0){
                    inclCluster = 1;
                    break;
                }
            }

            if (inclCluster == 0) {
                Printf("\r\nSe ha detectado atacante.");
                GetRXSourceAddr(riActual, attacker);
                BYTE numNodo;
                switch(attacker[7]){
                    case 0x11:
                        numNodo = 1;
                        break;
                    case 0x22:
                        numNodo = 2;
                        break;
                    case 0x33:
                        numNodo = 3;
                        break;
                    default:
                        numNodo = 0;
                        break;
                }
                char trazaAtt[80];
                sprintf(trazaAtt, "\r\nSe ha detectado como atacante al nodo %d.\r\n", numNodo);
                Printf(trazaAtt);     
                //Añado en la tabla de atacantes al que he detectado
                BYTE dirAtt[MY_ADDRESS_LENGTH], dirDet[MY_ADDRESS_LENGTH], a;
                for (i = 0; i < MY_ADDRESS_LENGTH; i++){
                    dirAtt[i] = *(attacker + i);
                    dirDet[i] = GetMyLongAddress(i);
                }
                
                //Pedimos la tabla de atacantes
                REPO_MSSG_RCVD PeticionRepoTablaAtacantes;
                PeticionRepoTablaAtacantes.Action = ActSndDta;
                PeticionRepoTablaAtacantes.DataType = EnvTablaAt;               
   
                for (i = 0; i < (CONNECTION_SIZE+1)*(CONNECTION_SIZE+1); i++){                
                    PeticionRepoTablaAtacantes.Param1 = &i;
                    CRM_Message(NMM, SubM_Repo, &PeticionRepoTablaAtacantes);
                    if (isSameAddress((*(at*)(PeticionRepoTablaAtacantes.Param2)).direccionAtacante, dirAtt) && isSameAddress((*(at*)PeticionRepoTablaAtacantes.Param2).direccionDetector, dirDet)){
                        if((*(at*)(PeticionRepoTablaAtacantes.Param2)).esAtacante == 5){
                            Printf("\r\nYa habia detectado a ese como atacante.");
                            return FALSE;
                        } else {            
                            (*(at*)PeticionRepoTablaAtacantes.Param2).esAtacante++;
                            char trazaAtt[80];
                            sprintf(trazaAtt, "\r\nSe ha detectado como atacante %d veces.\r\n", (*(at*)PeticionRepoTablaAtacantes.Param2).esAtacante);
                            Printf(trazaAtt);     
                            
                            if ((*(at*)(PeticionRepoTablaAtacantes.Param2)).esAtacante == 5){
                                //Mando un mensaje por VCC al resto de nodos con la informacion del atacante.
                                Printf("\r\nMando informacion del atacante y el detector al resto de nodos.");
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
                        }
                    }
                    if (isSameAddress((*(at*)(PeticionRepoTablaAtacantes.Param2)).direccionAtacante, dirAtt) && (*(at*)(PeticionRepoTablaAtacantes.Param2)).esAtacante != 0){
                        if (isSameAddress((*(at*)(PeticionRepoTablaAtacantes.Param2)).direccionDetector, myLongAddress) && (*(at*)(PeticionRepoTablaAtacantes.Param2)).esAtacante == 5){
                            nDetectado++;
                        } else if (!isSameAddress((*(at*)(PeticionRepoTablaAtacantes.Param2)).direccionDetector, myLongAddress) && (*(at*)(PeticionRepoTablaAtacantes.Param2)).esAtacante == 1){
                            nDetectado++;
                        }
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
                            EXEC_MSSG_RCVD PeticionDisconn;
                            PeticionDisconn.Action = ActDisconn;
                            PeticionDisconn.Param1 = index;
                            CRM_Message(NMM, SubM_Exec, &PeticionDisconn);
                            numNodosRed--;
                            DumpConnection(0xFF);                            
                        }
                    }                                   

                }
                
                return TRUE;
            } else {
                return FALSE;
            }
        }
    }
    return FALSE;
}

/*La rutina propia de ejecución en forma de una interrupción para que ocurra
 de forma periodica (sujeto a que otras interrupciones puedan retrasarla en
 funcion de la prioridad que tengan).*/
BOOL CRM_Optm_Int(void)
{
    tiempo++;
#if defined(TEST5)
    t1G = MiWi_TickGet(); //Cada vez que entra se guarda en t1 el tiempo.
#endif
//    if(!RecibiendoMssg && !EnviandoMssgApp)
#ifdef DATACLUSTERING
    BYTE i;
    for (i = 0; i < numNodosRed; i++){
        if(isSameAddress(ConnectionTable[i].Address, EUINodoExt1)){
#endif
    while(GetPayloadToRead(MIWI_0434)){
        Recibir_info();
    }
    
    if (WhichRIHasData() != 0){
        Recibir_info();
    }    
    
#ifdef DATACLUSTERING
        }
    }
#endif        
        
    CRM_VCC_Mssg_Rcvd(&PeticionRecepcion); //Esto va a enviar o recibir los mensajes de VCC
    
#if defined TEST6
#if defined (NODE_1)//NodoEmisor)
    if(OptmEstado == WaitinAnsw4ChngChn)
    {
        if(MilisDeTimeOut < TIEMPODEESPERARESPUESTA)
            MilisDeTimeOut += 1;
        else if(MilisDeTimeOut == TIEMPODEESPERARESPUESTA)
        {
            //TODO actualizar que no ha aceptado la peticion y eliminar de la tabla de conexiones.
            OptmEstado = Idle; //Ya no esperamos respuesta.
            MilisDeTimeOut = 0;//Ponemos a 0.
//            MiApp_RemoveConnection(0); //Eliminamos de la tabla de conexiones.
        }
    }
#endif
#endif
#if defined GAMETHEORY
    if(EstadoGT != Clear)
    {
        if(MilisDeTimeOut < TIEMPODEESPERARESPUESTA){
            MilisDeTimeOut += 1;
        }
        else if(MilisDeTimeOut == TIEMPODEESPERARESPUESTA)
        {
            Printf("\r\nSe ha acabado el timeout.");
            //TODO actualizar que no ha aceptado la peticion y eliminar de la tabla de conexiones.
            //Se devuelven todas las variables usadas al estado inicial
            EstadoGT = Clear;
            CHNG_MSSG_RCVD = 0;
            inicioCambio = FALSE;
            nRespuestas = 0;
            nRespuestasAfirmativas = 0;
            nRespuestasNegativas = 0;
            MilisDeTimeOut = 0;//Ponemos a 0.
//            MiApp_RemoveConnection(0); //Eliminamos de la tabla de conexiones.
        }
    }
#endif    
        
    if(Cuentamseg<Periodomseg)
    {
        Cuentamseg++;
        return FALSE;
    }
    else if(Cuentamseg==Periodomseg)
    {
        if(!EnviandoMssgApp && !RecibiendoMssg)
        {
            Cuentamseg = 0;
#if defined TEST4 || defined TEST5
#if defined(NODE_2)//NodoReceptor)
            Printf("\r\n\r\nSe ejecuta la parte reactiva de la estrategia cognitiva.");
            //        t2 = MiWi_TickGet(); //XXX-GuilJa lo llevamos al procesado del test
            //        tT = MiWi_TickGetDiff(t2,t1); //más arriba.
            //        Printf("\r\n");
            //        Printf("El numero de Ticks para el nodo receptor (rutina de recepcion y cambio de canal) :");
            //        Printf("\r\n");
            //        PrintDec(tT.byte.MB);
            //        PrintDec(tT.byte.UB);
            //        PrintDec(tT.byte.HB);
            //        PrintDec(tT.byte.LB);
            //        Printf("\r\n");
            //        Printf("Nota: Todo 5s en caso de error.");
            //        Printf("\r\n");
            //        tT.Val = 0xFFFFFFFF; //Por si a la siguiente no hemos entrado en el procesador.
//                    DumpConnection(0xFF); //Para comprobar que hemos cambiado de canal
            return TRUE;
#elif defined(NODE_1)//NodoEmisor
            Printf("\r\n\r\nSe ejecuta la parte activa de la estrategia cognitiva.");
            Test4();
            //        Printf("\r\n");
            //        Printf("El numero de Ticks para el nodo emisor (rutina de optimizacion, envio de mensaje y cambio de canal) :");
            //        Printf("\r\n");
            //        PrintDec(tT.byte.MB);
            //        PrintDec(tT.byte.UB);
            //        PrintDec(tT.byte.HB);
            //        PrintDec(tT.byte.LB);
            //        Printf("\r\n");
            //        Printf("Nota: Todo 5s en caso de error.");
            //        Printf("\r\n");
            //        tT.Val = 0xFFFFFFFF; //Por si a la siguiente no hemos entrado en el procesador.
//                    DumpConnection(0xFF); //Para comprobar que hemos cambiado de canal
            return TRUE;
#endif
#endif
#if defined TEST3
            Test2();
            Cuentamseg = 0;
            return TRUE;
#endif
            //TODO aqui deben ir las llamadas a las funciones que deba ejecutar el
            //optimizer.

#ifdef DATACLUSTERING
    learningTime++;
    if (learningTime == reinicioAtacantesTimeMax){
        Printf("Se reinicia la tabla de atacantes\r\n");        
        RestoreConnTable(TablaConexionesInicial, 1);
        DumpConnection(0xFF);
        REPO_MSSG_RCVD PeticionRepoInitAtt;
        PeticionRepoInitAtt.Action = ActStr;
        PeticionRepoInitAtt.DataType = InitTAtt;

        CRM_Message(NMM, SubM_Repo, &PeticionRepoInitAtt);
        learningTime = 0;
    }
    //Esto lo tiene que hacer hasta que se termine el tiempo de aprendizaje
    if(GetPayloadToRead(riActual) != 0 && learningTime < learningTimeMax && MSSG_PROC_OPTM == 0 && aprendizaje == 0){
        if(!primera){
            primera = 1;
            learningTime = 0;
        }
        MSSG_PROC_OPTM = 1;     
        
        CRM_Optm_Calculo_Coordenadas();
        
    } else if (learningTime == learningTimeMax && aprendizaje == 0){
        aprendizaje = 1;
        REPO_MSSG_RCVD PeticionRepoInitAtt;
        PeticionRepoInitAtt.Action = ActStr;
        PeticionRepoInitAtt.DataType = InitTAtt;

        CRM_Message(NMM, SubM_Repo, &PeticionRepoInitAtt);
        SaveConnTable(TablaConexionesInicial);
        DumpConnection(0xFF);
    }
            
    if (aprendizaje == 1){
        if(normalizado == 0) {
            REPO_MSSG_RCVD PeticionRepoLista;
            PeticionRepoLista.Action = ActSndDta;
            PeticionRepoLista.DataType = EnvListaPaq;

            CRM_Message(NMM, SubM_Repo, &PeticionRepoLista);
            
            CRM_Optm_Normalizar_Coordenadas();
            normalizado = 1;
        } else if (normalizado == 1 && clustersDone == 0) {
            CRM_Optm_Calculo_Clusters();
            clustersDone = 1;
        } else {
            if(GetPayloadToRead(riActual) != 0 && MSSG_PROC_OPTM == 0){
                MSSG_PROC_OPTM = 1;
                if (learningTime == 0xEFFFFFFF){
                    learningTime = 0;
                }
                CRM_Optm_Detectar_Atacante();
            }
        }
    }
    
    /*if (learningTime == 3000){
        SetTXPower(riActual, 0xFF);
    }*/
#endif



#if defined GAMETHEORY 
    if(GetPayloadToRead(riActual) && MSSG_PROC_OPTM == 0){
        MSSG_PROC_OPTM = 1;
        BYTE Direccion[MY_ADDRESS_LENGTH];
        BYTE err;
        err = GetRXSourceAddr(riActual, Direccion);
        if (err & 0x80) {
            Printf("\r\nError al obtener la direccion: ");
            PrintChar(err);
            return FALSE;
        } else {
            REPO_MSSG_RCVD PeticionRepoMssg;
            PeticionRepoMssg.Action = ActStr;
            PeticionRepoMssg.DataType = AddMsg;
            PeticionRepoMssg.Param1 = Direccion;

            CRM_Message(NMM, SubM_Repo, &PeticionRepoMssg);
        }
    }
           
        
    if (EstadoGT == Clear){                        
        //Pedir a repo el numero de retransmisiones en el canal actual        
        BYTE BackupCanal = GetOpChannel(riActual);
        REPO_MSSG_RCVD PeticionRepoRTx;
        PeticionRepoRTx.Action = ActSndDta;
        PeticionRepoRTx.DataType = EnvRTx;
        PeticionRepoRTx.Param1 = &BackupCanal;
        PeticionRepoRTx.Transceiver = riActual;

        CRM_Message(NMM, SubM_Repo, &PeticionRepoRTx);        
        num_rtx = *(BYTE*)(PeticionRepoRTx.Param2);
        
        inicioCambio = CRM_Optm_Calcular_Costes(num_rtx);
        //Reinicio el numero de retransmisiones        
        REPO_MSSG_RCVD PeticionRepoResetRTx;
        PeticionRepoResetRTx.Action = ActStr;
        PeticionRepoResetRTx.DataType = RstRTx;
        CRM_Message(NMM, SubM_Repo, &PeticionRepoResetRTx);
        if (inicioCambio){           
            char traza[80];
            sprintf(traza, "\r\n\r\nHa habido %d retransmisiones en el canal actual.", num_rtx);
            Printf(traza);            
            MSSG_PROC_OPTM = 1;
            OPTM_MSSG_RCVD PeticionInicioCambio;
            PeticionInicioCambio.Action = SubActCambio;
            PeticionInicioCambio.Transceiver = riActual;
            CRM_Optm_Cons(&PeticionInicioCambio);
        }            
    }
#endif    
        }
    }
    return FALSE;
}

BOOL CRM_Timer5_Int(void)
{    
    BYTE i;
    BYTE nodo = 0;
    BYTE delay = rand() % 100;//Si se quiere un tiempo fijo quitar rand y poner 0
    if(mseg<Periodo)
    {
        mseg++;
    }
    else if(mseg>=(Periodo+delay))
    {
        if(!EnviandoMssgApp && !RecibiendoMssg)
        {
            mseg = 0;
#if defined GAMETHEORY
            #if defined NODE_1
//            if(nodo = 0){
                Enviar_Paquete_Datos_App(riActual, LONG_MIWI_ADDRMODE, &EUINodoExt1);
//                nodo = 1;
//            } else {
//                Enviar_Paquete_Datos_App(riActual, LONG_MIWI_ADDRMODE, &EUINodoExt2);
//                nodo = 0;
//            }
            #elif defined NODE_2
            Enviar_Paquete_Datos_App(riActual, LONG_MIWI_ADDRMODE, &EUINodoExt1);
            #elif defined NODE_3
            Enviar_Paquete_Datos_App(riActual, LONG_MIWI_ADDRMODE, &EUINodoExt1);
            #endif
#endif
#ifdef DATACLUSTERING
            for (i = 0; i < numNodosRed; i++){
                if(isSameAddress(ConnectionTable[i].Address, EUINodoExt1)){
                    Enviar_Paquete_Datos_App(riActual, LONG_MIWI_ADDRMODE, &EUINodoExt1);
                }
            }
#endif
        }
    }
    
    #ifdef NODE_1    
    if(tiempoCambio<tiempoCambioPotTX) {
        tiempoCambio++;
    } else if(tiempoCambio==tiempoCambioPotTX) {
        SetTXPower(riActual, 0xFF);
        Periodo = 250;
        Printf("\r\n/////////////////////////////////Se cambia la potencia de transmision.");
        tiempoCambio = tiempoCambioPotTX+100; //Para que no siga sumando ni cambiando la potencia
    }

    if(tiempoCambio<tiempoCambioFrecPaquetes) {
        tiempoCambio++;
    } else if(tiempoCambio==tiempoCambioFrecPaquetes) {
        
        Printf("\r\nSe cambia el tiempo entre paquetes.");
        tiempoCambio = tiempoCambioFrecPaquetes+100;
    }
    
    #endif
    
}

#if defined TEST3
//TEST2-BORRABLE
/*Solo util para realizar el test2*/
//Estamos intentando simular una situacion en la que optimizer solicita una 
//accion a discovery y en consecuencia ordena a executor llevar a cabo otra
//determinada accion. En concreto mide niveles de señal en discovery y a 
//continuacion cambia el canal al 15.
BYTE Test2(void)
{
    //Hacemos una invocacion primero a Discovery.
    DWORD TodosCanales = 0xFFFFFFFF;
    BYTE TiempoScan = 10;
    BYTE TipoScan = NOISE_DETECT_ENERGY;
    //Creacion de un mensaje de discovery.
    DISC_MSSG_RCVD PeticionDiscTest2;
    PeticionDiscTest2.OrgModule = SubM_Opt;
    PeticionDiscTest2.Action = ActSignDetc;
    PeticionDiscTest2.Param1 = &TodosCanales;
    PeticionDiscTest2.Param2 = &TiempoScan;
    PeticionDiscTest2.Param3 = &TipoScan; //NOISE DETECT ENERGY.
    CRM_Message(NMM, SubM_Disc, &PeticionDiscTest2);//envio del messge.
    //Creacion de un mensaje para executor.
    EXEC_MSSG_RCVD PeticionExecTest2;
    PeticionExecTest2.OrgModule = SubM_Opt;
    PeticionExecTest2.Action = ActChnHop;
    PeticionExecTest2.Param1 = 15;
    CRM_Message(NMM, SubM_Exec, &PeticionExecTest2);//envio del messg.

    DumpConnection(0xFF);//Para comprobar que hemos cambiado de canal
}
#endif
/*Fin de la funcion para el test2*/

#if defined(TEST4)
BYTE Test4(void)
{
    PeticionTest2.Transceiver = ri;
    CRM_Optm_GameTheory(&PeticionTest2);
    return 0;
}
#endif

/*****************************************************************************/
/******************************FIN DE FUNCIONES*******************************/
/*****************************************************************************/