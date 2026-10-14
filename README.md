# DS1

Proyecto experimental de recompilación de Dead Space para Xbox 360 con XenonRecomp.

## Estado

El diagnóstico arranca el código traducido, completa la primera entrega de comandos GPU y procesa dos listas de configuración anidadas de la segunda entrega. Conserva un shader de vértices y uno de píxeles, junto con su configuración. Captura y decodifica el primer DRAW_INDX_2 (0x1FCEA980): cuenta 1, máscara de color cero y profundidad desactivada; se detiene antes de ejecutarlo. No traduce ni ejecuta shaders y no dispone de renderizador: aún no muestra imagen ni es jugable.

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

El analizador `inspect-shader-flow.py` identifica el flujo lineal de los shaders capturados: cuatro instrucciones ALU de vértices y una de píxeles. Las cinco operaciones MAX/RETAIN_PREV y sus máscaras de salida están decodificadas: interpolador cero, posición (0,0,0,1) y color MAX(r0,r0). No aparecen destinos de exportación a memoria en ellas. Falta ejecutar los shaders y rasterizar; este informe no autoriza omitir el dibujo. Véase SHADER-ANALYSIS.md.
