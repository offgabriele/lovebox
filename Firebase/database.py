from firebase import firebase
import firebase_admin
from firebase_admin import credentials, db, firestore, storage
import os, re, string, random, time, subprocess
from datetime import datetime

alphabet = string.ascii_lowercase + string.digits
id = ''.join(random.choices(alphabet, k=6))

os.chdir("D:\\Gabriele\\Projects\\Lovebox\\firmware")
oldfilepath ="D:\\Gabriele\\Projects\\Lovebox\\firmware\\firmware.ino.esp32.bin"
os.chdir("D:\\Gabriele\\Projects\\Lovebox\\firmware")
command = "wsl strings -n 4 -a firmware.ino.esp32.bin | wsl grep 'LoVeBoX' "
#result = subprocess.check_output(command, shell=True)
#result = result.decode("utf-8") 
#print(result)
#result = result.replace("t", "")
#version = str(result)
stamp = os.path.getmtime('firmware.ino.esp32.bin')
stamp = datetime.fromtimestamp(stamp)
stamp = stamp.strftime("%m/%d/%Y %H:%M:%S")
filename = "lovebox({}).bin".format(id)
newfilepath = "D:\\Gabriele\\Projects\\Lovebox\\firmware\\{}".format(filename)
os.rename(oldfilepath, newfilepath)
cred = credentials.Certificate("D:\Gabriele\Projects\Lovebox\Firebase\lovebox-offgabriele-firebase-adminsdk-ha0he-e2e8c758f3.json")
firebase_admin.initialize_app(cred, {
    'storageBucket': 'lovebox-offgabriele.appspot.com',
    'databaseURL': 'https://lovebox-offgabriele.firebaseio.com/'
})
bd = firestore.client()
bucket = storage.bucket()
blob = bucket.blob(filename)
blob.upload_from_filename(newfilepath)
blob.make_public()
url = blob.public_url
version = input("version: ")
ref = db.reference('firmware')
users_ref = ref.child(version)
users_ref.set({
    'filename' : filename,
    'creation' : stamp,
    'version' : version,
    'url' : url,
})

ref = db.reference('config')
users_ref = ref.child('24:6F:28:B3:58:50')
users_ref.set({
    'version' : version,
})