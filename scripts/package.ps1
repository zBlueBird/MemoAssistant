[CmdletBinding()]
param(
    [string]$BuildDirectory = "build-release",
    [string]$OutputDirectory = "dist",
    [string]$QtDirectory = "D:\software\QtCreator\6.11.1\mingw_64",
    [string]$MinGwBinDirectory = "D:\software\QtCreator\Tools\mingw1310_64\bin",
    [string]$CMake = "D:\software\QtCreator\Tools\CMake_64\bin\cmake.exe",
    [string]$NinjaDirectory = "D:\software\QtCreator\Tools\Ninja"
)

$ErrorActionPreference = "Stop"
$projectDirectory = [IO.Path]::GetFullPath((Split-Path -Parent $PSScriptRoot))
$buildPath = [IO.Path]::GetFullPath((Join-Path $projectDirectory $BuildDirectory))
$outputPath = [IO.Path]::GetFullPath((Join-Path $projectDirectory $OutputDirectory))
$projectPrefix = $projectDirectory.TrimEnd('\') + '\'
$outputName = Split-Path -Leaf $outputPath

if (-not $buildPath.StartsWith($projectPrefix, [StringComparison]::OrdinalIgnoreCase)) {
    throw "Build directory must be inside the project directory."
}
if ((Split-Path -Parent $outputPath) -ne $projectDirectory -or $outputName -notmatch '^dist(?:[-_.].+)?$') {
    throw "Output directory must be a direct child of the project named dist or dist-<suffix>."
}

$executablePath = Join-Path $buildPath "MemoAssistant.exe"
$windeployqt = Join-Path $QtDirectory "bin\windeployqt.exe"
$compiler = Join-Path $MinGwBinDirectory "g++.exe"

foreach ($requiredPath in @($CMake, $windeployqt, $compiler)) {
    if (-not (Test-Path -LiteralPath $requiredPath -PathType Leaf)) {
        throw "Required tool not found: $requiredPath"
    }
}

$env:PATH = "$MinGwBinDirectory;$NinjaDirectory;$env:PATH"

& $CMake -S $projectDirectory -B $buildPath -G Ninja `
    "-DCMAKE_PREFIX_PATH=$($QtDirectory.Replace('\', '/'))" `
    "-DCMAKE_CXX_COMPILER=$($compiler.Replace('\', '/'))" `
    -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
if ($LASTEXITCODE -ne 0) { throw "CMake configuration failed with exit code $LASTEXITCODE." }

& $CMake --build $buildPath
if ($LASTEXITCODE -ne 0) { throw "Release build failed with exit code $LASTEXITCODE." }
if (-not (Test-Path -LiteralPath $executablePath -PathType Leaf)) {
    throw "Release executable not found: $executablePath"
}

if (Test-Path -LiteralPath $outputPath) {
    Remove-Item -LiteralPath $outputPath -Recurse -Force -Confirm:$false
}
New-Item -ItemType Directory -Path $outputPath | Out-Null
Copy-Item -LiteralPath $executablePath -Destination $outputPath

$deployedExecutable = Join-Path $outputPath "MemoAssistant.exe"
& $windeployqt --release --compiler-runtime --no-translations --no-opengl-sw `
    --skip-plugin-types generic,imageformats,networkinformation,styles,tls `
    $deployedExecutable
if ($LASTEXITCODE -ne 0) { throw "windeployqt failed with exit code $LASTEXITCODE." }

Copy-Item -LiteralPath (Join-Path $projectDirectory "THIRD_PARTY_NOTICES.md") -Destination $outputPath
Copy-Item -LiteralPath (Join-Path $projectDirectory "licenses") -Destination (Join-Path $outputPath "licenses") -Recurse

$requiredFiles = @(
    "MemoAssistant.exe",
    "Qt6Core.dll",
    "Qt6Gui.dll",
    "Qt6Widgets.dll",
    "Qt6Sql.dll",
    "Qt6Network.dll",
    "platforms\qwindows.dll",
    "sqldrivers\qsqlite.dll",
    "libgcc_s_seh-1.dll",
    "libstdc++-6.dll",
    "libwinpthread-1.dll",
    "THIRD_PARTY_NOTICES.md",
    "licenses\LGPL-3.0.txt"
)

$missingFiles = $requiredFiles | Where-Object {
    -not (Test-Path -LiteralPath (Join-Path $outputPath $_) -PathType Leaf)
}
if ($missingFiles) {
    throw "Package is missing required files: $($missingFiles -join ', ')"
}

$sqlDriverDirectory = Join-Path $outputPath "sqldrivers"
Get-ChildItem -LiteralPath $sqlDriverDirectory -Filter "*.dll" |
    Where-Object { $_.Name -ne "qsqlite.dll" } |
    Remove-Item -Force -Confirm:$false

$remainingSqlDrivers = Get-ChildItem -LiteralPath $sqlDriverDirectory -Filter "*.dll"
if ($remainingSqlDrivers.Count -ne 1 -or $remainingSqlDrivers[0].Name -ne "qsqlite.dll") {
    throw "Package must contain qsqlite.dll as its only SQL driver."
}

$manifestPath = Join-Path $outputPath "SHA256SUMS.txt"
Get-ChildItem -LiteralPath $outputPath -File -Recurse |
    Where-Object { $_.FullName -ne $manifestPath } |
    Sort-Object FullName |
    ForEach-Object {
        $hash = (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
        $relativePath = $_.FullName.Substring($outputPath.Length + 1).Replace('\', '/')
        "$hash  $relativePath"
    } | Set-Content -LiteralPath $manifestPath -Encoding utf8

Write-Host "Package created at: $outputPath"
Write-Host "SHA-256 manifest: $manifestPath"