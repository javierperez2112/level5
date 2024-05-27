# level 5: EDAoogle

## Grupo 2
* Ignacio Rojana
* Javier Pérez
* Rocco Gastaldi

## Generación del índice de búsqueda
El índice de búsqueda se genera con el comando `"CREATE VIRTUAL TABLE fulltext USING fts5 (title, path, body);`. Utilizamos la extensión de *full text search* ó
FTS, por lo que se tiene que crear una `VIRTUAL TABLE`. El código de `mkindex.cpp` itera sobre todos los archivos en el subdirectorio `/wiki` del directorio
especificado a través de la línea de comandos. Para cada archivo, se almacena su título (e.g. *Gato.html*), su ubicación respecto a donde se ejecuta el programa,
y el texto sin las etiquetas de HTML, eliminadas por `removeTags`. Para introducir cada fila a la tabla, se usa el comando
`INSERT INTO fulltext (title, path, body) VALUES('título','/path','texto...')`.

## Búsqueda
Para la búsqueda, se utiliza el comando `SELECT * from fulltext WHERE fulltext MATCH 'término de búsqueda' ORDER BY rank;`. El método `handleRequest` toma el
texto recibido del usuario y lo introduce en el comando como término de búsqueda. La extensión de FTS se encarga de devolver las filas donde se encuentre el
texto especificado por el término de búsqueda a través de la función `onDatabaseEntry`. Para evitar solicitudes maliciosas, se elimina cualquier comilla 
simple (') en el término de búsqueda, así todo el término quedará encerrado en comillas en el comando de SQL, y será interpretado como simple texto.

## Bonus point: operadores y FTS
El índice de búsqueda se implementó con la extensión *fts5* de *sqlite3*, que además de hacer todo el trabajo para la búsqueda, implementa los operadores
`NOT`, `AND` y `OR`, que ponen condiciones para la búsqueda. Por ejemplo, `carl NOT sagan` no devolvería el artículo de Carl Sagan, pero sí otros que
contienen *Carl*. `planeta AND enano` devuelve sólo artículos que contienen las dos palabras, así que devuelve, por ejemplo, *Plutón*. `tierra OR marte`
devuelve cualquier artículo que mencione a alguno de los dos planetas. Estos operadores se pueden combinar, y si se escriben varios términos, se toman
como si hubiera un `AND` en el medio.
