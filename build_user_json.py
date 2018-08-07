#!/usr/bin/env python3

import pymongo
import bson
import datetime
import base64
from bson.objectid import ObjectId


def build_user(user):
    '''    
    Dictionary user has the following key value pairs:
       user_id - an 64bit int representation of the user's id
       user_name - a string representation of the user's name 
       image_path - the path to a user icon/image file
       name - a string of the user's "real" name
       fragmentation
       followers - an array of user ids of followers
       following - an array of user ids of following
    '''

    mongo_id = (' "_id" : { "$oid" : "' + str(ObjectId()) + '" }, ')
  
    user_id = ('"user_id" : { "$numberLong" : "' + str(user['user_id']) + '" }, ')

    user_name = ('"username" : "' + user['username'] + '", ')

    #open the media specified by media path, read its contents to a buffer as
    #binary, and store its size
    with open(user['image_path'], 'r+b') as f:
        image = f.read()
        image_length = len(image) 
        image  = ('"image_length" : { "$numberInt" : "' + str(image_length) +
                '" }, "image" : { "$binary" : { "base64": "' + 
                str(base64.b64encode(image))[2:-1] + 
                '", "subType" : "00" } }, ')

    curent_time = int(round(datetime.datetime.utcnow().timestamp() * 1000))
    bio = ('"bio" : { "name" : "' + user['name'] + '", "date_created" : { ' +
           '"$date" : { "$numberLong" : "' + str(curent_time) + '" } }, ' +
           '"date_modified" : { "$date" : { "$numberLong" : "' + str(curent_time) + 
           '" } } }, ')

    fragmentation = ('"fragmentation" : { "$numberInt" : "' + str(user['fragmentation'])
                    + '" }, ')


    followers = ('"followers" : { "direction" : { "$numberInt" : "0" }, "count" : ' +
                 '{ "$numberInt" : "' + str(len(user['followers'])) + '" }, ' +
                 '"user_ids" : [ ')
    if(len(user['followers']) == 0):
        followers = followers + '] }, '
    else:
        for i in user['followers']:
            followers = followers + '{ "$numberLong" : "' + str(i) + '"}, '
        #remove the trailing comma from the last audience id
        followers = followers[:-2] + ' ] }, '
    

    following = ('"following" : { "direction" : { "$numberInt" : "1" }, "count" : ' +
                 '{ "$numberInt" : "' + str(len(user['following'])) + '" }, ' +
                 '"user_ids" : [ ')
    if(len(user['following']) == 0):
        following = following + '] }, '
    else:
        for i in user['following']:
            following = following + '{ "$numberLong" : "' + str(i) + '" }, '
        #remove the trailing comma from the last audience id
        following = following[:-2] + ' ] } '

    
    json_string = ('{' + mongo_id + user_id + user_name + image + bio + 
                   fragmentation + followers + following + '}')
    print(json_string)   
    return json_string
