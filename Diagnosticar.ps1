param([string]$Xex = 'C:\Users\Yahir Aquino\Downloads\Dead Space 1\default.xex')
$ErrorActionPreference = 'Stop'
& (Join-Path $PSScriptRoot 'bin\DeadSpaceDiagnostic.exe') $Xex
if ($LASTEXITCODE -eq 20) { Write-Host 'Diagnóstico detenido en un servicio Xbox pendiente (esperado).' }
elseif ($LASTEXITCODE -ne 0) { throw "El diagnóstico terminó con código $LASTEXITCODE" }
