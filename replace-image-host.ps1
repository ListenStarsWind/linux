param(
    [string]$Root = "."
)

$OldHost = "md-wind.oss-cn-nanjing.aliyuncs.com"
$NewHost = "wind-note-image.oss-cn-shenzhen.aliyuncs.com"

$Files = Get-ChildItem -Path $Root -Recurse -File -Filter *.md

$ChangedFiles = 0
$TotalReplacements = 0

foreach ($File in $Files) {
    $Content = [System.IO.File]::ReadAllText($File.FullName)

    $Count = ([regex]::Matches(
        $Content,
        [regex]::Escape($OldHost)
    )).Count

    if ($Count -eq 0) {
        continue
    }

    $NewContent = $Content.Replace($OldHost, $NewHost)

    [System.IO.File]::WriteAllText(
        $File.FullName,
        $NewContent,
        [System.Text.UTF8Encoding]::new($false)
    )

    $ChangedFiles++
    $TotalReplacements += $Count

    Write-Host "已修改: $($File.FullName)  ($Count 处)"
}

Write-Host ""
Write-Host "完成"
Write-Host "扫描 Markdown 文件: $($Files.Count)"
Write-Host "修改文件: $ChangedFiles"
Write-Host "替换链接: $TotalReplacements"