# functions for interacting with the 
# user and dispatch mongo collections

import pymongo
import json
from pymongo import MongoClient


def search_user(user_id):
    #searches for a user in insta.user
    client = MongoClient()
    db = client.insta
    collection = db.user
    return (collection.find_one({ "user_id": user_id }))

def decode_following(user):
    decoded_user = user['following']
    return decoded_user['user_ids']    

def search_dispatch(dispatch_id):
    client = MongoClient()
    db = client.insta
    collection = db.dispatch
    return (collection.find_one({ "dispatch_id": dispatch_id }))

