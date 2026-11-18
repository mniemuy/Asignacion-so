#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <string.h>


typedef struct {
    int id;   //así podemos saber en qué archivos estamos y qué y cuantos bloques ocupamoss
    int size;
    int *bloques;
    int nBloques;
} Archivo;

//para guardar y luego usar los archivos los voy a guardar en un arreglo dinámico
//osea poder usar realloc cuando sea necesario
//vi que también se podían representar como lista enlazada, pero alargaba mucho la tarea

typedef struct { //para representar el disco
int ocupado; //bloque libre o ocupado? como un bitmap
int fileid;   // para saber qué archivo corresponde a cada bloque
} Bloque;

int main(){

int tdisco = 0; //tamaño del disco
int tbloque = 0; //tamaño de cada bloque
int metodo = 0; //para el método de asignación
int tarchivo = 0; //tamaño de archivo
int bloquesR = 0; 
int archivoMin = 0;  //max y min archivo
int archivoMax = 0;
int capacidadArchivos = 10;// cuantos se pueden registrar al inicio
int cantidadArchivos = 0; //cuantos están registrados actualmente 




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
//acá podemos ocupar arreglos o malloc

//según leí si el usuario se pasa con el tamaño del disco a varios GB puede provocar un stack overflow
//el heap normalmente puede crecer más, entonces podemos usar memoria dinámica en vez de un arreglo
printf("Cantidad de bloques%d\n", bloques);

Bloque *disco = malloc(bloques * sizeof(Bloque));

Archivo *archivos = malloc(capacidadArchivos * sizeof(Archivo));

if (disco == NULL || archivos == NULL) {
    printf("Error de reserva de memoria\n");
    free(disco);
    free(archivos);
    return 1;
}

for (int i = 0; i < bloques; i++) {
    disco[i].ocupado = 0;
    disco[i].fileid = -1;  //inicialización de campos como ninguno ocupado y todos los fileid en -1
}
///////////////////7ARCHIVOFALSO///////////////////////////////////////////////////
archivos[0].id = 1;
archivos[0].size = 12;          // tamaño en KB
archivos[0].bloques = NULL;     // aún no tiene bloques asignados
archivos[0].nBloques = 0;

cantidadArchivos++;


printf("Archivo %d: %d KB\n",archivos[0].id, archivos[0].size);


////////////////////////////////////////////////////////////////////////////////después

printf("Elige un método de asignación\n");
printf("1. Contigua\n"); //antes era escribir ahora opciones
printf("2. Enlazada\n");
printf("3. Indexada\n");
scanf("%d", &metodo);


if(metodo == 1){
    tarchivo = archivos[0].size;
    bloquesR = ceil((double)tarchivo/tbloque); //el double porque si no trunca antes de hacerle ceil 10/4 = 2 -> ceil(2) = 2
//revisar las siguientes bloquesr posiciones

//17nov ahora puedo continuar con esto, usaré best fit
//si voy recorriendo el disco buscando cadenas de "bits" libres entonces puedo encontrar la cadena 
//que le da el mejor fit, con un recorrido debería bastar
    int inicioActual = -1;
    int longitudActual = 0;
    int mejorInicio = -1;
    int mejorSize = bloques + 1;

    for (int i = 0; i < bloques; i++) {

        if (disco[i].ocupado == 0) { //si está libre
            if (longitudActual == 0) {  
                inicioActual = i;  //y la longitud es 0, es una nueva cadena, hace que sea la posicion actual
            }
            longitudActual++; //le suma el lugar libre al contador
        }
        else { // si no está libre
            if (longitudActual >= bloquesR && longitudActual < mejorSize) { 
                // si la longitud actual es igual o mayor a los bloques que se necesitan reservar
                //y la longitud actual es menor que el mejor tamaño
                mejorSize = longitudActual; //nos quedamos con el actual
                mejorInicio = inicioActual;
            }

            longitudActual = 0; //encontró un bloque ocupado, evalúa el espacio y reinicia el counter
        }
    }

    //comprobar el último espacio si termina con espacios libres,  ya que nunca encontrará
    //otro 1
    if (longitudActual >= bloquesR && longitudActual < mejorSize) {
        mejorSize = longitudActual;
        mejorInicio = inicioActual;
    }

    //resultado
    if (mejorInicio == -1) {
        printf("No hay espacio para el archivo\n");
    }
    else {
        printf("Best espacio comienza en: %d\n", mejorInicio);
        printf("Tamaño del espacio: %d bloques\n", mejorSize);

//mejorinicio + bloquesR da el final de la cadena + 1 entonces menor que

    for (int i = mejorInicio; i < mejorInicio + bloquesR; i++) { 
        disco[i].ocupado = 1;
        disco[i].fileid = archivos[0].id;
        //para todos los espacios requeridos los llena con 1nos y llena el fileid de los bloques
        }
    }






}else if(metodo == 2){

    //a este hay que tener en cuenta que el puntero ocupa 4 bytes entonces es lo mismo de arriba pero al tamaño de bloque le restas los 4 bytes del puntero
}else if(metodo == 3){
    bloquesR = ceil((double)tarchivo/tbloque) + 1;
}else{
    
}


free(archivos);
free(disco);
    return 0;
}

