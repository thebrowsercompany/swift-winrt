function Restore-Nuget {
    param(
        [string]$PackagesDir
    )
    $NugetDownloadPath = Join-Path $env:TEMP "nuget.exe"
    if (-not (Test-Path $NugetDownloadPath)) {
        Invoke-WebRequest -Uri "https://dist.nuget.org/win-x86-commandline/latest/nuget.exe" -OutFile $NugetDownloadPath
    }
    $Projections = Get-Content -Path $PSScriptRoot\projections.json | ConvertFrom-Json
    $PackagesConfigContent = "<?xml version=""1.0"" encoding=""utf-8""?>`n"
    $PackagesConfigContent += "<packages>`n"
    $Projections.Packages | ForEach-Object {
        $id = $_.Id
        $version = $_.Version
        $PackagesConfigContent += "  <package id=""$id"" version=""$version"" />`n"
    }
    $PackagesConfigContent += "</packages>"
    $PackagesConfigPath = Join-Path $PSScriptRoot "build\debug\generated\packages.config"
    $PackagesConfigContent | Out-File -FilePath $PackagesConfigPath
    & $NugetDownloadPath restore $PackagesConfigPath -PackagesDirectory $PackagesDir
}

function Get-WinMDInputs() {
    param(
        $Package
    )
    $Id = $Package.Id
    $Version = $Package.Version
    return Get-ChildItem -Path $PackagesDir\$Id.$Version\ -Filter *.winmd -Recurse
}

function Invoke-SwiftWinRT() {
    param(
        [string]$PackagesDir
    )

    $OutputLocation = Join-Path $PSScriptRoot ".\build\debug\generated"
    if (Test-Path $OutputLocation) {
        Remove-Item -Path $OutputLocation -Recurse -Force
    }

    $RspParams = "-output $OutputLocation`n"
    # read projections.json and for each "include" write to -include param. for each "exclude" write to -exclude param
    $Projections = Get-Content -Path $PSScriptRoot\projections.json | ConvertFrom-Json

    $Projections.Include | ForEach-Object {
        $RspParams += "-include $_`n"
    }
    $Projections.Exclude | ForEach-Object {
        $RspParams += "-exclude $_`n"
    }

    $Projections.Packages | ForEach-Object {
        Get-WinMDInputs -Package $_ | ForEach-Object {
            $RspParams += "-input $($_.FullName)`n"
        }

    }

    # write rsp params to file
    $RspFile = Join-Path $PSScriptRoot "swift-winrt.rsp"
    $RspParams | Out-File -FilePath $RspFile
    & $PSScriptRoot\out\release\bin\swiftwinrt.exe "@$RspFile"

    # check error code
    if ($LASTEXITCODE -ne 0) {
        Write-Host "swiftwinrt failed with error code $LASTEXITCODE" -ForegroundColor Red
        return
    }

    # for each directory in output location, copy to project directory into a "generated" folder
    $ProjectDir = Join-Path $PSScriptRoot "Sources"
    $OutputDirectories = Get-ChildItem -Path "$OutputLocation\Sources" -Directory
    $OutputDirectories | ForEach-Object {
        $OutputDir = $_.FullName
        $OutputDirName = $_.Name
        $ProjectGeneratedDir = if ($OutputDirName -eq "CWinRT") { "$OutputDirName" } else { "$OutputDirName\Generated" }
        $ProjectGeneratedDir = Join-Path $ProjectDir "$ProjectGeneratedDir"
        if (-not (Test-Path $ProjectGeneratedDir)) {
            New-Item -Path $ProjectGeneratedDir -ItemType Directory -Force | Out-Null
        }
        Copy-Item -Path $OutputDir\* -Destination $ProjectGeneratedDir -Recurse -Force
    }
}

function Copy-PackageAssets {
    param(
        [string]$PackagesDir
    )

    $Arch = "x64"
    $Projections = Get-Content -Path $PSScriptRoot\projections.json | ConvertFrom-Json
    $Package = $Projections.Package
    $PackageVersion = Get-NugetPackageVersion -Package $Package

    $ProjectName = $Projections.Project
    $ProjectDir = Join-Path $PSScriptRoot "Sources\C$ProjectName\nuget"

    # copy dlls from runtimes\win-<arch>\native to vendor\bin
    $PackageDir = Join-Path $PackagesDir "$Package.$PackageVersion"
    $PackagesRuntimeDir = Join-Path $PackageDir "runtimes\win-$Arch\native"
    $PackagesBinaries = Get-ChildItem -Path $PackagesRuntimeDir -Filter *.dll -Recurse

    $ProjectBinaryDir = Join-Path $ProjectDir "bin"
    if (-not (Test-Path $ProjectBinaryDir)) {
        New-Item -Path $ProjectBinaryDir -ItemType Directory -Force | Out-Null
    }

    $PackagesBinaries | ForEach-Object {
        Copy-Item -Path $_.FullName -Destination $ProjectBinaryDir -Force
    }

    # copy headers from include to vendor\include
    $ProjectHeadersDir = Join-Path $ProjectDir "include"
    if (-not (Test-Path $ProjectHeadersDir)) {
        New-Item -Path $ProjectHeadersDir -ItemType Directory -Force | Out-Null
    }
    $PackagesHeaderDir = Join-Path $PackageDir "include"
    $PackagesHeaders = Get-ChildItem -Path $PackagesHeaderDir -Filter *.h -Recurse
    $PackagesHeaders | ForEach-Object {
        Copy-Item -Path $_.FullName -Destination $ProjectHeadersDir -Force
    }

    # Copy libs from lib\win-<arch> to vendor\lib
    $ProjectLibsDir = Join-Path $ProjectDir "lib"
    $PackagesLibsDir = Join-Path $PackageDir "lib\win10-x64"
    $PackagesLibs = Get-ChildItem -Path $PackagesLibsDir -Filter *.lib -Recurse
    if (-not (Test-Path $ProjectLibsDir)) {
        New-Item -Path $ProjectLibsDir -ItemType Directory -Force | Out-Null
    }
    $PackagesLibs | ForEach-Object {
        Copy-Item -Path $_.FullName -Destination $ProjectLibsDir -Force
    }
}

$PackagesDir = Join-Path $PSScriptRoot ".packages"
Restore-Nuget -PackagesDir $PackagesDir
Invoke-SwiftWinRT -PackagesDir $PackagesDir
#Copy-PackageAssets -PackagesDir $PackagesDir

if ($LASTEXITCODE -eq 0) {
    Write-Host "SwiftWinRT bindings generated successfully!" -ForegroundColor Green
}