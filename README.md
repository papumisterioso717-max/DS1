# DS1

Proyecto experimental de recompilación de Dead Space para Xbox 360 con XenonRecomp.

## Estado

El diagnóstico procesa 24 dibujos iniciales sin salida y avanza por la configuración de GPU. Ya conserva descriptores de recursos y evalúa condiciones con máscaras de 64 bits. El dibujo activo en 0x1F8E0594 pasa su condición y se detiene en `PENDING_GPU_DRAW renderer=absent`. No hay imagen ni ejecución jugable. Véase GPU-PREDICATION.md y status.json.

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

## Registro histórico de avances (el estado vigente figura arriba)

El analizador `inspect-shader-flow.py` identifica el flujo lineal de los shaders capturados: cuatro instrucciones ALU de vértices y una de píxeles. Las cinco operaciones MAX/RETAIN_PREV y sus máscaras de salida están decodificadas: interpolador cero, posición (0,0,0,1) y color MAX(r0,r0). No aparecen destinos de exportación a memoria en ellas. Falta ejecutar los shaders y rasterizar; este informe no autoriza omitir el dibujo. Véase SHADER-ANALYSIS.md.

`evaluate-shader-exports.py` evalúa las salidas del subconjunto capturado fuera del runtime. Confirma las constantes de vértices y conserva como desconocido el color de píxel si falta r0. Sus 17 pruebas pasan; aún no ejecuta el dibujo ni genera imagen.

`link-point-shaders.py` conecta fuera del runtime el interpolador del primer punto con r0 del shader de píxeles. Produce color hipotético (0,0,0,0), sin calcular cobertura ni escribir imagen. Solo admite la configuración capturada; nueve pruebas verifican el enlace y sus rechazos.

El inspector de geometría identifica un punto de 1x1 y el píxel candidato (0,0). La cobertura final sigue pendiente del recorte global y el estado de muestreo; no hay imagen renderizada.

La captura del runtime ahora distingue escrituras del juego de valores iniciales en superficie y recorte global. Antes del primer dibujo no se observaron escrituras de esos tres registros: su inicialización sigue por resolver. El diagnóstico permanece detenido, sin imagen.

La vista previa posterior al primer dibujo revela otros 23 dibujos idénticos en la misma lista, sin cambios de configuración. Es una inspección de lectura: ninguno se ejecuta ni se omite.

Se localizaron los generadores recompilados de la secuencia y se verificaron en aislamiento: 24 dibujos y la lista previa de configuración GPR coinciden con la captura. Véase DRAW-SEQUENCE.md. La ejecución gráfica sigue pendiente.

Existe una prueba opcional `DS1_PROBE_ZERO_SURFACE=1`, desactivada por defecto, que omite hasta 24 dibujos con estado restringido siguiendo una compatibilidad de Xenia. Sus efectos de shader no están verificados. La prueba llega a `EVENT_WRITE_SHD` y se detiene; no genera imagen. El modo normal conserva el bloqueo del primer dibujo. Detalles en DRAW-SEQUENCE.md.

El diagnóstico identifica el evento de finalización posterior: solicita escribir `BF8E00D4` en `1FCEE004`. Se captura y decodifica, pero no se escribe una finalización de shaders no ejecutados.

Se verificó en aislamiento el productor de dos señales de finalización y sus lectores de CPU: posición etiquetada del búfer y número de secuencia. Detalles en COMPLETION-PROTOCOL.md; la finalización GPU real sigue pendiente.

El evento literal PS_DONE_TS ya puede publicar su señal para trabajo síncrono admitido, con escritura atómica y validación de memoria. Rechaza señales después de dibujos omitidos; el juego todavía no supera su bloqueo gráfico.
