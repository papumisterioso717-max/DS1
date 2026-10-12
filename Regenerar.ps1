$ErrorActionPreference = 'Stop'
$recompiler = Join-Path $PSScriptRoot '..\XenonTools\XenonRecomp.exe'
$contextHeader = 'C:\Users\Yahir Aquino\Documents\Codex\XenonRecomp\XenonUtils\ppc_context.h'
& $recompiler (Join-Path $PSScriptRoot 'DeadSpace.toml') $contextHeader 2>&1 | Tee-Object -FilePath (Join-Path $PSScriptRoot 'generation.log')
if ($LASTEXITCODE -ne 0) { throw "XenonRecomp terminó con código $LASTEXITCODE" }
Write-Host 'Generación terminada. Revisa generation.log: un código 0 no garantiza ausencia de errores.'
