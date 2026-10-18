param([Parameter(Mandatory=$true)][string]$ArmBin)
$ErrorActionPreference = 'Stop'
$compiler = Join-Path $ArmBin 'arm-none-eabi-gcc.exe'
$linker = Join-Path $ArmBin 'arm-none-eabi-ld.exe'
$strip = Join-Path $ArmBin 'arm-none-eabi-strip.exe'
foreach ($tool in @($compiler,$linker,$strip)) {
    if (!(Test-Path -LiteralPath $tool)) { throw "Missing tool: $tool" }
}
$source = Join-Path $PSScriptRoot '..\magiclantern-zh-src\modules\af_live'
Push-Location $source
try {
    New-Item -ItemType Directory -Force -Path build | Out-Null
    # Freeze the historical generated metadata (including its build date) for
    # a byte-for-byte reproduction. The regular Makefile regenerates it.
    Copy-Item -LiteralPath (Join-Path $PSScriptRoot 'r21-module_strings.h') -Destination 'build/module_strings.h' -Force
    $flags = @('-Os','-D__ARM__','-nostdlib','-I../../include','-Wall','-Wextra',
        '-Werror-implicit-function-declaration','-Wdouble-promotion','-Winline',
        '-Wundef','-Wno-unused-parameter','-Wno-unused-function','-Wno-format',
        '-fomit-frame-pointer','-fno-strict-aliasing','-ffast-math',
        '-fsingle-precision-constant','-fno-builtin-printf','-fno-common',
        '-fno-extended-identifiers','-std=gnu99','-I../../src','-ggdb3',
        '-DCONFIG_MAGICLANTERN=1','-DCONFIG_DEBUGMSG=','-march=armv5te',
        '-mlong-calls','-DMODULE','-DMODULE_NAME=af_live','-DCONFIG_MAGICLANTERN',
        '-DVIDAF_LEGACY_R9','-I../../src/','-I.','-c','-o','build/af_live.o','af_live.c')
    & $compiler @flags
    if ($LASTEXITCODE) { throw 'AF compile failed' }
    & $linker '-r' '-o' 'build/af_live.mo' 'build/af_live.o'
    if ($LASTEXITCODE) { throw 'AF link failed' }
    & $strip '--strip-debug' '--strip-unneeded' '--discard-all' '--discard-locals' 'build/af_live.mo'
    if ($LASTEXITCODE) { throw 'AF strip failed' }
    Get-FileHash -LiteralPath 'build/af_live.mo' -Algorithm SHA256
} finally { Pop-Location }
