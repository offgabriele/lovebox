import os
os.chdir('D:\\Gabriele\\Projects\\Lovebox\\firmware\\')
print(os.listdir(os.getcwd()))
with open('version.h', 'r') as f:
    g = f.read()
version = g[33:40]
