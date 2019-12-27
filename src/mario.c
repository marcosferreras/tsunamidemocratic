#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

#define tamCola 15
#define true 1
#define false 0

typedef struct solicitud {
	char id[50];
	int atendido;
	int tipo;
	pthread_t hilo;
} Solicitud;
Solicitud colaSolicitudes[tamCola];
FILE *logFile;

pthread_mutex_t mutexLog;
pthread_mutex_t mutexColaSolicitudes;
pthread_mutex_t mutexColaSocial;

int contadorSolicitudes;
int contadorSolicitudesCola;
int contadorActividadesCola;
int listaCerrada;

void *accionesAtendedor(void * a);
void inicializarSolicitud(Solicitud* solicitud);
int sacarNumero(char *id);
void writeLogMessage (char *id , char *msg);

int main(){
	int tipoAtendedor[3]={1,2,3};
	srand(time(NULL));
	for(int i=0;i<tamCola;i++){
		sprintf(colaSolicitudes[i].id, "Solicitud_%d", rand()%50+1);
		colaSolicitudes[i].tipo=rand()%2+1;
	}
	pthread_t atendedor_1, atendedor_2, atendedor_3;
	pthread_create(&atendedor_1, NULL, accionesAtendedor, (void *) &tipoAtendedor[0]);
	//pthread_create(&atendedor_2, NULL, accionesAtendedor, (void *) &tipoAtendedor[1]);
	//pthread_create(&atendedor_3, NULL, accionesAtendedor, (void *) &tipoAtendedor[2])8;
	pause();
}


void *accionesAtendedor(void *ptrs){
	int tipoEvaluacion, id, contador=0, calculo, espera, apto, posicion=0, cafe=0;
	char atendedor[25], salida[150];
	//printf("Exito %d", *(int *)ptrs);
	sprintf(atendedor,"Atendedor_%d",*(int *)ptrs);
	do{
		contador=0;//😂️
		apto=true;
		cafe++;
		do{
			if(contador%2==0 || contador==0){
					pthread_mutex_lock(&mutexColaSolicitudes);
			}
					
			contador++;
			id=-1;
			for(int i=0;i<tamCola;i++){
				if((colaSolicitudes[i].tipo==tipoEvaluacion || tipoEvaluacion==3) && colaSolicitudes[i].atendido==0){
					if((id>sacarNumero(colaSolicitudes[i].id) || id==-1)){
						if(sacarNumero(colaSolicitudes[i].id)!=0){
							id=sacarNumero(colaSolicitudes[i].id);
							posicion=i;
						}				
					}
				}
			}
			if(id==-1){
				tipoEvaluacion=3;
				if(contador%2==0){
					//printf("%d-Par: %d\n",*(int *)ptrs,id);
					pthread_mutex_unlock(&mutexColaSolicitudes);
					sleep(1);
				}
			}
		}while(id==-1);
		colaSolicitudes[posicion].atendido=1;
		pthread_mutex_unlock(&mutexColaSolicitudes);
		printf("Atendedor_%d: comienza a procesar la solicitud_%d\n",*(int *)ptrs,id);
		sprintf(salida,"comienza a procesar la solicitud_%d",id);
		writeLogMessage(atendedor,salida);
		srand(time(NULL));
		calculo=rand()%10+1;
		if(calculo<=7){
			espera=rand()%4+1;
			printf("Atendedor_%d: todo correcto al procesar la solicitud_%d\n",*(int *)ptrs,id);
			sprintf(salida,"todo correcto al procesar la solicitud_%d",id);
			writeLogMessage(atendedor,salida);
		}else if(calculo<=9){
			espera=rand()%5+2;
			printf("Atendedor_%d: ha ocurrido un error inerperado al procesar los datos de la solicitud_%d\n",*(int *)ptrs,id);
			sprintf(salida,"ha ocurrido un error inerperado al procesar los datos de la solicitud_%d",id);
			writeLogMessage(atendedor,salida);
		}else{
			printf("Atendedor_%d: el usuario de solicitud_%d posee antecedentes\n",*(int *)ptrs,id);
			sprintf(salida,"el usuario de solicitud_%d posee antecedentes",id);
			writeLogMessage(atendedor,salida);
			espera=rand()%5+6;
			apto==false;
		}
		sleep(espera);
		printf("Atendedor_%d: el tiempo necesario para atender la solicitud_%d ha sido %d\n",*(int *)ptrs,id,espera);
		sprintf(salida,"el tiempo necesario para atender la solicitud_%d ha sido %d",id,espera);
		writeLogMessage(atendedor,salida);
		pthread_mutex_lock(&mutexColaSolicitudes);
			if(apto==false){
				colaSolicitudes[posicion].atendido=3;
			} else{
				colaSolicitudes[posicion].atendido=2;
			}
		pthread_mutex_unlock(&mutexColaSolicitudes);
		if(cafe%5==0){
			printf("Atendedor_%d: procede a tomarse un cafe☕️\n",*(int *)ptrs);
			sprintf(salida,"procede a tomarse un cafe☕️");
			writeLogMessage(atendedor,salida);
			sleep(10);
		}
	}while(true);
}

/**
 *@author Marcos Ferreras
 *Inicializa los campos de la estructura solicitud a 0.  
*/
void inicializarSolicitud(Solicitud* solicitud){
	pthread_mutex_lock(&mutexColaSolicitudes);
	sprintf(solicitud->id, "0");
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
	//sprintf(stnow,"");
	strftime (stnow, 23, " %d/ %m/ %y %H: %M: %S " , tlocal) ;
	// Escribimos en el log. La primera vez de cada ejecución, borrará el log.txt en caso de que exista.
	fprintf (logFile , "[%s] %s : %s\n" , stnow , id , msg) ;
	fclose (logFile);
	pthread_mutex_unlock(&mutexLog); 

}








































