$ErrorActionPreference = 'Stop'
$python = 'C:\Users\Yahir Aquino\.cache\codex-runtimes\codex-primary-runtime\dependencies\python\python.exe'
& $python (Join-Path $PSScriptRoot 'build.py')
if ($LASTEXITCODE -ne 0) { throw "Falló la compilación: $LASTEXITCODE" }
