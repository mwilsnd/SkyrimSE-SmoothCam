import argparse
import os
import shutil
import subprocess
import asyncio

args = argparse.ArgumentParser()
args.add_argument("--skyrim-tools-dir", type=str, default="../SkyrimTools", help="Directory containing 'Papyrus Compiler/' and 'Data/'")
args.add_argument("--dont-clean", action="store_true", help="Don't clean build/package artifacts")
args.add_argument("--skip-buck-build", action="store_true", help="Skip buck2 build invocations")
args.add_argument("--installed", type=str, default="", help="Components already installed by this script in a prior invocation")
args = args.parse_args()

papyrus_compiler = args.skyrim_tools_dir + "/Papyrus_Compiler/PapyrusCompiler.exe"
papyrus_scripts_folder = args.skyrim_tools_dir + "/Data/Scripts/Source"

async def run_subcmd(icon, cmds, cwd=None, pipe=True):
    print(icon + " | " + cmds[0] + ":")

    if pipe == True:
        proc = await asyncio.create_subprocess_shell(" ".join(cmds), stdout=asyncio.subprocess.PIPE, stderr=asyncio.subprocess.STDOUT, cwd=cwd)
        while proc.returncode == None and not proc.stdout.at_eof():
            line = await asyncio.wait_for(proc.stdout.readline(), 100)
            if line:
                print("   : ", line.decode("utf-8").strip())
        check_fail_code(await proc.wait())
    else:
        return subprocess.run(cmds, cwd=cwd)

"""
After installing something which mutates PATH, we have to restart
to get the latest environment.
"""
def restart(withArgs):
    subprocess.run([os.environ["user_python_cmd"], "scripts/check-install.py"] + withArgs)
    exit()

def stop(msg):
    print("🛑 | " + msg)
    exit()

def check_fail_code(res):
    if isinstance(res, int):
        if res != 0:
            print("💀 | Command failed, aborting")
            exit()
    else:
        if res.returncode != 0:
            print("💀 | Command failed, aborting")
            exit()

async def test_try_install(nice_name, cmds, installScripts):
    print(f"🧪 | Looking for {nice_name}...")
    missing = False

    for cmd in cmds:
        if shutil.which(cmd) == None:
            missing = True
            break
    
    if missing:
        print(f"❗ | Unable to find {nice_name}, trying to install it...")
        for script in installScripts:
            await run_subcmd("📦", ["powershell.exe", "-executionpolicy", "bypass", "-File", script])

        if cmds[0] in args.installed:
            stop("Already attempted to install " + cmds[0] + ", aborting")
        else:
            print(" ❔ | Trying again... (restarting script)")
            restart(["--installed", args.installed + ":" + cmds[0]])

async def run():
    print("📣 | Looking for required tools...")

    await test_try_install("7-zip", ["7z"], ["scripts/install-7z.ps1"])
    await test_try_install("D toolchain (dub, ldc2)", ["dub", "ldc2"], ["scripts/install-ldc.ps1"])

    print("🧪 | Looking for buck2...")
    buck2 = "buck2"
    if shutil.which(buck2) == None:
        buck2 = os.path.abspath("buck2/buck2.exe")
        print("❔ | Trying local copy...")
        if shutil.which(buck2) == None:
            print("❗ | Unable to find buck2, installing a local copy...")
            await test_try_install("zstandard", ["zstd"], ["scripts/install-zstd.ps1"])
            await run_subcmd("🌐", ["powershell.exe", "-executionpolicy", "bypass", "-File", "scripts/fetch-buck.ps1"])
            print("❔ | Trying local copy...")
            if shutil.which(buck2) == None:
                stop("Unable to find buck2, aborting")

    print("🔨 | Build tools configured! You can now try running package.py.")

loop = asyncio.get_event_loop()
loop.run_until_complete(asyncio.wait([
    loop.create_task(run()),
]))
loop.close()
