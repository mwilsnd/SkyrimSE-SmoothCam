function Try-Command {
    Try {
        return (& "$args" "--version")
    } Catch {
        Write-Host $_
        return $false
    }
}

if (Try-Command python) {
    $Env:user_python_cmd = "python"
} elseif (Try-Command python3) {
    $Env:user_python_cmd = "python3"
} elseif (Try-Command py) {
    $Env:user_python_cmd = "py"
} else {
    throw "Unable to locate python 3 command!"
}
