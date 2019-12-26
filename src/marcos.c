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
//Defino las funciones
void *accionesAtendedor(void *ptr);
void *accionesSolicitud(void *ptr);
void manejadoraSolicitud(int signal);
void manejadoraFinalizar(int signal);
void writeLogMessage (char *id , char *msg);
int sacarNumero(char *id);
void *usuarioEnActividad(void *id);
pthread_mutex_t mutexLog;
pthread_mutex_t mutexColaSolicitudes;
pthread_mutex_t mutexColaSocial;
pthread_cond_t condActividades;
FILE *logFile;
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
Solicitud colaSolicitudes[tamCola];
int idUsuariosActividad[4];
//Para la asignacion de ID a las solicitudes
int contadorSolicitudes;
//Para saber cuantos hay en cola
int contadorSolicitudesCola;
int contadorActividadesCola;
//Para saber 
int listaCerrada;

/*
 *	Main
 */

int main(){
	//Tipo de atendedor 1->Invitacion 2->QR 3->PRO
	int tipoAtendedor[3]={1,2,3};
	int i;
	//Tratamiento de señales
	struct sigaction sSolicitud={0};
	struct sigaction sFinalizar={0};
	sSolicitud.sa_handler=manejadoraSolicitud;
	sigaction(SIGUSR1,&sSolicitud,NULL);
	sigaction(SIGUSR2,&sSolicitud,NULL);
	sFinalizar.sa_handler=manejadoraFinalizar;
	sigaction(SIGINT,&sFinalizar,NULL);
	//Inicializar recursos	
	srand(time(NULL));
	contadorSolicitudes=0;
	contadorSolicitudesCola=0;
	contadorActividadesCola=0;
	listaCerrada=false;
	logFile=NULL;
	pthread_cond_init(&condActividades, NULL);
	for(i=0;i<tamCola;i++){
		inicializarSolicitud(&colaSolicitudes[i]);
	}
	for(i=0;i<4;i++){
		idUsuariosActividad[i]=0;
	}
	pthread_mutex_init(&mutexColaSocial,NULL);
	pthread_mutex_init(&mutexLog, NULL);
	pthread_mutex_init(&mutexColaSolicitudes, NULL);
	//Encargados de las solicitudes de:
	pthread_t atendedor_1, atendedor_2, atendedor_3;
	//Creamos los hilos de los "usuarios destacados"
	printf("Iniciando tsunami\n");
	writeLogMessage("1","Iniciando tsunami");
	pthread_create(&atendedor_1, NULL, accionesAtendedor, (void *) &tipoAtendedor[0]);
	pthread_create(&atendedor_2, NULL, accionesAtendedor, (void *) &tipoAtendedor[1]);
	pthread_create(&atendedor_3, NULL, accionesAtendedor, (void *) &tipoAtendedor[2]);
	//PRUEBA pthread_create(&atendedor_3, NULL, accionesSolicitud, (void *) &colaSolicitudes[0]);
	//PRUEBA sleep(5);
	//PRUEBA strcpy(colaSolicitudes[1].id, "1");
	//PRUEBA pthread_create(&atendedor_3, NULL, accionesSolicitud, (void *) &colaSolicitudes[1]);
	//Espera por señales de forma indefinda. No es espera activa!!!
	while(true){
		pause();
	}
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
	//printf("gfdgxf\n");
	//writeLogMessage("sdfsdfsdfsdfsd","fdfsdfsdfsdf");

	
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
		printf("%d\n",enAtencion);
		//Invitación
		aleatorio = rand()%101;
		if(solicitud->tipo==0){
			printf("%d rand\n",aleatorio);
			if(aleatorio<=10){
				//TODO Escribir en log
				printf("ID %s : La solicitud por invitacion se ha eliminado por cansarse de esperar\n", solicitud->id);
				writeLogMessage(solicitud->id,"La solicitud por invitacion se ha eliminado por cansarse de esperar");
				inicializarSolicitud(solicitud);
				pthread_exit(NULL); 
			}
		//QR
		} else {
			if(aleatorio<=30){
				//TODO Escribir en log
				printf("ID %s : La solicitud por QR se ha eliminado por no considerarse muy fiable\n", solicitud->id);
				writeLogMessage(solicitud->id,"La solicitud por QR se ha eliminado por no considerarse muy fiable");
				inicializarSolicitud(solicitud);
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
			inicializarSolicitud(solicitud);
			printf("ID %s : El usuario esta preparado para la actividad\n", solicitud->id);
			writeLogMessage(solicitud->id,"El usuario esta preparado para la actividad");
			pthread_mutex_lock(&mutexColaSocial);
			while(listaCerrada==true){
				pthread_mutex_unlock(&mutexColaSocial);
				sleep(3);
				pthread_mutex_lock(&mutexColaSocial);
			}
			//Copio el id del usuario a la actividad
			idUsuariosActividad[contadorActividadesCola]= sacarNumero(id);
			//Incremento el numero de actividades
			contadorActividadesCola++;
			if(contadorActividadesCola==4){
				//Aviso al coordinador que soy el ultimo
				pthread_cond_signal(&condActividades);
			}
			pthread_mutex_unlock(&mutexColaSocial);
			//Libero espacio en la cola
			pthread_mutex_lock(&mutexColaSolicitudes);
			inicializarSolicitud(solicitud);
			pthread_mutex_unlock(&mutexColaSolicitudes);
			pthread_exit(NULL);
			
		} else {
			//Libero espacio en cola de solicitudes
			inicializarSolicitud(solicitud);
		}
		pthread_exit(NULL);
	} else if(enAtencion==3){
		//Solicitud con antecedentes. Libero el hueco en la cola y finalizo hilo.
		inicializarSolicitud(solicitud);
		pthread_exit(NULL);
	}
	
	
	
}
void *usuarioEnActividad(void *id){
	char idTemp[50];
	sprintf(id,"%d",*(int *)id);
	sleep(3);
	pthread_mutex_lock(&mutexColaSolicitudes);
	contadorActividadesCola--;
	if(contadorActividadesCola==0){
		pthread_cond_signal(&condActividades);
	}
	pthread_mutex_unlock(&mutexColaSolicitudes);
	//TODO Escribir en el log
	printf("ID %s : El usuario ha finalizado la actividad\n", solicitud->id);
	writeLogMessage(idTemp,"El usuario ha finalizado la actividad");
	pthread_exit(NULL);
}

//Escribimos en el log
void writeLogMessage (char *id , char *msg) {
	int i;
	pthread_mutex_lock(&mutexLog);
	if(logFile==NULL){
		logFile = fopen ("log.txt", "w") ;
	} else {
		logFile = fopen ("log.txt", "a") ;
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
	if(signal=="SIGUSR1"){
		
}
void manejadoraFinalizar(int signal){
	//Temporal
	exit(0);
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
int sacarNumero(char *id){
	int numero=0;
	int posicion_=0, contador=0;
	for (int i=0;i<strlen(id);i++){
		if(id[i]>='0' && id[i]<='9'){
			numero=numero*10 + id[i] - '0';
		}
	}
	return numero;
}

