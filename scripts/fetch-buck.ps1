New-Item -ItemType Directory -Force -Path buck2/bootstrap
Invoke-WebRequest "https://github.com/facebook/buck2/releases/download/2024-03-15/buck2-x86_64-pc-windows-msvc.exe.zst" -OutFile "buck2/bootstrap/buck2-x86_64-pc-windows-msvc.exe.zst"
zstd -d buck2/bootstrap/buck2-x86_64-pc-windows-msvc.exe.zst -o buck2/buck2.exe
