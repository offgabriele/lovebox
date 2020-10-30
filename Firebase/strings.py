import subprocess, os
os.chdir("D:\\Gabriele\\Projects\\Lovebox\\firmware")
command = "wsl strings -n 4 -a firmware.ino.esp32.bin | wsl grep 'MaGiC' "
result = subprocess.check_output(command, shell=True)
result = result.decode("utf-8") 
print(result)
result = result.replace("t", "")