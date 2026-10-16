$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'ProfileTools.ps1')
function Assert-True([bool]$condition, [string]$message) {
    if (-not $condition) { throw "FAIL: $message" }
}
$profile = [IO.File]::ReadAllText((Join-Path $PSScriptRoot 'Profile.ini'))
$original = @'
; existing custom settings
[Render]
Resolution=2560x1440
FramerateLimit=60
FramerateLimit=90
EnableVSync=false
[Other]
Unrelated=value[with brackets]
[HAVOK]
Enabled=false
DynamicMaxTimeScaling=false
MaximumFramerate=60
'@
$merged = Merge-144Profile $original $profile
Assert-True ($merged.Contains('Resolution=2560x1440')) 'preserve resolution'
Assert-True ($merged.Contains('EnableVSync=false')) 'preserve VSync'
Assert-True ($merged.Contains('Unrelated=value[with brackets]')) 'preserve unrelated section'
Assert-True ($merged.Contains('; existing custom settings')) 'preserve comments'
Assert-True (([regex]::Matches($merged, '(?m)^FramerateLimit=144\r?$')).Count -eq 2) 'update duplicate keys consistently'
Assert-True ($merged.Contains('DynamicMaxTimeScaling=true')) 'enable dynamic timing'
Assert-True ($merged.Contains('MaximumFramerate=144')) 'set Havok upper bound'
Assert-True ($merged.Contains('LoadingScreenFramerateLimit=60')) 'keep loading capped'
Assert-True ((Merge-144Profile $merged $profile) -eq $merged) 'idempotent merge'
$commented = Merge-144Profile '' ("; ignored=comment`n# ignored=comment`n" + $profile)
Assert-True (-not $commented.Contains('ignored=')) 'ignore profile comments'

$tempRoot = Join-Path ([IO.Path]::GetTempPath()) ('sync144-test-' + [Guid]::NewGuid().ToString('N'))
try {
    $plugins = Join-Path $tempRoot 'Data\SKSE\Plugins'
    New-Item -ItemType Directory -Path $plugins -Force | Out-Null
    [IO.File]::WriteAllText((Join-Path $tempRoot 'SkyrimSE.exe'), '')
    $base = Join-Path $plugins 'SSEDisplayTweaks.ini'
    [IO.File]::WriteAllText($base, 'base unchanged')
    $target = Join-Path $plugins 'SSEDisplayTweaks_custom.ini'
    $failed = $false
    try { & (Join-Path $PSScriptRoot 'Apply-144FPS.ps1') -GameDirectory $tempRoot }
    catch { $failed = $true }
    Assert-True $failed 'missing DLL blocks application'
    Assert-True (-not (Test-Path -LiteralPath $target)) 'missing dependency writes no override'
    [IO.File]::WriteAllText((Join-Path $plugins 'SSEDisplayTweaks.dll'), '')
    [IO.File]::WriteAllText($target, $original)
    & (Join-Path $PSScriptRoot 'Apply-144FPS.ps1') -GameDirectory $tempRoot
    $backups = @(Get-ChildItem -LiteralPath $plugins -Filter '*.bak')
    Assert-True ($backups.Count -eq 1) 'backup created'
    Assert-True ([IO.File]::ReadAllText($backups[0].FullName) -eq $original) 'backup is exact'
    Assert-True ([IO.File]::ReadAllText($base) -eq 'base unchanged') 'base INI unchanged'
    Assert-True ([IO.File]::ReadAllText($target) -eq $merged) 'installer merges real custom INI'
    & (Join-Path $PSScriptRoot 'Apply-144FPS.ps1') -GameDirectory $tempRoot
    Assert-True (@(Get-ChildItem -LiteralPath $plugins -Filter '*.bak').Count -eq 1) 'repeat apply creates no redundant backup'
    Remove-Item -LiteralPath $target
    & (Join-Path $PSScriptRoot 'Apply-144FPS.ps1') -GameDirectory $tempRoot
    Assert-True ([IO.File]::ReadAllText($target).Contains('FramerateLimit=144')) 'fresh custom INI created'
    Write-Host 'PASS: profile preservation, duplicates, idempotence, dependency rejection, exact backup, fresh install'
} finally {
    if (Test-Path -LiteralPath $tempRoot) { Remove-Item -LiteralPath $tempRoot -Recurse -Force }
}
