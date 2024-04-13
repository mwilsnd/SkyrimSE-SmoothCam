if (!
    #current role
    (New-Object Security.Principal.WindowsPrincipal(
        [Security.Principal.WindowsIdentity]::GetCurrent()
    #is admin?
    )).IsInRole(
        [Security.Principal.WindowsBuiltInRole]::Administrator
    )
) {
    echo "You must run this script from an elevated powershell window!"
    exit
}

. .\scripts/install-vsbuild-tools.ps1
. .\scripts/install-python3.ps1
. .\scripts/find-python3.ps1
$env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")
Write-Host "Using python command: $Env:user_python_cmd"
. "$Env:user_python_cmd" "scripts/check-install.py"
