#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
//Tamanio de la cola de solicitudes
#define tamCola 15
//Cola de solicitudes
int cola[tamCola];
//1->Invitacion 2->QR 3->PRO
int tipoAtendedor[3]={1,2,3};
//Defino las funciones
void *accionesAtendedor(void *ptr);
void manejadoraSolicitud(int signal);
void manejadoraFinalizar(int signal);
void writeLogMessage (char *id , char *msg);
pthread_mutex_t mutexLog;
FILE *logFile;
//Para la asignacion de ID a las solicitudes
int contadorSolicitudes;

/*
 *	Main
 */

int main(){
	struct sigaction sSolicitud={0};
	struct sigaction sFinalizar={0};
	sSolicitud.sa_handler=manejadoraSolicitud;
	sigaction(SIGUSR1,&sSolicitud,NULL);
	sigaction(SIGUSR2,&sSolicitud,NULL);
	sFinalizar.sa_handler=manejadoraFinalizar;
	sigaction(SIGINT,&sFinalizar,NULL);
	//Inicializar recursos
	contadorSolicitudes=0;
	pthread_mutex_init(&mutexLog, NULL);
	writeLogMessage("1111","1");
	writeLogMessage("1111","2");
	writeLogMessage("1111","3");
	//Encargados de las solicitudes de:
	pthread_t atendedor_1, atendedor_2, atendedor_3;
	//Creamos los hilos de los "usuarios destacados"
	pthread_create(&atendedor_1, NULL, accionesAtendedor, (void *) &tipoAtendedor[0]);
	pthread_create(&atendedor_2, NULL, accionesAtendedor, (void *) &tipoAtendedor[1]);
	pthread_create(&atendedor_3, NULL, accionesAtendedor, (void *) &tipoAtendedor[2]);
	
	sleep(1);
}

/*
 *	Funciones
 */

/*//Funcion del hilo encargado de las invitaciones
void *despachoInvitador(void *ptr) {
	int i = 0;
	printf("Soy el encargado de las invitaciones\n");
	writeLogMessage("1111",Hola%d);
	writeLogMessage("1111","5");
	writeLogMessage("1111","6");
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
}*/
void *accionesAtendedor(void *ptr){
	char cadena[100];
	sprintf(cadena,"%d",*(int *)ptr);
	writeLogMessage("hola",cadena);
	sprintf(cadena,"%d",5666);
	writeLogMessage("hola",cadena);
	
}
//Escribimos en el log
void writeLogMessage (char *id , char *msg) {
	// Calculamos la hora actual
	pthread_mutex_lock(&mutexLog); 
	time_t now = time (0) ;
	struct tm *tlocal = localtime (&now) ;
	char stnow [19];
	strftime (stnow, 19, " %d/ %m/ %y %H: %M: %S " , tlocal) ;
	// Escribimos en el log. La primera vez de cada ejecución, borrará el log.txt en caso de que exista.
	if(logFile==NULL){
		logFile = fopen ("log.txt", "w") ;
	} else {
		logFile = fopen ("log.txt", "a") ;
	}
	fprintf (logFile , "[%s] %s : %s\n" , stnow , id , msg) ;
	fclose (logFile);
	pthread_mutex_unlock(&mutexLog); 

}
void manejadoraSolicitud(int signal){
}
void manejadoraFinalizar(int signal){
}


