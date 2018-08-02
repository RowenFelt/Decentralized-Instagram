# functions for interacting with the 
# user and dispatch mongo collections

import pymongo
import json
import build_dispatch_json as bdj
from pymongo import MongoClient
from subprocess import call
import sys

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

def write_dispatch(media_path, body_text, user_id, audience, tags,
    user_tags, parent_type, parent_id, fragmentation, dispatch_id, 
    dispatch_type):
    dispatch_dict = {"media_path": media_path, "body_text": body_text,
        "user_id": user_id, "audience": audience, "tags": tags, 
        "user_tags": user_tags, "parent_type": parent_type, 
        "parent_id": parent_id, "fragmentation": fragmentation,
        "dispatch_id": dispatch_id}
    dispatch_json = bdj.build_dispatch(dispatch_dict)
    command = create_dispatch_command(dispatch_type, dispatch_json)
    file = open("new_dispatch.txt", "w")
    file.write(command)
    file.close()
    notified_users = audience + user_tags
    notified_users.append(user_id)
    for i in notified_users:
        client_command = ["./client", str(i), "3999", "new_dispatch.txt"]
        call(client_command)
    return call(["rm", "new_post.txt"])     

def create_dispatch_command(dispatch_type, dispatch_json):
    # no real difference between a comment and a post
    if(dispatch_type == "post"):
        return ("push dispatch " + dispatch_json)
    elif(dispatch_type == "comment"):
        return ("push child*** " + dispatch_json)
    elif(dispatch_type == "message"):
        return ("push message* " + dispatch_json)
    elif(dispatch_type == "user_tag"):
        return ("push user_tag " + dispatch_json)
    else:
        print("INVALID DISPATCH TYPE")
        sys.exit()

def update_feed(user_id):
    user = search_user(user_id)
    following = decode_following(user)
    for i in following:
        command = "pull all***** " + i
        file = open("update_feed.txt", "w")
        file.write(command)
        file.close()
        client_command = ["./client", i, "3999", "update_feed.txt"]
        call(client_command)
    return call(["rm", "update_feed.txt"])
