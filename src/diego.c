//Resumen
/*
Dudas:
*/
/*
Trabajando en:
	Corregida la funcion, actualizada para funcionar con la nueva actualizacion del codigo
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
//Tamanio de la cola de solicitudes
#define tamCola 15
#define true 1
#define false 0

//Estructuras

//Estructura de actividad
typedef struct actividad{
	char idUsuario[50];
	pthread_t hilo;
} Actividad;

//Estructura de solicitud
typedef struct solicitud {
	char id[50];
	//0->Sin atender 1->En proceso de atención 2->Atendido 3->Atendido con Antecedentes
	int atendido;
	//Tipo 0->Invitación 1->QR
	int tipo;
	pthread_t hilo;
} Solicitud;

//Vector con el identificador de los atendedores
int id[3]={1,2,3};
//Defino las funciones
void *accionesAtendedor(void *ptr);
void *accionesSolicitud(void *ptr);
void *accionesCoordinadorSocial(void *ptr);
void manejadoraSolicitud(int signal);
void manejadoraFinalizar(int signal);
void writeLogMessage (char *id , char *msg);
void inicializarSolicitud(Solicitud* solicitud);
void inicializarActividad(Actividad* actividad);
//Cola de solicitudes
Solicitud colaSolicitudes[tamCola];
Actividad colaActividadSocial[4];
//Para la asignacion de ID a las solicitudes
int contadorSolicitudes;
//Para saber cuantos hay en cola
int contadorSolicitudesCola;
int contadorActividadesCola;
//Para saber 
int listaCerrada;
//Mutex
pthread_mutex_t mutexLog;
pthread_mutex_t mutexColaSolicitudes;
pthread_mutex_t mutexColaSocial;
//Archivo para los logs
FILE *logFile;
/*
 *	Main
 */

int main(){
	//Tipo de atendedor 1->Invitacion 2->QR 3->PRO
	int tipoAtendedor[3]={1,2,3};
	int i;
	//Tratamiento de señales
	struct sigaction solicitud={0};
	struct sigaction finalizar={0};
//	sSolicitud.sa_handler=manejadoraSolicitud;
	sigaction(SIGUSR1,&solicitud,NULL);
	sigaction(SIGUSR2,&solicitud,NULL);
//	sFinalizar.sa_handler=manejadoraFinalizar;
	sigaction(SIGINT,&finalizar,NULL);
	//Inicializar recursos	
	srand(time(NULL));
	contadorSolicitudes=0;
	contadorSolicitudesCola=0;
	contadorActividadesCola=0;
	listaCerrada=false;
	logFile=NULL;
	//Prueba de logs
	/*
	char buffer[500];
	sprintf(buffer,"Cola de solicitudes llena, Solicitud ignorada\n");
	writeLogMessage ( "id" , buffer);
	*/
	for(i=0;i<tamCola;i++){
		inicializarSolicitud(&colaSolicitudes[i]);
	}
	for(i=0;i<4;i++){
		inicializarActividad(&colaActividadSocial[i]);
	}

	//Inicializamos los mutex
	pthread_mutex_init(&mutexColaSocial,NULL);
	pthread_mutex_init(&mutexLog, NULL);
	pthread_mutex_init(&mutexColaSolicitudes, NULL);
	//Encargados de las solicitudes:
	pthread_t atendedor_1, atendedor_2, atendedor_3, coordinador;
	//Creamos los hilos de los atendedores
	pthread_create(&atendedor_1, NULL, accionesAtendedor, (void *) &tipoAtendedor[0]);
	pthread_create(&atendedor_2, NULL, accionesAtendedor, (void *) &tipoAtendedor[1]);
	pthread_create(&atendedor_3, NULL, accionesAtendedor, (void *) &tipoAtendedor[2]);
	//Creamos el hilo del coordinador
	pthread_create(&coordinador, NULL, accionesCoordinadorSocial, NULL);
	sleep(1);
}

/*
 *	Funciones
 */
void *accionesAtendedor(void *ptr){
	char cadena[100];

	
}

void *accionesCoordinadorSocial(void *ptr){


}

void nuevaSolicitud(int signal){
	printf("Nueva solicitud\n");
//Variable para imprimir los mensajes /*sprintf(buffer, "mensaje");*/ /*writeLogMessage (char *id , char *msg)*/
	char buffer[500];
//Seccion Critica
	pthread_mutex_lock(&mutexColaSolicitudes);
	//Declaro la solicitud
	Solicitud solicitud;
	//Contador del bucle
	int i = 0, completo = 0;
	//Variable para el numero de solicitud
	char valorId = ' ';
	//Compruebo el espacio en la lista de solicitudes
	for(i = 0; i < tamCola; i++){
		//Si el hueco de la solicitud esta libre, entro
		if(&colaSolicitudes[tamCola] == NULL){
			//Introduzco el identificador con su valor y lo incremento
			contadorSolicitudes++;
			//Guardo el valor en un char para introducirlo en id
			valorId = contadorSolicitudes;
			//Introduzco el numero en el identificador de la solicitud
			sprintf(solicitud.id,"solicitud_%d",valorId);
			printf("%s añadida a la lista de solicitudes\n",solicitud.id);
			sprintf(buffer, "%s añadida a la lista de solicitudes\n",solicitud.id);
			printf("%s lista para ser atendida\n",solicitud.id);
			sprintf(buffer, "%s lista para ser atendida\n",solicitud.id);
			solicitud.atendido = 0;
			//Indico el tipo de solicitud segun la senial
			//Invitacion
			if(signal == SIGUSR1){
				printf("%s de invitacion\n",solicitud.id);
				sprintf(buffer, "%s de invitacion\n",solicitud.id);
				solicitud.tipo = 0;
			//QR
			}else{
				printf("%s por codigo QR\n",solicitud.id);
				sprintf(buffer,"%s por codigo QR\n",solicitud.id);
				solicitud.tipo = 1;
			}
			//Crear el hilo
			//Mando la solicitud para ser procesada
			pthread_create(&solicitud.hilo, NULL, accionesSolicitud, (void *) &solicitud);
		//Cola llena
		}else{
			if(completo == tamCola){
				printf("Cola de solicitudes llena, Solicitud ignorada\n");
				sprintf(buffer,"Cola de solicitudes llena, Solicitud ignorada\n");
			}
			completo++;
		}	
	}
//Envio el mensaje guardado en el buffer a la funcion writeLogMessage
	writeLogMessage ( solicitud.id , buffer);
//Fin de la seccion critica
	pthread_mutex_unlock(&mutexColaSolicitudes);
}
//Escribimos en el log
void writeLogMessage (char *id , char *msg) {
	pthread_mutex_lock(&mutexLog);
	if(logFile==NULL){
		logFile = fopen ("log.txt", "w") ;
	} else {
		logFile = fopen ("log.txt", "a") ;
	}
	// Calculamos la hora actual
	time_t now = time (0) ;
	struct tm *tlocal = localtime (&now) ;
	char stnow [19];
	//No poner %M:%S, empieza a poner caracteres extraños, ni separarlo
	strftime (stnow, 19, "%d/ %m/ %y %H:%M%S" , tlocal) ;
	// Escribimos en el log. La primera vez de cada ejecución, borrará el log.txt en caso de que exista.
	fprintf (logFile , "[%s] %s : %s\n" , stnow , id , msg) ;
	fclose (logFile);
	pthread_mutex_unlock(&mutexLog); 
}

//Codigo reutilizable para las funciones
/*
//Funcion del hilo encargado de las invitaciones
void *despachoInvitador(void *ptr) {
	printf("Soy el encargado de las invitaciones\n");
}

//Funcion del hilo encargado de los codigos QR
void *despachoQR(void *ptr) {
	//Números aleatorios
	srand(time(NULL));
	int num;
	//Genero disparo aleatorio
	num = (0+(rand()%(100)-0)+1);
	//Descarto el 40% de las invitaciones
	if(num <=40){
		//Poco fiable
		if(num <=30){
			printf("--Solicitud descartada por falta de fiabilidad--\n");	
		//Se canso de esperar
		}else{
			printf("--El solicitante se ha cansado de esperar--\n");	
		}
	}
	//Descarto un 15% (aplicacion mal hecha)
	num = (0+(rand()%(100)-0)+1);
	if(num <= 15){
		printf("--La solicitud ha sido rechazada con exito--\n");
	}
	printf("Soy el encargado de los codigos QR\n");
}

//Funcion del AtendedorPRO
void *despachoAtendedorPRO(void *ptr) {
	//Números aleatorios
	srand(time(NULL));
	int num;
	//Genero el numero aleatorio
	num = (0+(rand()%(100)-0)+1);
	printf("Soy el atendedorPRO\n");
}
*/
/*
 * @author Marcos Ferreras
 */

/**
 * Función que se encarga de las acciones de la solicitud
 * @autor Marcos Ferreras
*/
void *accionesSolicitud(void *structSolicitud){
	//char idSolicitud[50], cadenaLog[100];
	Solicitud* solicitud = (Solicitud *)structSolicitud;
	//0->Sin atender 1->En proceso de atención 2->Atendido
	int enAtencion=0;
	int aleatorio;
	int participo;
	char id[50];
	
	//strcpy(solicitud->id,"1");
	//if(strcmp("0\0",solicitud->id)==0)	
	//	printf("Hola\n");
	
	//writeLogMessage("1","La solicitud se ha recibido");
	//printf("Hola\n");
	//writeLogMessage("1111","5");
	//TODO Escribir en log
	printf("ID %s : La solicitud de tipo %d ha sido registrada\n", solicitud->id, solicitud->tipo);
	printf("ID %s : La solicitud de tipo %d esta esperando (4 segundos)\n", solicitud->id, solicitud->tipo);
	sleep(4);
	pthread_mutex_lock(&mutexColaSolicitudes);
	//Copio el id
	strcpy(id,solicitud->id);
	enAtencion=solicitud->atendido;
	//Mientras esté sin atender
	while(enAtencion==0){
		printf("%d\n",enAtencion);
		//Invitación
		aleatorio = rand()%101;
		if(solicitud->tipo==0){
			printf("%d rand\n",aleatorio);
			if(aleatorio<=10){
				//TODO Escribir en log
				printf("ID %s : La solicitud de tipo %d se ha eliminado por cansarse de esperar\n", solicitud->id, solicitud->tipo);
				inicializarSolicitud(solicitud);
				pthread_exit(NULL); 
			}
		//QR
		} else {
			if(aleatorio<=30){
				//TODO Escribir en log
				printf("ID %s : La solicitud de tipo %d se ha eliminado por no considerarse muy fiable\n", solicitud->id, solicitud->tipo);
				inicializarSolicitud(solicitud);
				pthread_exit(NULL); 
			}
		}
		//Descarte fallo aplicacion
		aleatorio = rand()%101;
		if(aleatorio<=15){
			//TODO Escribir en log
			printf("ID %s : La solicitud de tipo %d se ha eliminado por fallo de la aplicacion\n", solicitud->id, solicitud->tipo);	
			//TODO Pendiente de crear una función que me saque de la cola y reordene el array de la cola, para evitar huecos
			inicializarSolicitud(solicitud);
			pthread_exit(NULL);
		}
		pthread_mutex_unlock(&mutexColaSolicitudes);
		sleep(4);
		pthread_mutex_lock(&mutexColaSolicitudes);
		enAtencion=solicitud->atendido;
		
	}
	//enAtencion=solicitud->atendido;
	//Mientras me esten atendiendo esperaré. No he soltado el mutex cuando salí del bucle anterior.
	while(enAtencion==2){
		pthread_mutex_unlock(&mutexColaSolicitudes);
		sleep(2);
		pthread_mutex_lock(&mutexColaSolicitudes);
		enAtencion=solicitud->atendido;
	}
	//Al salir de este bucle no he soltado el mutex. En este punto lo desbloqueo, y decido si me uno a una actividad social o no (Si puedo).
	enAtencion=solicitud->atendido;
	pthread_mutex_unlock(&mutexColaSolicitudes);
	if(enAtencion == 3){
		participo=rand()%2;
		//0->Si Participo 1->No Participo
		if(participo==0){
			//Libero hueco en solicitudes
			inicializarSolicitud(solicitud);
			pthread_mutex_lock(&mutexColaSocial);
			while(listaCerrada==true){
				pthread_mutex_unlock(&mutexColaSocial);
				sleep(3);
				pthread_mutex_lock(&mutexColaSocial);
			}
			//Copio el id del usuario a la actividad
			strcpy(colaActividadSocial[contadorActividadesCola].idUsuario, id);
			//Incremento el numero de actividades
			contadorActividadesCola++;
			
		} else {
			//Libero espacio en cola de solicitudes
			//TODO Pendiente de crear una función que me saque de la cola y reordene el array de la cola, para evitar huecos
			inicializarSolicitud(solicitud);
		}
		pthread_exit(NULL);
	}
	//Solicitud con antecedentes. Libero el hueco en la cola y finalizo hilo.
	//TODO Pendiente de crear una función que me saque de la cola y reordene el array de la cola, para evitar huecos
	inicializarSolicitud(solicitud);
	pthread_exit(NULL);
}

/**
 *@author Marcos Ferreras
 *Inicializa los campos de la estructura solicitud a 0.  
*/
void inicializarSolicitud(Solicitud* solicitud){
	pthread_mutex_lock(&mutexColaSolicitudes);
	strcpy(solicitud->id, "0");
	solicitud->atendido=0;
	solicitud->tipo=0;
	solicitud->hilo=0;
	pthread_mutex_unlock(&mutexColaSolicitudes);
}
/**
 *@author Marcos Ferreras
*/
void inicializarActividad(Actividad* actividad){
	pthread_mutex_lock(&mutexColaSocial);
	strcpy(actividad->idUsuario, "0");
}
