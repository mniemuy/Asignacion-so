# Tarea 3 Sistemas Operativos

Simulador de asignación de archivos en disco con asignación contigua, enlazada e indexada.
Los archivos no se crean de verdad, solo se lleva la cuenta de qué bloques usa cada uno y dónde quedan.

## Compilar

Tiene que ser en linux o algo unix, con gcc:

```
gcc -Wall -Wextra Tarea3SO.c -o Tarea3SO -lm
```

El -lm va al final porque se usa ceil de math.h, si no se pone no compila.

## Ejecutar

```
./Tarea3SO
```

Va pidiendo los datos en este orden:

- mínimo del archivo en KB
- máximo del archivo en KB
- tamaño del disco en MB
- tamaño de los bloques en KB
- el método: 1 contigua, 2 enlazada o 3 indexada

Si no se quieren escribir uno por uno se pueden pasar de una. Por ejemplo archivos de 16 a 64 KB,
disco de 1 MB, bloques de 4 KB y enlazada:

```
printf '16\n64\n1\n4\n2\n' | ./Tarea3SO
```

Si el rango está malo, el bloque es más grande que el disco o el método no es 1, 2 o 3, avisa y se cierra.

## Cómo funciona

El disco se divide en bloques (disco / bloque) y se guarda en un arreglo de structs Bloque. Cada bloque
dice si está libre u ocupado, osea funciona como el bitmap (en la clase le dicen bit vector), de qué
archivo es y en enlazada a qué bloque apunta. Los archivos van en otro arreglo dinámico que crece con
realloc cuando se llena, y cada uno guarda su id, su tamaño y los bloques que usa, así siempre se sabe
dónde está cada archivo.

La simulación va de a un segundo:

- cada 2 segundos se crea un archivo con un tamaño random dentro del rango
- desde los 30 segundos, cada 5 (30, 35, 40...), se elimina un archivo al azar y se liberan sus bloques.
  En esos mismos segundos se accede a una posición de otro archivo al azar y se muestra en qué bloque
  físico cae y cuántos accesos costó encontrarlo
- cada vez que se crea o se borra un archivo se mide la fragmentación externa y se guarda para sacar
  el promedio

### Los métodos

Contigua: los bloques tienen que ir seguidos y el hueco se elige con best fit, osea el hueco libre más
chico donde quepa, como en la clase de memoria. Para leer el bloque k basta con inicio + k, así que es
1 acceso.

Enlazada: sirve cualquier bloque libre, se toman los primeros que aparecen en el bitmap. Cada bloque
guarda un puntero de 4 bytes al siguiente, entonces para datos le quedan 4 bytes menos. Para llegar al
bloque k hay que partir del primero e ir siguiendo los punteros, son k + 1 accesos.

Indexada: también sirve cualquier bloque libre, pero aparte se usa un bloque índice donde van los
punteros a los bloques de datos. Para leer cualquier bloque se lee el índice y después el dato, son 2
accesos.

Es lo mismo que la tabla del f.txt de la clase de file system.

### Cuándo termina

Cuando llega un archivo que no se puede meter, ya sea porque no quedan bloques libres suficientes o,
en contigua, porque hay libres pero ningún hueco seguido alcanza (fragmentación externa).

## Lo que muestra al final

- en qué segundo terminó y con qué método
- archivos creados, eliminados y los que quedaron en el disco
- bloques ocupados, los KB guardados y la fragmentación interna (lo que sobra en el último bloque de
  cada archivo)
- la fragmentación externa al final, el promedio y la máxima, con los huecos y el más grande
- cuántos accesos hubo y el promedio. Si terminó antes de los 30 segundos no hubo ninguno y lo dice

## Supuestos

- los tamaños de archivo son KB enteros
- un puntero a bloque ocupa 4 bytes
- la fragmentación externa se saca como 1 - mayor hueco / bloques libres y solo cuenta en contigua. En
  enlazada e indexada cualquier libre sirve, así que da 0 %, pero igual se muestran los huecos
- el bitmap y la tabla de archivos están en memoria, así que mirarlos no cuenta como acceso, solo leer
  un bloque del disco
- en enlazada la clase dice que el directorio guarda solo el primer y el último bloque. Acá se guardan
  todos para tener el seguimiento, pero al acceder la cadena se recorre con los punteros

## Limitaciones

- en indexada cada archivo tiene un solo bloque índice. Si un archivo necesita más punteros de los que
  caben (bloque / 4), la simulación se termina con ese aviso
- los tiempos son de verdad (sleep), entonces con un disco grande y archivos chicos se puede demorar
  harto en llenarse
