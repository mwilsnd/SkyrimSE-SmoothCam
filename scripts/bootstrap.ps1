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

powershell.exe -File "scripts/install-vsbuild-tools.ps1" -NoNewWindow -Wait
powershell.exe -File "scripts/install-python3.ps1" -NoNewWindow -Wait
$env:Path = [System.Environment]::GetEnvironmentVariable("Path","Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path","User")
python scripts/check-install.py
