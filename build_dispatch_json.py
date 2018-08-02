import pymongo
import time
import datetime
import bson
import base64
from bson.objectid import ObjectId


#       TODO: verify that numberLong and numberInt's linup properly
#       fix mongoid using ObjectId function if possible
#       find a way to encode media as binary without converting to a string
#       find a way to encode timestamps as the integer values returned by the datetime
#       repeat the process for the user
def build_dispatch(dis):
        
    #Dictionary dis has the following key value pairs:
    #   media_path - the path to an image file for which the user has read permissions 
    #   body_text - a caption for the media 
    #   user_id - the user's id
    #   audience - the audience of the the post as an array of user_id's
    #   tags - any hashtags for the post, as an array of strings
    #   user_tags - an array of user ids
    #   parent_type - the type of the object the parent id references 
    #   parent_id - either a user or dispatch id
    #   fragmentation  
    #   dispatch_id
    
    #bson.objectid.ObjectId(str(dis['user_id']) + str(dis['dispatch_id'])) 
    mongo_id = (' "_id" : { "$oid" : "' + str(ObjectId()) + '" }, ')
  
    #open the media specified by media path, read its contents to a buffer as
    #binary, and store its size
    with open(dis['media_path'], 'r+b') as f:
        media = f.read()
        media_size = len(media) 
        body = ('"body" : { "media_size" : { "$numberInt" : "' + str(media_size) +
                '" }, "media" : { "$binary" : { "base64": "' + 
                str(base64.b64encode(media))[2:-1] + 
                '", "subType" : "00" } }, "text" : "' + dis['body_text'] +'" }, ')

    user_id = ('"user_id" : { "$numberLong" : "' + str(dis['user_id']) + '"}, ')
   

    #generate timestamp and store as  
    #curent_time = datetime.datetime.utcnow()
    curent_time = int(round(datetime.datetime.utcnow().timestamp() * 1000))
    timestamp = ('"timestamp" : { "$date" : { "$numberLong" : "' + 
                str(curent_time) + '" } }, ')


    audience = ('"audience_size" : { "$numberInt" : "' + str(len(dis['audience'])) +  
                '" }, "audience" : [ ')
    if(len(dis['audience']) == 0):
        audience = audience + ' ], '
    else:
        for i in dis['audience']:
            audience = audience + '{ "$numberLong" : "' + str(i) + '" }, '
        #remove the trailing comma from the last audience id
        audience = audience[:-2] + ' ], '

    
    tags = ('"num_tags" : { "$numberInt" : "' + str(len(dis['tags'])) +
                 '" }, "tags" : [ ')
    if(len(dis['tags']) == 0):
        tags = tags + ' ], '
    else:
        for i in dis['tags']:
            tags = tags + ' "' + str(i) + '", '
        tags = tags[:-2] + ' ], ' 

    
    user_tags = ('"num_user_tags" : { "$numberInt" : "' + str(len(dis['user_tags'])) +
                 '" }, "user_tags" : [ ')
    if(len(dis['user_tags']) == 0):
        user_tags = user_tags + ' ], '
    else:
        for i in dis['user_tags']:
            user_tags = user_tags + '{ "$numberLong" : "' + str(i) + '" }, '
        user_tags = user_tags[:-2] + '], ' 


    #all dispatches are posts with the user as the parent
    parent = ('"dispatch_parent" : { "type" : { "$numberInt" : "' +
              str(dis['parent_type']) + '" }, "id" : { "$numberLong" : "' +
              str(dis['parent_id']) + '"} }, ')
    fragmentation = ('"fragmentation" : { "$numberInt" : "' + str(dis['fragmentation'])
                    + '" }, ')
    dispatch_id = ('"dispatch_id" : { "$numberLong" : "' + str(dis['dispatch_id']) + 
                   '"} ')
    
    json_string = ('{' + mongo_id + body + user_id + timestamp + audience + tags 
            + user_tags +parent + fragmentation + dispatch_id + '}')
   
    return json_string
