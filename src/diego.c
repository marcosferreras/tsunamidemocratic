//Resumen
/*
Dudas:
*/
/*
Trabajando en:
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
//Tamanio de la cola de solicitudes
#define tamCola 15
//Cola de solicitudes
int cola[tamCola];
//Vector con el identificador de los "usuarios destacados"
int i[3]={1,2,3};
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
	pthread_t invitador, QR, atendedorPRO;
	//Creamos los hilos de los "usuarios destacados"
	pthread_create(&invitador, NULL, despachoInvitador, (void *) &i[0]);
	pthread_create(&QR, NULL, despachoQR, (void *) &i[1]);
	pthread_create(&atendedorPRO, NULL, despachoAtendedorPRO, (void *) &i[2]);
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
	printf("Soy el encargado de los codigos QR\n");
}

//Funcion del AtendedorPRO
void *despachoAtendedorPRO(void *ptr) {
	printf("Soy el atendedorPRO\n");
}


