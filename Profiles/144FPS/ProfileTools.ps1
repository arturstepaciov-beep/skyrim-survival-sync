function Set-ProfileIniValue {
    param([string]$Text, [string]$Section, [string]$Key, [string]$Value)
    $textLf = $Text.Replace("`r`n", "`n").Replace("`r", "`n")
    $sectionPattern = '(?ims)(^[ \t]*\[' + [regex]::Escape($Section) + '\][^\n]*(?:\n|$))(.*?)(?=^[ \t]*\[|\z)'
    $keyPattern = '(?im)^[ \t]*' + [regex]::Escape($Key) + '[ \t]*=[^\n]*$'
    if (-not [regex]::IsMatch($textLf, $sectionPattern)) {
        return $textLf.TrimEnd() + "`n`n[$Section]`n$Key=$Value`n"
    }
    return [regex]::Replace($textLf, $sectionPattern, [System.Text.RegularExpressions.MatchEvaluator]{
        param($match)
        $body = $match.Groups[2].Value
        if ([regex]::IsMatch($body, $keyPattern)) {
            $body = [regex]::Replace($body, $keyPattern, ($Key + '=' + $Value))
        } else {
            $body = $body.TrimEnd() + "`n$Key=$Value`n"
        }
        return $match.Groups[1].Value + $body
    })
}

function Merge-144Profile {
    param([string]$Original, [string]$Profile)
    $section = ''
    $result = $Original
    foreach ($line in ($Profile -split '\r?\n')) {
        if ($line -match '^\s*[;#]' -or [string]::IsNullOrWhiteSpace($line)) { continue }
        if ($line -match '^\s*\[([^\]]+)\]\s*$') {
            $section = $Matches[1]
        } elseif ($line -match '^\s*([^;#=][^=]*?)\s*=\s*(.*?)\s*$') {
            if (-not $section) { throw 'Profile value has no section' }
            $result = Set-ProfileIniValue $result $section $Matches[1].Trim() $Matches[2].Trim()
        }
    }
    return $result.Replace("`r`n", "`n").Replace("`n", "`r`n")
}
