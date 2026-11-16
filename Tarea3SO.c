#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <string.h>


int main(){
int tdisco = 0;
int tbloque = 0;
int metodo = 0;
int tarchivo = 0;
int bloquesR = 0; //bloques reservados
int archivoMin = 0;
int archivoMax = 0;

printf("Mínimo del archivo?\n");
scanf("%d", &archivoMin);

printf("Máximo del archivo?\n");
scanf("%d", &archivoMax);

if(archivoMin <= 0 || archivoMax < archivoMin){
    printf("Error de rango\n");
    return 1;
}

printf("Tamaño de disco en MB:\n");
scanf(" %d",&tdisco);
printf("Tamaño de bloques en KB:\n");
scanf("%d",&tbloque);

//para sacar la cantidad de bloques voy a pasarlo todo a KB, multiplicar por 1024 es mejor que dividir por 1024, ahorras decimales
if(tdisco <= 0 || tbloque <= 0 || tdisco*1024 < tbloque){
    return -1;
}

int bloques = (tdisco*1024)/tbloque;

printf("Cantidad de bloques%d\n", bloques);


printf("Elige un método de asignación\n");
//prefiero dejar el método de asignación como opciones
printf("1. Contigua\n");
printf("2. Enlazada\n");
printf("3. Indexada\n");
scanf("%d", &metodo);

if(metodo == 1){
bloquesR = ceil((double)tarchivo/tbloque); //el double porque si no trunca antes de hacerle ceil 10/4 = 2 -> ceil(2) = 2
//revisar las siguientes bloquesr posiciones


}else if(metodo == 2){

    //a este hay que tener en cuenta que el puntero ocupa 4 bytes entonces es lo mismo de arriba pero al tamaño de bloque le restas los 4 bytes del puntero
}else if(metodo == 3){
    bloquesR = ceil((double)tarchivo/tbloque) + 1;
}else{
    
}


//printf("Has elegido para el tamaño del disco: %d\n", tdisco);
//printf("Para cada bloque: %d\n", tbloque);
//printf("Y el método es: %s\n", metodo);

//printf("Hola");
    return 0;
}

