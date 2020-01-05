#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
//Tamanio de la cola de solicitudes
#define true 1
#define false 0
int tamCola;
int numAtendedoresPRO;
//Defino las funciones
int salidaApta();
void *accionesAtendedor(void *ptr);
void *accionesSolicitud(void *ptr);
void manejadoraSolicitud(int signal);
void manejadoraFinalizar(int signal);
void manejadoraSolicitudMaxima(int signal);
void manejadoraAtendedorMaxima(int signal);
void writeLogMessage (char *id , char *msg);
int sacarNumero(char *id);
void *usuarioEnActividad(void *id);
void *accionesCoordinadorSocial();
void creadorAtendedoresPRO();
pthread_mutex_t mutexLog;
pthread_mutex_t mutexColaSolicitudes;
pthread_mutex_t mutexColaSocial;
pthread_mutex_t salir;
pthread_cond_t condActividades;
FILE *logFile;
//Condicion de salida
int finPrograma = false;
typedef struct solicitud {
	char id[50];
	//0->Sin atender 1->En proceso de atención 2->Atendido 3->Atendido con Antecedentes
	int atendido;
	//Tipo 0->Invitación 1->QR
	int tipo;
	pthread_t hilo;
} Solicitud;
void inicializarSolicitud(Solicitud* solicitud);
//Cola de solicitudes
Solicitud *colaSolicitudes;
int idUsuariosActividad[4];
//Para la asignacion de ID a las solicitudes
int contadorSolicitudes;
//Para saber cuantos hay en cola
int contadorSolicitudesCola;
int contadorActividadesCola;
//Para saber 
int listaCerrada;
int tipoAtendedor[3]={1,2,3};

/*
 *	Main
 */

int main(int argc, char **argv){
	//Imprime el pid para poder utilizar el mandador
	printf("Pid: %d\n",getpid());
	//Tipo de atendedor 1->Invitacion 2->QR 3->PRO
//	int tipoAtendedor[3]={1,2,3};
	int i;
	char buffer[100];
	//Tratamiento de señales
	struct sigaction sSolicitud={0};
	struct sigaction sFinalizar={0};
	sSolicitud.sa_handler=manejadoraSolicitud;
	sigaction(SIGUSR1,&sSolicitud,NULL);
	sigaction(SIGUSR2,&sSolicitud,NULL);
	sFinalizar.sa_handler=manejadoraFinalizar;
	sigaction(SIGINT,&sFinalizar,NULL);
	//Alterar numero de solicitudes
	struct sigaction sAnyadirSolicitudMaxima={0};
	sAnyadirSolicitudMaxima.sa_handler=manejadoraSolicitudMaxima;
	sigaction(SIGTERM,&sAnyadirSolicitudMaxima,NULL);
	//Alterar numero de atendedores
	struct sigaction sAnyadirAtendedorMaxima={0};
	sAnyadirAtendedorMaxima.sa_handler=manejadoraAtendedorMaxima;
	sigaction(SIGALRM, &sAnyadirAtendedorMaxima, NULL);
	//Inicializar recursos	
	srand(time(NULL));
	contadorSolicitudes=0;
	contadorSolicitudesCola=0;
	contadorActividadesCola=0;
	listaCerrada=false;
	logFile=NULL;
	if(argc != 3 || atoi(argv[1]) <= 0){
		printf("Error en los parametros de ejecucion. Ejecute como ./ejecutable maxSolicitudes[>0] atendedoresPro\n");
	} else {
		tamCola=atoi(argv[1]);
		numAtendedoresPRO = atoi(argv[2]);
		printf("Cola:%d \n", tamCola);
		colaSolicitudes = (Solicitud*)malloc(sizeof(Solicitud)*tamCola);
		for(i=0;i<tamCola;i++){
			inicializarSolicitud(&colaSolicitudes[i]);
		}
		for(i=0;i<4;i++){
			idUsuariosActividad[i]=0;
		}
		pthread_mutex_init(&mutexColaSocial,NULL);
		pthread_mutex_init(&mutexLog, NULL);
		pthread_mutex_init(&mutexColaSolicitudes, NULL);
		pthread_cond_init(&condActividades, NULL);
		pthread_mutex_init(&salir, NULL);
		//Encargados de las solicitudes de:
		pthread_t atendedor_1, atendedor_2, coordinador;
		//Creamos los hilos de los "usuarios destacados"
		//argv[1] numero de solicitudes maximo
		printf("Iniciando tsunami\n");
		writeLogMessage("1","Iniciando tsunami");
		pthread_create(&atendedor_1, NULL, accionesAtendedor, (void *) &tipoAtendedor[0]);
		sprintf(buffer,"Atendedor_%d ha sido creado\n",tipoAtendedor[0]);
		writeLogMessage ( "1" , buffer);
		printf("Atendedor_%d ha sido creado\n",tipoAtendedor[0]);
		//writeLogMessage("1","Atendedor_%d ha sido creado"tipoAtendedor[0]);
		pthread_create(&atendedor_2, NULL, accionesAtendedor, (void *) &tipoAtendedor[1]);
		sprintf(buffer,"Atendedor_%d ha sido creado\n",tipoAtendedor[1]);
		writeLogMessage ( "1" , buffer);
		printf("Atendedor_%d ha sido creado\n",tipoAtendedor[1]);
		//writeLogMessage("1","Atendedor_%d ha sido creado",tipoAtendedor[1]);
		//Funcion que crea los atendedores PRO en funcion de la variable pasada como argumento
		creadorAtendedoresPRO(tipoAtendedor);
		//Creamos el hilo del coordinador
		pthread_create(&coordinador, NULL, accionesCoordinadorSocial, NULL);

		while(true){
			pause();
		}
	}
	return 0;
}
/**
 * Función que crea los hilos de los AtendedoresPRO
 * @autor Diego Narciandi
*/
void creadorAtendedoresPRO(int tipoAtendedor[]){
	int i = 0;
	char buffer[100];
	int atendedoresPRO[numAtendedoresPRO];
	//Inicializo el vector de atendedoresPRO con sus IDs
	for(i = 0 ; i < numAtendedoresPRO ; i++){
		//Empiezo en 3 ya que el atendedor 1 y 2 ya han sido creados
		atendedoresPRO[i] = i+3;
	}
	pthread_t atendedorPRO;
	for(i = 0 ; i < numAtendedoresPRO ; i++){
		pthread_create(&atendedorPRO, NULL, accionesAtendedor, (void *) &tipoAtendedor[2]);
		sprintf(buffer,"Atendedor_%d ha sido creado\n",atendedoresPRO[i]);
		writeLogMessage ( "1" , buffer);
		printf("Atendedor_%d ha sido creado\n",atendedoresPRO[i]);
	}
}

void nuevaSolicitud(int signal){
	printf("Nueva solicitud\n");
	//Variable para imprimir los mensajes /*sprintf(buffer, "mensaje");*/ /*writeLogMessage (char *id , char *msg)*/
	char buffer[500];
	//Declaro la solicitud
	Solicitud *solicitud;
	//Contador del bucle
	int i = 0;
	//Condicion de salida (Solicitud guardada)
	int guardado = false;
	//Variable para el numero de solicitud
	int valorId = 0;
	//Cola de solicitudes aun abierta
	if(finPrograma == false){
	//Seccion Critica
		pthread_mutex_lock(&mutexColaSolicitudes);
		//Compruebo el espacio en la lista de solicitudes
		while((i < tamCola) && (guardado == false)){
			//Si el hueco de la solicitud esta libre, entro
			if(strcmp(colaSolicitudes[i].id,"0")==0){
				solicitud = &colaSolicitudes[i];
				//Introduzco el identificador con su valor y lo incremento
				contadorSolicitudes++;
				//Guardo el valor en un char para introducirlo en id
				valorId = contadorSolicitudes;
				//Invitacion
				if(signal == SIGUSR1){
					solicitud->tipo = 0;
				//QR
				}else{
					solicitud->tipo = 1;
				}
				//Solicitud guardada, salgo del bucle
				guardado = true;
			//Posicion ya ocupada
			}	
		i++;
		}
	//Fin de la seccion critica
		pthread_mutex_unlock(&mutexColaSolicitudes);
		if(guardado == true){
			//Introduzco el numero en el identificador de la solicitud
			sprintf(solicitud->id,"solicitud_%d",valorId);
				printf("%s añadida a la lista de solicitudes\n",solicitud->id);
				sprintf(buffer, "%s añadida a la lista de solicitudes\n",solicitud->id);
				printf("%s lista para ser atendida\n",solicitud->id);
				sprintf(buffer, "%s lista para ser atendida\n",solicitud->id);
				//Indico el tipo de solicitud segun la senial
				//Invitacion
				if(signal == SIGUSR1){
					printf("%s de invitacion\n",solicitud->id);
					sprintf(buffer, "%s de invitacion\n",solicitud->id);
				//QR
				}else{
					printf("%s por codigo QR\n",solicitud->id);
					sprintf(buffer,"%s por codigo QR\n",solicitud->id);
				}
			//Mando la solicitud para ser procesada
			pthread_create(&solicitud->hilo, NULL, accionesSolicitud, (void *) solicitud);
			//Envio el mensaje guardado en el buffer a la funcion writeLogMessage
			writeLogMessage ( solicitud->id , buffer);
		}
	}
	//Cola llena o Cola de solicitudes cerrada
	if((finPrograma == true) || (guardado == false)){
		if(finPrograma == true){
			printf("Imposible entrar en la cola, ha sido cerrada\n");
			sprintf(buffer,"Imposible entrar en la cola, ha sido cerrada\n");
			writeLogMessage ( "FIN" , buffer);
		}else{
			printf("Cola de solicitudes llena, Solicitud ignorada\n");
			sprintf(buffer,"Cola de solicitudes llena, Solicitud ignorada\n");
			writeLogMessage ( "ERROR" , buffer);
		}
	}
}
/**
 * Función que se encarga de las acciones de la solicitud
 * @autor Marcos Ferreras
*/
void *accionesSolicitud(void *structSolicitud){
	//char idSolicitud[50], cadenaLog[100];
	Solicitud* solicitud = (Solicitud *)structSolicitud;
	//0->Sin atender 1->En proceso de atención 2->Atendido 3->Atendido con Antecedentes
	int enAtencion=0;
	int aleatorio;
	int participo;
	char id[50];
	
	//strcpy(solicitud->id,"1");
	//if(strcmp("0\0",solicitud->id)==0)	
	//	printf("Hola\n");
	if(solicitud->tipo==0){
		printf("ID %s : La solicitud por invitacion se ha registrado\n", solicitud->id);
		writeLogMessage(solicitud->id,"La solicitud por invitacion se ha registrado");
	} else {
		printf("ID %s : La solicitud por QR se ha registrado\n", solicitud->id);
		writeLogMessage(solicitud->id,"La solicitud por QR se ha registrado");
	}
	//printf("Hola\n");
	//writeLogMessage("1111","5");
	sleep(4);
	pthread_mutex_lock(&mutexColaSolicitudes);
	//Copio el id
	strcpy(id,solicitud->id);
	enAtencion=solicitud->atendido;
	//Mientras esté sin atender
	while(enAtencion==0){
		//Invitación
		aleatorio = rand()%101;
		if(solicitud->tipo==0){
			if(aleatorio<=10){
				//TODO Escribir en log
				printf("ID %s : La solicitud por invitacion se ha eliminado por cansarse de esperar\n", solicitud->id);
				writeLogMessage(solicitud->id,"La solicitud por invitacion se ha eliminado por cansarse de esperar");
				inicializarSolicitud(solicitud);
				pthread_mutex_unlock(&mutexColaSolicitudes);
				pthread_exit(NULL); 
			}
		//QR
		} else {
			if(aleatorio<=30){
				//TODO Escribir en log
				printf("ID %s : La solicitud por QR se ha eliminado por no considerarse muy fiable\n", solicitud->id);
				writeLogMessage(solicitud->id,"La solicitud por QR se ha eliminado por no considerarse muy fiable");
				inicializarSolicitud(solicitud);
				pthread_mutex_unlock(&mutexColaSolicitudes);
				pthread_exit(NULL); 
			}
		}
		//Descarte fallo aplicacion
		aleatorio = rand()%101;
		if(aleatorio<=15){
			//TODO Escribir en log
			printf("ID %s : La solicitud se ha eliminado por fallo de la aplicacion\n", solicitud->id);
			writeLogMessage(solicitud->id,"La solicitud se ha eliminado por fallo de la aplicacion");	
			inicializarSolicitud(solicitud);
			pthread_mutex_unlock(&mutexColaSolicitudes);
			pthread_exit(NULL);
		}
		pthread_mutex_unlock(&mutexColaSolicitudes);
		sleep(4);
		pthread_mutex_lock(&mutexColaSolicitudes);
		enAtencion=solicitud->atendido;
		
	}
	//enAtencion=solicitud->atendido;
	//Mientras me esten atendiendo esperaré. No he soltado el mutex cuando salí del bucle anterior.
	while(enAtencion==1){
		pthread_mutex_unlock(&mutexColaSolicitudes);
		sleep(2);
		pthread_mutex_lock(&mutexColaSolicitudes);
		enAtencion=solicitud->atendido;
	}
	//Al salir de este bucle no he soltado el mutex. En este punto lo desbloqueo, y decido si me uno a una actividad social o no (Si puedo).
	enAtencion=solicitud->atendido;
	pthread_mutex_unlock(&mutexColaSolicitudes);
	if(enAtencion == 2){
		participo=rand()%2;
		//0->Si Participo 1->No Participo
		if(participo==0){
			//Libero hueco en solicitudes
			printf("ID %s : El usuario ha decidido participar en una actividad cultural\n", solicitud->id);
			writeLogMessage(solicitud->id,"El usuario ha decidido participar en una actividad cultural");
			pthread_mutex_lock(&mutexColaSocial);
			while(listaCerrada==true){
				pthread_mutex_unlock(&mutexColaSocial);
				printf("ID %s : La lista de actividades esta cerrada. Esperando...\n", solicitud->id);
				writeLogMessage(solicitud->id,"La lista de actividades esta cerrada. Esperando...");
				sleep(3);
				pthread_mutex_lock(&mutexColaSocial);
			}
			//Copio el id del usuario a la actividad
			idUsuariosActividad[contadorActividadesCola]= sacarNumero(id);
			//Incremento el numero de actividades
			contadorActividadesCola++;
			if(contadorActividadesCola==4){
				//Aviso al coordinador que soy el ultimo
				printf("ID %s : Soy el ultimo para la actividad. Voy a avisar al coordinador\n", solicitud->id);
				writeLogMessage(solicitud->id,"Soy el ultimo para la actividad. Voy a avisar al coordinador");
				listaCerrada = true;
				pthread_cond_signal(&condActividades);
			}
			pthread_mutex_unlock(&mutexColaSocial);
			//Libero espacio en la cola
			pthread_mutex_lock(&mutexColaSolicitudes);
			inicializarSolicitud(solicitud);
			pthread_mutex_unlock(&mutexColaSolicitudes);
			pthread_exit(NULL);
			
		} else {
			printf("ID %s : El usuario ha decidido NO participar en una actividad cultural\n", solicitud->id);
			writeLogMessage(solicitud->id,"El usuario ha decidido NO participar en una actividad cultural");
			//Libero espacio en cola de solicitudes
			pthread_mutex_lock(&mutexColaSolicitudes);
			inicializarSolicitud(solicitud);
			pthread_mutex_unlock(&mutexColaSolicitudes);
		}
		pthread_exit(NULL);
	} else if(enAtencion==3){
		//Solicitud con antecedentes. Libero el hueco en la cola y finalizo hilo.
		printf("ID %s : El usuario tiene antecedentes y no se puede unir a ninguna actividad\n", solicitud->id);
		writeLogMessage(solicitud->id,"El usuario tiene antecedentes y no se puede unir a ninguna actividad");
		pthread_mutex_lock(&mutexColaSolicitudes);
		inicializarSolicitud(solicitud);
		pthread_mutex_unlock(&mutexColaSolicitudes);
		pthread_exit(NULL);
	}	
}
/**
 *@author Mario Alvarez
 *Procesa las solicitudes. 
*/
void *accionesAtendedor(void *ptrs){
	//El atendedor 1 atiende solicitudes tipo 0, el 2 tipo 1 y el 3 todas las anteriores
	//El tipo de solicitudes viene dado por el numero del atendedor en cuestion
	int tipoEvaluacion,tipoEvaluacionEstatica, id, contador=0, calculo, espera, apto, posicion=0, cafe=0,i;
	char atendedor[25], salida[150];
	sprintf(atendedor,"Atendedor_%d",*(int *)ptrs);
	tipoEvaluacionEstatica=*(int *)ptrs-1;
	do{
		//Se ponen todos los valores a cero antes de cada iteracion
		contador=0;
		apto=true;
		cafe++;
		do{
			//Se comprueba si la cola esta cerrada y si lo esta se comprueba que haya usuarios remanentes para saber si puede finalizarse la aplicacion
			pthread_mutex_lock(&salir);
			if(finPrograma) {
				pthread_mutex_unlock(&mutexColaSolicitudes);
				if(salidaApta()){
						printf("Atendedor_%d: la cola de solicitudes esta vacia y se procede a la finalizacion del programa\n",*(int *)ptrs);
						sprintf(salida,"la cola de solicitudes esta vacia y se procede a la finalizacion del programa");
						writeLogMessage(atendedor,salida);
					
						pthread_mutex_unlock(&salir);
						
						exit(0);//Se sale del programa
				}
				pthread_mutex_unlock(&mutexColaSolicitudes);
			}	
			pthread_mutex_unlock(&salir);
			//Se vigilan las iteraciones pares ya que en ellas se reinicia el tipo de evaluacion 
			if(contador%2==0 || contador==0){
					tipoEvaluacion=tipoEvaluacionEstatica;
					pthread_mutex_lock(&mutexColaSolicitudes);
			}
					
			contador++;
			id=-1;
			for(i=0;i<tamCola;i++){
				//Se evalua tambien en >2 para esos atendedores pro que se añadieron de mas
				if((colaSolicitudes[i].tipo==tipoEvaluacion || tipoEvaluacion>=2) && colaSolicitudes[i].atendido==0){
					//El valor por defecto del id es-1 y por eso se valora a parte
					//Se seleccionara siempre la id mas baja ya que en teoria es la que mas tiempo lleva esperando
					if((id>sacarNumero(colaSolicitudes[i].id) || id==-1)){
						//Se comprueba el !=0 ya que las id por defecto son 0 y es un numero muy bajo que da problemas
						if(sacarNumero(colaSolicitudes[i].id)!=0){
							id=sacarNumero(colaSolicitudes[i].id);
							posicion=i;
						}				
					}
				}
			}
			//Si no se ha encontrado nada se vuelve al pricipio con el tipo =2(PRO)
			if(id==-1){
				tipoEvaluacion=2;
				//En las iteraciones pares se libera el mutex y se duerme
				if(contador%2==0){
					//printf("%d-Par: %d\n",*(int *)ptrs,id);
					pthread_mutex_unlock(&mutexColaSolicitudes);
					sleep(1);
				}
			}
		}while(id==-1);
		colaSolicitudes[posicion].atendido=1;//Se cambia el flag antes de liberar el mutex para evitar cambios en los datos
		pthread_mutex_unlock(&mutexColaSolicitudes);
		printf("Atendedor_%d: comienza a procesar la solicitud_%d\n",*(int *)ptrs,id);
		sprintf(salida,"comienza a procesar la solicitud_%d",id);
		writeLogMessage(atendedor,salida);
		srand(time(NULL));
		//se genera un numero aleatorio para saber el estado de atencion
		calculo=rand()%10+1;
		if(calculo<=7){
			espera=rand()%4+1;
			printf("Atendedor_%d: todo correcto al procesar la solicitud_%d\n",*(int *)ptrs,id);
			sprintf(salida,"todo correcto al procesar la solicitud_%d",id);
			writeLogMessage(atendedor,salida);
		}else if(calculo<=9){
			espera=rand()%5+2;
			printf("Atendedor_%d: existe un error en los datos de la solicitud_%d\n",*(int *)ptrs,id);
			sprintf(salida,"existe un error en los datos de la solicitud_%d",id);
			writeLogMessage(atendedor,salida);
		}else{
			printf("Atendedor_%d: el usuario de solicitud_%d posee antecedentes\n",*(int *)ptrs,id);
			sprintf(salida,"el usuario de solicitud_%d posee antecedentes",id);
			writeLogMessage(atendedor,salida);
			espera=rand()%5+6;
			apto=false;
		}
		sleep(espera);//Se espera en funcion de lo sucedido previamente
		printf("Atendedor_%d: el tiempo necesario para atender la solicitud_%d ha sido %d\n",*(int *)ptrs,id,espera);
		sprintf(salida,"el tiempo necesario para atender la solicitud_%d ha sido %d",id,espera);
		writeLogMessage(atendedor,salida);
		//Se procede a cambiar el estado de la solicitud en caso de si tiene antecedentes o no
		pthread_mutex_lock(&mutexColaSolicitudes);
		if(apto==false){//Tiene antecedentes
			colaSolicitudes[posicion].atendido=3;
		} else{//No tiene antecedentes
			colaSolicitudes[posicion].atendido=2;
		}
		pthread_mutex_unlock(&mutexColaSolicitudes);
		
		if(cafe%5==0){//En las rondas multiplo de cinco se duerme para el cafe
			printf("Atendedor_%d: procede a tomarse un cafe\n",*(int *)ptrs);
			sprintf(salida,"procede a tomarse un cafe");
			writeLogMessage(atendedor,salida);
			sleep(10);
		}
		
	}while(true);
}
void *accionesCoordinadorSocial(){
	pthread_t usuario1, usuario2, usuario3, usuario4;
	while(true){
		pthread_mutex_lock(&mutexColaSocial);
		pthread_cond_wait(&condActividades, &mutexColaSocial);

		listaCerrada = true;
		writeLogMessage("", "Lista cerrada");


		pthread_create(&usuario1, NULL, usuarioEnActividad, (void *) &idUsuariosActividad[0]);
		pthread_create(&usuario2, NULL, usuarioEnActividad, (void *) &idUsuariosActividad[1]);
		pthread_create(&usuario3, NULL, usuarioEnActividad, (void *) &idUsuariosActividad[2]);
		pthread_create(&usuario4, NULL, usuarioEnActividad, (void *) &idUsuariosActividad[3]);


		pthread_cond_wait(&condActividades, &mutexColaSocial);

		listaCerrada = false;
		writeLogMessage("", "Lista abierta de nuevo");

		pthread_mutex_unlock(&mutexColaSocial);
	}
}
/**
*@author Marcos Ferreras
*Esta función se encarga de realizar la actividad. Es continuación de accionesSolicitud
*/
void *usuarioEnActividad(void *id){
	char idUser[50];
	sprintf(id,"Usuario_%d", *(int *)id);
	printf("Usuario_%d -> Se ha unido a una actividad cultural\n", *(int *)id);
	writeLogMessage(idUser, "Se ha unido a una actividad cultural");

	
	sleep(3);

	pthread_mutex_lock(&mutexColaSocial);
	contadorActividadesCola--;
	if(contadorActividadesCola==0){
		printf("Usuario_%d -> Soy el último en finalizar la actividad. Voy a avisar al coordinador\n", *(int *)id);
		writeLogMessage(id,"Soy el último en finalizar la actividad. Voy a avisar al coordinador");
		pthread_cond_signal(&condActividades);
	}
	printf("Usuario_%d -> Ha finalizado la actividad cultural\n", *(int *)id);
	writeLogMessage(id, "Ha finalizado la actividad cultural");
	pthread_mutex_unlock(&mutexColaSocial);
	pthread_exit(NULL);
}
/**
*Registra los mensajes e id en un fichero de log.
*/
void writeLogMessage (char *id , char *msg) {
	int i;
	pthread_mutex_lock(&mutexLog);
	if(logFile==NULL){
		logFile = fopen ("registroTiempos.log", "w") ;
	} else {
		logFile = fopen ("registroTiempos.log", "a") ;
	}
	// Calculamos la hora actual
	time_t now = time (0) ;
	struct tm *tlocal = localtime (&now) ;
	char stnow [23];
	//strcpy(stnow,"");
	strftime (stnow, 23, " %d/ %m/ %y %H: %M: %S " , tlocal) ;
	// Escribimos en el log. La primera vez de cada ejecución, borrará el log.txt en caso de que exista.
	fprintf (logFile , "[%s] %s : %s\n" , stnow , id , msg) ;
	fclose (logFile);
	pthread_mutex_unlock(&mutexLog); 

}
void manejadoraSolicitud(int signal){
	nuevaSolicitud(signal);
		
}
void manejadoraFinalizar(int signal){
	//Salida del programa, se cierra la cola
	pthread_mutex_lock(&salir);
	finPrograma = true;
	pthread_mutex_unlock(&salir);
	printf("La cola de solicitudes ha sido cerrada\n");
	writeLogMessage ( "FIN" , "La cola de solicitudes ha sido cerrada");
}
/**
 *@author Marcos Ferreras
 *Inicializa los campos de la estructura solicitud a 0.  
*/
void inicializarSolicitud(Solicitud* solicitud){
	strcpy(solicitud->id, "0");
	solicitud->atendido=0;
	solicitud->tipo=0;
	solicitud->hilo=0;
}
int sacarNumero(char *id){
	int numero=0,posicion_=0, contador=0,i;
	for (i=0;i<strlen(id);i++){
		if(id[i]>='0' && id[i]<='9'){
			numero=numero*10 + id[i] - '0';
		}
	}
	return numero;
}
/**
 *@author Mario Alvarez
 *Comprueba que no haya solicitudes sin procesar.  
*/
int salidaApta(){
	int output=true,i;
	pthread_mutex_lock(&mutexColaSolicitudes);
	for(i=0;i<tamCola;i++){
		if(strcmp(colaSolicitudes[i].id,"0")!=0){
			output=false;
		}
	}
	pthread_mutex_unlock(&mutexColaSolicitudes);
	return output;
}
/**
 *@author Mario Alvarez
 *Amplia la cola de solicitudes durante la ejecucion.  
*/
void manejadoraSolicitudMaxima(int signal){
	char movimiento[5];
	int numeroMovimientos, i;
	printf("Para reformar la cola seleccione una cantidad + a ampliar\n");
	scanf("%s",movimiento);
	numeroMovimientos=atoi(movimiento);
		pthread_mutex_lock(&mutexColaSolicitudes);
		//Se genera una cola auxiliar para guardar los datos
		Solicitud *colaAux = (Solicitud*)malloc(sizeof(Solicitud)*tamCola);
		for(i=0;i<tamCola;i++){
			colaAux[i]=colaSolicitudes[i];
		}
		//Se desplazan los datos de un puntero a otro
		colaSolicitudes = realloc(colaSolicitudes,sizeof(Solicitud)*(tamCola+numeroMovimientos));
		for(i=0;i<tamCola;i++){
			colaSolicitudes[i]=colaAux[i];
		}
		//Se inicializan las solicitudes nuevas
		for(i=0;i<numeroMovimientos;i++){
			inicializarSolicitud(&colaSolicitudes[tamCola + i]);
		}
		//Se amplia la cola
		tamCola=+numeroMovimientos;
		free(colaAux);
		pthread_mutex_unlock(&mutexColaSolicitudes);
		printf("Cola de solicitudes ampliada\n");
		writeLogMessage ( "Main" , "La cola de solicitudes ha sido ampliada");
}

void manejadoraAtendedorMaxima(int signal){
	int tipo, numero, i;
	pthread_t atendedorPROampliar;
	char buffer[100];
	
	printf("\nIndique el numero de atendedoresPRO a ampliar\n\n");
	scanf("%d", &numero);

	int atendedoresPROampliar[numero];

	if(numero>0){
		
		//Inicializo el vector de atendedoresPRO con sus IDs
		for(i = 0; i < numero; i++){
			atendedoresPROampliar[i] = i + numAtendedoresPRO + 3;
		}

		for(i = 0; i < numero; i++){
			pthread_create(&atendedorPROampliar, NULL, accionesAtendedor, (void *) &tipoAtendedor[2]);
			//sprintf(buffer,"Atendedor_%d ha sido creado\n",atendedoresPROampliar[i]);
			writeLogMessage ( "1" , buffer);
			printf("Atendedor_%d ha sido creado\n",atendedoresPROampliar[i]);
		}
		numAtendedoresPRO = numero + numAtendedoresPRO;
	} else {
		printf("El cambio no tendra repercusion en el programa. Introduca mas de 3 atendedores para incrementarlos.");
		writeLogMessage("Main","El cambio no tendra repercusion en el programa. Introduca mas de 3 atendedores para incrementarlos.");
	}
}
