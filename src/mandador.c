#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
void ejecutar(int senial1,int senial2,int alternar,int pid, int espera);
void ejecutarAvanzado(int senial1,int senial2,int alternar,int espera,int pid,int repetir,int patron1,int patron2);
void menu(int pid);
int main(int argc, char *argv[]){

//Función principal.
	if(argv[1] == NULL){
		//Error
		printf("El número de argumentos no coincide con los solicitados\n");
		printf("Introduzca:\n");
		printf("./mandador pid\n");
	}else{
		//Defino las variables que voy a utilizar
		//Asigno el pid
		int pid = atoi(argv[1]);
		menu(pid);
	}
	return 0;
}

void menu(int pid){
int senial1 = 0,senial2 = 0,alternar = 0, defecto = 0,espera = 0, avanzado = 0,repetir = 0, patron1 = 0,patron2 = 0;
	//Configuracion basica
	printf("Prefiere probar con los valores predefinidos?\n");
	printf("1- Si\n2- No\n");
	scanf("%d",&alternar);
	//Configurar
	if(defecto == 2){
		printf("Indique el numero de señales SIGUSR1\n");
		scanf("%d",&senial1);
		printf("Indique el numero de señales SIGUSR2\n");
		scanf("%d",&senial2);
		printf("Desea altenarlas?\n");
		printf("1- Si\n2- No\n");
		scanf("%d",&alternar);
		printf("Introduzca el numero de segundos de espera entre cada señal\n");
		scanf("%d",&espera);
		printf("Configuracion avanzada\n");
		printf("1- Si\n2- No\n");
		scanf("%d",&avanzado);
		if(avanzado == 0){
			printf("Ejecutando...\n");
			ejecutar(senial1,senial2,alternar,pid,espera);
		}else{
			printf("Configuracion avanzada...\n");
			printf("Desea repetir señales?\n");
			printf("1- Si\n2- No\n");
			scanf("%d",&repetir);
			if(repetir == 1){
				printf("Introduzca el patron de repeticion o introduzca 0 0  para patron aleatorio\n");
				printf("Ejemplo:\n4\n1\n4 SIGUSR1,1 SIGUSR2");
				scanf("%d",&patron1);
				scanf("%d",&patron2);
				if(patron1 == 0 && patron2 == 0){
					srand(time(NULL));
					patron1 = rand() % (senial1-1+1) + 1;
					patron2 = rand() % (senial2-1+1) + 1;
					printf("Patron aleatorio %d %d\n",patron1,patron2);
				}
			}
			ejecutarAvanzado(senial1,senial2,alternar,espera,pid,repetir,patron1,patron2);
		}
	//Por defecto
	}else{
		printf("Valores por defecto,30,30,1\n");
		ejecutar(30,30,1,pid,0);
	}
}

void ejecutar(int senial1,int senial2,int alternar,int pid, int espera){
	int i = 0;
	int max = senial1+senial2;
	char cmd1[100];
	char cmd2[100];
	sprintf(cmd1,"kill -10 %d",pid);
	sprintf(cmd2,"kill -12 %d",pid);
	int valor1 = 0,valor2 = 0;
	//No Alterna señales
	if(alternar == 1){
		for(i = 0;i < max;i++){
			if(i < senial1){
				valor1 = system(cmd1);
			}
			if(i < senial2){
				valor2 = system(cmd2);
			}
			sleep(espera);
		}
	//Alterna señales
	}else{
		for(i = 0;i < max;i++){
			if(i % 2 == 0){
				valor1 = system(cmd1);
			}else{
				valor2 = system(cmd2);
			}
			sleep(espera);
		}
	}
	//Fin del programa
	sprintf(cmd1,"kill -4 %d",pid);
	valor1 = system(cmd1);
}

void ejecutarAvanzado(int senial1,int senial2,int alternar,int espera,int pid,int repetido,int patron1,int patron2){
int i = 0;
	int max = senial1+senial2;
	char cmd1[100];
	char cmd2[100];
	sprintf(cmd1,"kill -10 %d",pid);
	sprintf(cmd2,"kill -12 %d",pid);
	int valor1 = 0,valor2 = 0;
	int repetido1 = 0, repetido2 = 0;
	//No Alterna señales
	if(alternar == 1){
		if(repetido == 1){
			for(i = 0;i < max;i++){
				if(i < senial1 && repetido1 < patron1){
					valor1 = system(cmd1);
					repetido1++;
				}
				if(i < senial2 && repetido2 < patron2){
					valor2 = system(cmd2);
					repetido2++;
				}
				sleep(espera);
			}
		}else{
			for(i = 0;i < max;i++){
				if(i < senial1){
					valor1 = system(cmd1);
				}
				if(i < senial2){
					valor2 = system(cmd2);
				}
				sleep(espera);
			}
		}
		
	//Alterna señales
	}else{
		for(i = 0;i < max;i++){
			if(i % 2 == 0){
				valor1 = system(cmd1);
			}else{
				valor2 = system(cmd2);
			}
			sleep(espera);
		}
	}
	//Fin del programa
	sprintf(cmd1,"kill -4 %d",pid);
	valor1 = system(cmd1);
}
