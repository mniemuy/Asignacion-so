# Tarea 3 Sistemas Operativos

Simulador de asignación de archivos en disco con asignación contigua, enlazada e indexada.
Los archivos no se crean de verdad, solo se lleva la cuenta de qué bloques usa cada uno y dónde quedan.

Va pidiendo los datos en el mismo orden de la pauta:

- tamaño del disco en MB
- tamaño de los bloques en KB
- el método: 1 contigua, 2 enlazada o 3 indexada
- mínimo del archivo en KB
- máximo del archivo en KB

Si no se quieren escribir uno por uno se pueden pasar de una. Por ejemplo disco de 1 MB, bloques de
4 KB, enlazada y archivos de 16 a 64 KB:

```
printf '1\n4\n2\n16\n64\n' | ./Tarea3SO
```

Si el disco o el bloque están malos (el bloque más grande que el disco, o un disco de más de 1 TB), el
método no es 1, 2 o 3 o el rango está malo, avisa y se cierra. También se cierra si salen demasiados
bloques (más de unos 16 millones), en ese caso hay que usar bloques más grandes.

## Cómo funciona todo

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
accesos. Si el archivo es muy grande y los punteros no caben en un solo índice, se usan varios y se
encadenan: la última entrada de cada índice apunta al siguiente, así que cada uno guarda (bloque / 4) - 1
bloques de datos. En ese caso hay que ir pasando por los índices hasta llegar al que corresponde, son
los índices recorridos + 1 accesos.

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

- los tiempos son de verdad (sleep), entonces con un disco grande y archivos chicos se puede demorar
  harto en llenarse
- los datos se leen con scanf, así que si se escribe algo que no es número queda en 0 y el programa
  avisa que el dato está malo y se cierra
