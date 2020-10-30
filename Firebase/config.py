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

ref = db.reference('config')
users_ref = ref.child('AC:67:B2:07:F8:CC')
users_ref.set({
    'version' : '0-0-1'
})