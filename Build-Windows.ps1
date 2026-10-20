# Build party-leader quest snapshots alongside the retained v0.6 features.
param(
    [string]$WorkDirectory = (Join-Path $PSScriptRoot 'build-work'),
    [string]$BuildStamp = ''
)
$ErrorActionPreference = 'Stop'
$startedUtc = [DateTime]::UtcNow
$vilniusZone = [TimeZoneInfo]::FindSystemTimeZoneById('FLE Standard Time')
$startedLocal = [TimeZoneInfo]::ConvertTimeFromUtc($startedUtc, $vilniusZone)
if (-not $BuildStamp) { $BuildStamp = $startedLocal.ToString('yyyy-MM-dd_HH-mm-ss') + '-LT' }
if ($BuildStamp -notmatch '^\d{4}-\d{2}-\d{2}_\d{2}-\d{2}-\d{2}-LT$') { throw 'Invalid build timestamp' }
$packageName = "SurvivalSync-v0.8.5-$BuildStamp-Windows"
$buildVersion = "survival-sync-v085-direct-teleport-$BuildStamp"
$sourceCommit = (& git -C $PSScriptRoot rev-parse HEAD).Trim()
if ($LASTEXITCODE -ne 0) { throw 'Cannot determine package source commit' }
function Run([string]$Program, [string[]]$Arguments) {
    & $Program @Arguments
    if ($LASTEXITCODE -ne 0) { throw "$Program failed with exit code $LASTEXITCODE" }
}
foreach ($tool in @('git','xmake','node','pnpm')) {
    if (-not (Get-Command $tool -ErrorAction SilentlyContinue)) {
        throw "Missing $tool. Read README.md prerequisites first."
    }
}
$patchSource = Join-Path $PSScriptRoot 'world-items-v0.2.patch'
$source = Join-Path $WorkDirectory 'TiltedEvolution'
if (Test-Path $source) { throw "Choose a fresh WorkDirectory; existing checkout will not be overwritten." }
New-Item -ItemType Directory -Force $WorkDirectory | Out-Null
# Git for Windows may check out the patch with CRLF; binary patch headers require LF.
$patch = Join-Path $WorkDirectory 'source.patch'
[IO.File]::WriteAllText($patch, [IO.File]::ReadAllText($patchSource).Replace("`r`n", "`n"), [Text.UTF8Encoding]::new($false))
Run -Program 'git' -Arguments @('clone','https://github.com/tiltedphoques/TiltedEvolution.git',$source)
Push-Location $source
try {
    Run -Program 'git' -Arguments @('checkout','-b','survival-sync-v01','9ce0338522277e0ff0a12e8d2aa09ce67ad41a59')
    Run -Program 'git' -Arguments @('apply','--check',$patch)
    Run -Program 'git' -Arguments @('apply',$patch)
    $questPatch = Join-Path $WorkDirectory 'quest-sync.patch'
    [IO.File]::WriteAllText($questPatch, [IO.File]::ReadAllText((Join-Path $PSScriptRoot 'quest-sync-v07.patch')).Replace("`r`n", "`n"), [Text.UTF8Encoding]::new($false))
    Run -Program 'git' -Arguments @('apply','--check',$questPatch)
    Run -Program 'git' -Arguments @('apply',$questPatch)
    $progressPatch = Join-Path $WorkDirectory 'world-progress.patch'
    [IO.File]::WriteAllText($progressPatch, [IO.File]::ReadAllText((Join-Path $PSScriptRoot 'world-progress-v08.patch')).Replace("`r`n", "`n"), [Text.UTF8Encoding]::new($false))
    Run -Program 'git' -Arguments @('apply','--check',$progressPatch)
    Run -Program 'git' -Arguments @('apply',$progressPatch)
    $manifestInput = [IO.File]::OpenRead((Join-Path $PSScriptRoot 'WorldProgressBindings.h.gz'))
    $manifestGzip = [IO.Compression.GZipStream]::new($manifestInput, [IO.Compression.CompressionMode]::Decompress)
    $manifestOutput = [IO.File]::Create((Join-Path $source 'Code/common/WorldProgressBindings.h'))
    try { $manifestGzip.CopyTo($manifestOutput) } finally { $manifestOutput.Dispose(); $manifestGzip.Dispose(); $manifestInput.Dispose() }
    $buildInfoPath = Join-Path $source 'Code/common/BuildInfo.h.in'
    $buildInfo = Get-Content $buildInfoPath -Raw
    $buildInfo = $buildInfo.Replace('survival-sync-v085-direct-teleport-draft', $buildVersion)
    [IO.File]::WriteAllText($buildInfoPath, $buildInfo, [Text.UTF8Encoding]::new($false))
    Run -Program 'git' -Arguments @('submodule','update','--init','--recursive','--depth=1')
    Run -Program 'xmake' -Arguments @('config','--plat=windows','--arch=x64','--mode=releasedbg','--yes')
    # Run persistence checks first so Windows storage failures fail before the full compile.
    Run -Program 'xmake' -Arguments @('-y','CoopChestStoreTests')
    Run -Program 'xmake' -Arguments @('run','CoopChestStoreTests')
    Run -Program 'xmake' -Arguments @('-y','TPTests')
    # Native encoding tests include the new request packet round trip.
    Run -Program 'xmake' -Arguments @('run','TPTests','[encoding.survival],[encoding.worlditems],[encoding.weather],[encoding.homestead],[encoding.presentation],[encoding.coopchest],[encoding.questsync],[percentage-health],[world-progress],[encoding.worldprogress],[housing-layout],[coop-fixes]')
    Run -Program 'xmake' -Arguments @('-y')
    Run -Program 'xmake' -Arguments @('run','WorldItemPolicyTests')
    Run -Program 'xmake' -Arguments @('run','HomesteadPolicyTests')
    Run -Program 'xmake' -Arguments @('run','CoopChestStoreTests')
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
    if (-not (Test-Path (Join-Path $package 'SurvivalSyncCoop.esp'))) { throw 'Missing co-op chest base plugin' }
    $profileSource = Join-Path $PSScriptRoot 'Profiles'
    if (Test-Path $profileSource) { Copy-Item -Path $profileSource -Destination (Join-Path $package 'Profiles') -Recurse -Force }
    Copy-Item -Path 'Tools/CoopChests/README.md' -Destination (Join-Path $package 'CoopChests-README.md')
    Copy-Item -Path (Join-Path $PSScriptRoot 'README.md') -Destination (Join-Path $package 'SurvivalSync-README.md')
    Copy-Item -Path (Join-Path $PSScriptRoot 'LICENSE') -Destination (Join-Path $package 'LICENSE')
    @{
        package = $packageName
        buildVersion = $buildVersion
        sourceCommit = $sourceCommit
        upstreamCommit = '9ce0338522277e0ff0a12e8d2aa09ce67ad41a59'
        startedUtc = $startedUtc.ToString('o')
        startedVilnius = $startedLocal.ToString('yyyy-MM-dd HH:mm:ss')
        timeZone = 'Europe/Vilnius'
    } | ConvertTo-Json | Set-Content -Path (Join-Path $package 'BuildInfo.json') -Encoding utf8
    $zip = Join-Path $WorkDirectory "$packageName.zip"
    Compress-Archive -Path (Join-Path $package '*') -DestinationPath $zip -Force
    Write-Host "Windows package created: $zip"
    Write-Host 'This script never changes your installed Skyrim files.'
} finally { Pop-Location }


