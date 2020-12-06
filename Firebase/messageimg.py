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

ref = db.reference('messages/latest')
f = ref.get()
print(f, type(f))
ref = db.reference('messages')
try:
    name = f['timesent']
except:
    name = f['time']
name = name.replace('/', ' ')
#name = name.replace(':', ' ')
user_ref = ref.child(name)
if f['displayed'] == "false":
    f['timedisplayed'] = "not displayed"
if f['read'] == "false":
    f['timeread'] = "not read"
user_ref.set(f)
now = datetime.now()

filename = 'Giulietto.jpg'
newfilepath = "D:\\Gabriele\\Projects\\Lovebox\\firmware\\data\\Giulietto.jpg"

bd = firestore.client()
bucket = storage.bucket()
blob = bucket.blob(filename)
blob.upload_from_filename(newfilepath)
blob.make_public()
url = blob.public_url

filename = "/" + filename

users_ref = ref.child('latest')
users_ref.set({
    'type': 'image',
    'url' : url,
    'name' : filename,
    'timesent': now.strftime("%Y/%m/%d %H:%M:%S"),
    'displayed' : "false",
    'read' : "false",
    'datetime read' : None,
})