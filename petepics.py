#!/usr/bin/env python3
'''
functions that represent user actions and
associated helper methods.
These functions must be run from nodes/<node>/tests directory!
Authors: Rowen Felt and Campbell Boswell
'''

import pymongo
import json
import build_dispatch_json as bdj
import build_user_json as buj
from pymongo import MongoClient
from subprocess import call
import sys

def search_user(user_id):
    '''
    searches the mongo insta.user collection for a 
    user with match user_id, returns json object
    '''
    client = MongoClient()
    db = client.insta
    collection = db.user
    return (collection.find_one({ "user_id": user_id }))

def decode_following(user):
    '''
    Given a user object as dictionary, returns array
    of following user_ids
    '''
    decoded_user = user['following']
    return decoded_user['user_ids']    

def search_dispatch(dispatch_id):
    '''
    searches the mongo insta.dispatch collection for a 
    dispatch with matching dispatch_id, returns json object
    '''
    client = MongoClient()
    db = client.insta
    collection = db.dispatch
    return (collection.find_one({ "dispatch_id": dispatch_id }))

def lookup_user(username):
    '''
    searches the cassandra hashtable for users with a
    given name, creates pull user**** commands for each
    returned user_id, and executes the pull command in
    a client process. We wrote this as a c program because
    it was easier to communicate with cassandra
    '''
    call(["./search_user", username])

def write_dispatch(media_path, body_text, user_id, audience, tags,
    user_tags, parent_type, parent_id, fragmentation, dispatch_id, 
    dispatch_type):
    ''' 
    creates a dispatch json object from the provided fields,
    creates a file containing the relevant network protocol 
    (using the dispatch_type field (see create_dispatch_command),
    opens a client connection to the home user and each user 
    specified in audience and user_tags, and sends the command     
    '''
    dispatch_dict = {"media_path": media_path, "body_text": body_text,
        "user_id": user_id, "audience": audience, "tags": tags, 
        "user_tags": user_tags, "parent_type": parent_type, 
        "parent_id": parent_id, "fragmentation": fragmentation,
        "dispatch_id": dispatch_id}
    dispatch_json = bdj.build_dispatch(dispatch_dict)
    if(dispatch_json == None):
        print("build_dispatch() failed")
        return None
    command = create_dispatch_command(dispatch_type, dispatch_json)
    file = open("new_dispatch.txt", "w")
    file.write(command)
    file.close()
    notified_users = audience + user_tags
    notified_users.append(user_id)
    for i in notified_users:
        client_command = ["./client", str(i), 
                "3999", "new_dispatch.txt"]
        call(client_command)
    return call(["rm", "new_dispatch.txt"])     

def create_dispatch_command(dispatch_type, dispatch_json):
    '''
    creates the networking protocol string based on the
    dispatch type:
    post: push dispatch
    comment: push child***
    message: push message*
    user_tag: push user_tag
    '''
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

def write_user(user_id, username, image_path, name, fragmentation, 
        followers, following):
    '''
    creates or updates a user object from the provided fields
    '''
    user_dict = {"user_id": user_id, "username": username, 
        "image_path": image_path, "name": name, 
        "fragmentation": fragmentation, "followers": followers, 
        "following": following}
    user_json = buj.build_user(user_dict)
    if(user_json == None):
        print("build_user() failed")
        return None
    command = "push user**** " + user_json
    file = open("new_user.txt", "w")
    file.write(command)
    file.close()
    client_command = ["./client", str(user_id), "3999", "new_user.txt"]
    call(client_command)
    return call(["rm", "new_user.txt"])

def update_feed(user_id):
    '''
    updates the feed of user with user_id by first pulling the 
    given user object from the mongo insta.user collection, and 
    then calling the "pull all*****" network protcol on each 
    user_id in the user's "following" array
    '''
    user = search_user(user_id)
    following = decode_following(user)
    for i in following:
        command = "pull all***** " + str(i)
        file = open("update_feed.txt", "w")
        file.write(command)
        file.close()
        client_command = ["./client", i, "3999", "update_feed.txt"]
        call(client_command)
    return call(["rm", "update_feed.txt"])

def search_hashtags(user_id, tag):
    '''
    allows a user to seach the dispatches of the individuals they
    follow for instances of the specified hashtag which are 
    included in the dispatch definitions. This is achieved by using
    the "pull tags**** <tag>" command across all users in the 
    primary user's follow list
    '''
    user = search_user(user_id)
    following = decode_following(user)
    for i in following:
        command = "pull tags**** " + tag
        file = open("search_hashtags.txt", "w")
        file.write(command)
        file.close()
        client_command = ["./client", i, "3999", "search_hashtags.txt"]
        call(client_command)
    return call(["rm", "search_hashtags.txt"])

def view_profile(user_id):
    '''
    pulls all dispatches for which a given user is the owner, as well
    as all of the dispatches that a given user is tagged in. This 
    requires issuing a "pull child*** <user_id>" command, as well as
    a "pull user_tags <user_id>" command.
    '''
    user = search_user(user_id)
    command_file = "view_profile.txt"
    commands = ["pull child*** ", "pull user_tags "]
    client_command = ["./client", i, "3999", "view_profile.txt"]

    for i in range(len(commands)):
        file = open(command_file, "w")
        file.write(command[i] + str(user_id))
        file.close()
        call(client_command)

    return call(["rm", command_file])
        
