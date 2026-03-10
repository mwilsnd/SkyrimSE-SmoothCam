import argparse
import os
import shutil
import subprocess
import asyncio
import sys
from pathlib import WindowsPath

loop = asyncio.get_event_loop()
args = argparse.ArgumentParser()
args.add_argument("--skyrim-tools-dir", type=str, default="../SkyrimTools", help="Directory containing 'Papyrus Compiler/' and 'Data/'")
args.add_argument("--dont-clean", action="store_true", help="Don't clean build/package artifacts")
args.add_argument("--skip-buck-build", action="store_true", help="Skip buck2 build invocations")
args = args.parse_args()

papyrus_compiler = os.path.join(args.skyrim_tools_dir, "Papyrus Compiler/PapyrusCompiler.exe")
papyrus_scripts_folder = os.path.join(args.skyrim_tools_dir, "Data/Scripts/Source")
python_bin = sys.executable

async def run_subcmd(icon, cmds, cwd=None, pipe=True, returnPipe=False):
    print(icon + " | " + cmds[0] + ":")

    if pipe == True:
        proc = await asyncio.create_subprocess_shell(" ".join(cmds), stdout=asyncio.subprocess.PIPE, stderr=asyncio.subprocess.STDOUT, cwd=cwd)
        output = []
        
        while proc.returncode == None and not proc.stdout.at_eof():
            line = await asyncio.wait_for(proc.stdout.readline(), 100)
            if line:
                if returnPipe:
                    output.append(line.decode("utf-8").strip())
                else:
                    print("   : ", line.decode("utf-8").strip())
        check_fail_code(await proc.wait())
        if returnPipe:
            return output
    else:
        return subprocess.run(cmds, cwd=cwd)

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

def test_installed(nice_name, cmds, installScripts):
    print(f"🧪 | Looking for {nice_name}...")
    missing = False

    for cmd in cmds:
        if shutil.which(cmd) == None:
            missing = True
            break
    
    if missing:
        stop(f"Unable to find {nice_name}, please install it or run the bootstrap script")

async def getArtifactLocation(buck2, target):
    buildPath = await run_subcmd("🦌", [
        buck2, "targets "
        "--config", f"build.python_interpreter={python_bin}",
        "--config-file", "buck2/mode/release",
        "--show-output", target
    ],  returnPipe=True)
    return WindowsPath(buildPath[-1].split()[1]).parents[0]

async def run():
    print("📣 | This might take a while! Destination: ./SmoothCam.zip")
    print(f"📂 | Skyrim tools: {args.skyrim_tools_dir}")
    print(f"📂 | Papyrus compiler: {papyrus_compiler}")
    print(f"📂 | Papyrus scripts:{papyrus_scripts_folder}")
    
    # Check our environment
    print("🧪 | Looking for papyrus compiler...")
    if not os.path.exists(papyrus_compiler):
        return stop("Unable to find papyrus compiler, aborting")

    # We can try and install some stuff if missing
    test_installed("7-zip", ["7z"], ["scripts/install-7z.ps1"])
    test_installed("D toolchain (dub, ldc2)", ["dub", "ldc2"], ["scripts/install-ldc.ps1"])

    print("🧪 | Looking for buck2...")
    buck2 = "buck2"
    if shutil.which(buck2) == None:
        buck2 = os.path.abspath("buck2/buck2.exe")
        print("❔ | Trying local copy...")
        if shutil.which(buck2) == None:
            return stop("Unable to find buck2, please install it or run the bootstrap script")

    if not args.dont_clean:
        print("🧹 | Cleaning up old build artifacts...")
        
        if os.path.exists("Release_Package"):
            shutil.rmtree("Release_Package")

        if os.path.exists("build-out"):
            shutil.rmtree("build-out")

        await run_subcmd("🔨", ["dub", "clean", "--all-packages"], cwd="CodeGen/MCM/paper")
        await run_subcmd("🦌", [buck2, "clean"])

    if not os.path.exists("Release_Package"):
        print("📦 | Creating new release package...")
        shutil.copytree("Package", "Release_Package")
        
    if not os.path.exists("build-out"):
        os.makedirs("build-out")

    print("📜 | Building paper")
    await run_subcmd("🔨", ["dub", "build", "-b", "release", "--c", "application-release", "--force"], cwd="CodeGen/MCM/paper")

    print("📜 | Generating MCM script...")
    await run_subcmd("📜", [
        "\"CodeGen/MCM/paper/paper.exe\"",
        "\"CodeGen/MCM/mcm/mcm.psc\"",
        "\"../../../Release_Package/00 Data/SmoothCamMCM.psc\""
    ])

    print("🚽 | Compiling generated papyrus...")
    shutil.copyfile("Release_Package/00 Data/SmoothCamMCM.psc", os.path.join(papyrus_scripts_folder, "SmoothCamMCM.psc"))
    await run_subcmd("💩", [
        "\"" + papyrus_compiler + "\"",
        "SmoothCamMCM.psc",
        f"-f=\"{os.path.join(papyrus_scripts_folder, "TESV_Papyrus_Flags.flg")}\"",
        f"-i=\"{papyrus_scripts_folder}\"",
        "-o=\"Release_Package/00 Data\""
    ])

    if not args.skip_buck_build:
        print("🎥 | Building SmoothCam AE,Pre629")
        code = await run_subcmd("🦌", [
            buck2, "build",
            "--config", f"build.python_interpreter={python_bin}",
            "--out", "build-out/",
            "--config-file", "buck2/mode/release_pre629",
            ":SmoothCamAE"
        ], pipe=False)
        check_fail_code(code)

        print("🎥 | Building SmoothCam AE")
        code = await run_subcmd("🦌", [
            buck2, "build",
            "--config", f"build.python_interpreter={python_bin}",
            "--out", "build-out/",
            "--config-file", "buck2/mode/release",
            ":SmoothCamAE"
        ], pipe=False)
        check_fail_code(code)

        print("🎥 | Building SmoothCam SSE")
        code = await run_subcmd("🦌", [
            buck2, "build",
            "--config", f"build.python_interpreter={python_bin}",
            "--out", "build-out/",
            "--config-file", "buck2/mode/release",
            ":SmoothCamSSE"
        ], pipe=False)
        check_fail_code(code)

    aeLocation = await getArtifactLocation(buck2, ":SmoothCamAE")
    sseLocation = await getArtifactLocation(buck2, ":SmoothCamSSE")

    print("📑 | Copying artifacts to package...")
    shutil.copyfile("build-out/SmoothCamAE.dll", "Release_Package/00 Data/AE/SmoothCam.dll")
    shutil.copyfile(aeLocation / "SmoothCamAE.pdb", "Release_Package/00 Data/AE/SmoothCam.pdb")

    shutil.copyfile("build-out/SmoothCamAEPre629.dll", "Release_Package/00 Data/AE-Pre629/SmoothCam.dll")
    shutil.copyfile(aeLocation / "SmoothCamAEPre629.pdb", "Release_Package/00 Data/AE-Pre629/SmoothCam.pdb")

    shutil.copyfile("build-out/SmoothCamSSE.dll", "Release_Package/00 Data/SSE/SmoothCam.dll")
    shutil.copyfile(sseLocation / "SmoothCamSSE.pdb", "Release_Package/00 Data/SSE/SmoothCam.pdb")
    
    os.remove("Release_Package/00 Data/AE/placeholder")
    os.remove("Release_Package/00 Data/AE-Pre629/placeholder")
    os.remove("Release_Package/00 Data/SSE/placeholder")
    os.remove("Release_Package/00 Data/SmoothCamMCM.psc")

    print("📚 | Compressing package...")
    await run_subcmd("📗", ["7z", "a", "-tzip", "SmoothCam.zip", "Release_Package"])

    print("🍾 | Packaging completed!")

loop.run_until_complete(asyncio.wait([
    loop.create_task(run()),
]))
loop.close()
