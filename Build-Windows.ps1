# Rebuild corrected Volcano alpha sources; house/chest features are not implemented.
param([string]$WorkDirectory = (Join-Path $PSScriptRoot 'build-work'))
$ErrorActionPreference = 'Stop'
function Run([string]$Program, [string[]]$Arguments) {
    & $Program @Arguments
    if ($LASTEXITCODE -ne 0) { throw "$Program failed with exit code $LASTEXITCODE" }
}
foreach ($tool in @('git','xmake','node','pnpm')) {
    if (-not (Get-Command $tool -ErrorAction SilentlyContinue)) {
        throw "Missing $tool. Read README.md prerequisites first."
    }
}
$patch = Join-Path $PSScriptRoot 'world-items-v0.2.patch'
$source = Join-Path $WorkDirectory 'TiltedEvolution'
if (Test-Path $source) { throw "Choose a fresh WorkDirectory; existing checkout will not be overwritten." }
New-Item -ItemType Directory -Force $WorkDirectory | Out-Null
Run -Program 'git' -Arguments @('clone','https://github.com/tiltedphoques/TiltedEvolution.git',$source)
Push-Location $source
try {
    Run -Program 'git' -Arguments @('checkout','-b','survival-sync-v01','9ce0338522277e0ff0a12e8d2aa09ce67ad41a59')
    Run -Program 'git' -Arguments @('apply','--check',$patch)
    Run -Program 'git' -Arguments @('apply',$patch)
    Run -Program 'git' -Arguments @('submodule','update','--init','--recursive','--depth=1')
    Run -Program 'xmake' -Arguments @('config','--plat=windows','--arch=x64','--mode=releasedbg','--yes')
    Run -Program 'xmake' -Arguments @('-y')
    # Native encoding tests include the new request packet round trip.
    Run -Program 'xmake' -Arguments @('run','TPTests','[encoding.survival],[encoding.worlditems],[encoding.weather]')
    Run -Program 'xmake' -Arguments @('run','WorldItemPolicyTests')
    Run -Program 'pnpm' -Arguments @('--prefix','Code/skyrim_ui','install')
    Run -Program 'pnpm' -Arguments @('--prefix','Code/skyrim_ui','deploy:production')
    Run -Program 'xmake' -Arguments @('install','-o','distrib')
    $package = Join-Path $WorkDirectory 'package'
    $mod = Join-Path $package 'SkyrimTogetherReborn'
    New-Item -ItemType Directory -Force $mod | Out-Null
    Copy-Item -Path 'GameFiles/Skyrim/*' -Destination $package -Recurse -Force
    Copy-Item -Path 'build/windows/x64/releasedbg/*' -Destination $mod -Recurse -Force
    Copy-Item -Path 'distrib/bin/*' -Destination $mod -Recurse -Force
    Copy-Item -Path 'Code/skyrim_ui/dist/UI' -Destination $mod -Recurse -Force
    Get-ChildItem $mod -File | Where-Object { $_.Extension -in @('.lib','.exp','.pdb','.obj') -or $_.Name -like '*Tests.exe' } | Remove-Item
    foreach ($name in @('SkyrimTogether.exe','SkyrimTogetherServer.exe')) {
        if (-not (Test-Path (Join-Path $mod $name))) { throw "Missing packaged binary: $name" }
    }
    $configDir = Join-Path $mod 'config'
    New-Item -ItemType Directory -Force $configDir | Out-Null
    $configPath = Join-Path $configDir 'STServer.ini'
    Run -Program (Join-Path $mod 'SkyrimTogetherServer.exe') -Arguments @('--generate-config',$configPath)
    $ini = Get-Content $configPath -Raw
    foreach ($key in @('bEnableSurvivalTimeSync','bSyncPlayerCalendar')) {
        $pattern = '(?m)^\s*' + [regex]::Escape($key) + '\s*=.*$'
        if ($ini -notmatch $pattern) { throw "Missing generated config setting: $key" }
        $ini = [regex]::Replace($ini,$pattern,"$key=true")
    }
    Set-Content -Path $configPath -Value $ini -Encoding utf8
    Copy-Item -Path (Join-Path $PSScriptRoot 'README.md') -Destination (Join-Path $package 'SurvivalSync-README.md')
    Copy-Item -Path (Join-Path $PSScriptRoot 'LICENSE') -Destination (Join-Path $package 'LICENSE')
    $zip = Join-Path $WorkDirectory 'SurvivalSync-v0.4-Windows.zip'
    Compress-Archive -Path (Join-Path $package '*') -DestinationPath $zip -Force
    Write-Host "Windows package created: $zip"
    Write-Host 'This script never changes your installed Skyrim files.'
} finally { Pop-Location }
