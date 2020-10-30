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
name = f['time']
name = name.replace('/', ' ')
#name = name.replace(':', ' ')
user_ref = ref.child(name)
user_ref.set(f)
message = input("Insert message: ")
now = datetime.now()

users_ref = ref.child('latest')
users_ref.set({
    'message' : message,
    'time': now.strftime("%m/%d/%Y %H:%M:%S"),
    'displayed' : "false",
    'read' : "false",
    'datetime read' : None,
})