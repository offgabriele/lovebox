# Python program to convert  
# date to timestamp 
  
  
import time 
import datetime 

import firebase_admin
from firebase_admin import credentials, db, firestore, storage
import os, re, string, random, time
from datetime import datetime


cred = credentials.Certificate("D:\Gabriele\Projects\Lovebox\Firebase\lovebox-offgabriele-firebase-adminsdk-ha0he-e2e8c758f3.json")
firebase_admin.initialize_app(cred, {
    'storageBucket': 'lovebox-offgabriele.appspot.com',
    'databaseURL': 'https://lovebox-offgabriele.firebaseio.com/'
})

ref = db.reference('messages')
f = ref.get()
print(f)
for key in f:
    #print(key)
    if key == "latest":
        timedata = f["latest"]["timesent"]
        timestamp = time.mktime(datetime.strptime(timedata, 
                                                 "%Y/%m/%d %H:%M:%S").timetuple())
    else:
        timestamp = time.mktime(datetime.strptime(key, 
                                                 "%Y %m %d %H:%M:%S").timetuple()) 
    ref = db.reference('messages/' +key)
    #print(ref.get())
    ref.update({
    'timestamp': timestamp
    })