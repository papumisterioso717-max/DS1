# DS1

Proyecto experimental de recompilación de Dead Space para Xbox 360 con XenonRecomp.

## Estado

El diagnóstico crea ocho hilos de trabajo y llega a enviar la primera cola de comandos GPU. El procesador parcial reconoce la inicialización y las listas indirectas; resuelve la primera espera de coherencia en memoria compartida y recibe una segunda lista, que conserva la configuración SQ y conserva cuatro registros de significado desconocido y configura las escrituras scratch y resuelve la espera inicial de inactividad 3D y conserva `SQ_EO_RT` y interpreta `REG_RMW` y conserva `0x0081`/`0x0082` como valores opacos y conserva el grupo de configuración inicial, realiza una escritura scratch y entra en una lista anidada, que supera `INVALIDATE_STATE` y conserva dos shaders inmediatos (24 palabras de vértices y 9 de píxeles) y conserva su configuración y conserva los límites de índices de vértices y conserva la configuración de visibilidad y se detiene en `PA_CL_CLIP_CNTL` (`0x2204`). Los shaders todavía no se traducen ni ejecutan. Una captura de solo lectura permite inspeccionar los siguientes comandos sin ejecutarlos. No dispone de renderizador y aún no es jugable.

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
