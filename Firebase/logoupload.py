from firebase import firebase
import firebase_admin
from firebase_admin import credentials, db, firestore, storage
import os, re, string, random, time
from datetime import datetime
cred = credentials.Certificate("D:\Gabriele\Projects\Lovebox\Firebase\lovebox-offgabriele-firebase-adminsdk-ha0he-e2e8c758f3.json")
firebase_admin.initialize_app(cred, {
    'storageBucket': 'lovebox-offgabriele.appspot.com',
    'databaseURL': 'https://lovebox-offgabriele.firebaseio.com/'
})

filename = "heart.jpg"
newfilepath = "C:\\Users\\gabri\\Documents\\Arduino\\libraries\\TFT_eFEX\\examples\\Jpeg_ESP32\\data\\heart.jpg"
bd = firestore.client()
bucket = storage.bucket()
blob = bucket.blob(filename)
blob.upload_from_filename(newfilepath)
blob.make_public()
url = blob.public_url
print(url)