[CmdletBinding()]
param(
    [Parameter(Mandatory = $true)][string]$GameDirectory,
    [string]$PluginDirectory = ''
)
$ErrorActionPreference = 'Stop'
. (Join-Path $PSScriptRoot 'ProfileTools.ps1')
$game = (Resolve-Path -LiteralPath $GameDirectory).Path
if (-not (Test-Path -LiteralPath (Join-Path $game 'SkyrimSE.exe'))) {
    throw 'GameDirectory must contain SkyrimSE.exe.'
}
if (Get-Process -Name 'SkyrimSE','SkyrimTogether' -ErrorAction SilentlyContinue) {
    throw 'Close Skyrim and its multiplayer launcher before applying the profile.'
}
if (-not $PluginDirectory) { $PluginDirectory = Join-Path $game 'Data\SKSE\Plugins' }
$plugins = (Resolve-Path -LiteralPath $PluginDirectory).Path
if (-not (Test-Path -LiteralPath (Join-Path $plugins 'SSEDisplayTweaks.dll'))) {
    throw 'Install SSE Display Tweaks and its requirements matching your Skyrim version first.'
}
if (-not (Test-Path -LiteralPath (Join-Path $plugins 'SSEDisplayTweaks.ini'))) {
    throw 'The SSE Display Tweaks base INI is missing; install the complete plugin first.'
}
$target = Join-Path $plugins 'SSEDisplayTweaks_custom.ini'
$original = ''
$backup = $null
if (Test-Path -LiteralPath $target) {
    $original = [IO.File]::ReadAllText($target)
}
$profile = [IO.File]::ReadAllText((Join-Path $PSScriptRoot 'Profile.ini'))
$updated = Merge-144Profile $original $profile
if ($updated -eq $original) {
    Write-Host 'The 144 FPS profile is already applied.'
    return
}
$nonce = [Guid]::NewGuid().ToString('N')
$temp = Join-Path $plugins ("SurvivalSync-" + $nonce + '.tmp')
try {
    [IO.File]::WriteAllText($temp, $updated, [Text.UTF8Encoding]::new($false))
    if (Test-Path -LiteralPath $target) {
        $backup = $target + '.' + (Get-Date -Format 'yyyyMMdd-HHmmss') + '.' + $nonce + '.bak'
        Copy-Item -LiteralPath $target -Destination $backup
    }
    Move-Item -LiteralPath $temp -Destination $target -Force
} finally {
    if (Test-Path -LiteralPath $temp) { Remove-Item -LiteralPath $temp }
}
Write-Host "Applied: $target"
if ($backup) { Write-Host "Previous custom settings: $backup" }
else { Write-Host 'No prior custom INI existed. Remove this custom INI to undo this profile.' }
Write-Host 'Restart using your normal Skyrim Together launch setup. Check the SSEDisplayTweaks log to confirm the plugin loads.'
Write-Host 'This is an experimental profile. It does not install SKSE, Address Library or the Display Tweaks DLL.'
