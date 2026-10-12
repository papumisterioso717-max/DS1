# DS1

Proyecto experimental de recompilación de Dead Space para Xbox 360 con XenonRecomp.

## Estado

El diagnóstico entra en el código recompilado y actualmente crea ocho hilos de trabajo y captura la configuración gráfica en modo de diagnóstico y registra las notificaciones de cierre e interrupción; los alias de memoria física ya comparten respaldo y se registra la dirección del identificador de comandos y se captura la configuración de la cola GPU sin ejecutar comandos y la consulta de gamma funciona; el siguiente bloqueo es una escritura al registro GPU `0x7FC80714`; el marcador de importación que causaba la lectura en `0x59000100` ya está resuelto. Todavía no es jugable y no dispone de un renderizador.

Pruebas actuales: 320/320 archivos compilados y 50 comprobaciones de memoria física, además de las demás pruebas del runtime. El respaldo compartido requiere Windows 10 1803 o posterior.

## Contenido

- `runtime/`: servicios del sistema implementados para el diagnóstico.
- `tests/`: pruebas del runtime y de las instrucciones traducidas.
- Configuración de XenonRecomp, tablas de saltos y parche local del recompilador.
- `LEEME.txt` y `status.json`: avances y limitaciones registrados.

El XEX original, recursos del juego, fuentes generadas y compilaciones no están incluidos. Se requiere una copia propia del juego para regenerar el código.

## Configuración local

Esta es una instantánea del proyecto de desarrollo. Los scripts todavía contienen rutas de la máquina original; deben ajustarse antes de usarlos desde esta carpeta o desde otra computadora. `Regenerar.ps1` requiere XenonRecomp con el parche incluido y genera `ppc/`. `build.py` requiere las herramientas y bibliotecas indicadas en sus rutas.

Repositorio de XenonRecomp: https://github.com/hedge-dev/XenonRecomp

No se asigna una licencia general a los componentes de terceros; se conservan sus avisos en los archivos correspondientes.
