//Resumen
/*
Dudas:
*/
/*
Trabajando en:
	Escribir los logs
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <time.h>
//Tamanio de la cola de solicitudes
#define tamCola 15
//Cola de solicitudes
int cola[tamCola];
//Vector con el identificador de los "usuarios destacados"
int id[3]={1,2,3};
//Defino las funciones
void *despachoInvitador(void *ptr);
void *despachoQR(void *ptr);
void *despachoAtendedorPRO(void *ptr);

/*
 *	Main
 */

int main(){
printf("tamCola: %d\n",tamCola);
	//Encargados de las solicitudes de:
	pthread_t atendedor_1, atendedor_2, atendedor_3;
	//Enmascaro las señales
	
	//Archivo
	FILE *pa;
	//Creamos los hilos de los "usuarios destacados"
	pthread_create(&atendedor_1, NULL, despachoInvitador, (void *) &id[0]);
	pthread_create(&atendedor_2, NULL, despachoQR, (void *) &id[1]);
	pthread_create(&atendedor_3, NULL, despachoAtendedorPRO, (void *) &id[2]);
	sleep(1);
}

/*
 *	Funciones
 */

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
/*
//Escribimos en el log
void writeLogMessage (char *id , char *msg, FILE *pa) {
	// Calculamos la hora actual
	time_t now = time (0) ;
	struct tm *tlocal = localtime (&now) ;
	char stnow [19];
	strftime (stnow, 19, " %d/ %m/ %y %H: %M: %S " , tlocal) ;

	// Escribimos en el log
	logFile = fopen (pa , "a") ;
	fprintf (pa , "[%s] %s : %s\n" , stnow , id , msg) ;
	fclose (pa) ;

}
*/

