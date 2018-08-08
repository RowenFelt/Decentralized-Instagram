#!/usr/bin/env python3
'''
function that reads a csv of 'user actions'
and the data necessary to complete them (i.e.
the information which defines a disapatch),
then calls the corresponding functions for 
these actions in petepics.py
'''
import sys
import petepics as pp
import csv

def main():
    #argument checking
    if len(sys.argv) != 2:
        print("usage: takes the filepath of a csv file")
        return None



    with open(sys.argv[1], 'r') as csv_file:
        reader = csv.reader(csv_file, delimiter=',')
        for row in reader:
            num_fields = len(row) - 1

            if num_fields == 0:
                print("error, empty line in csv")
                return None
            #pullout the first string in the row, which specifies
            #which user story the data in the row describe
            action = row[0]

            if action == "message":
                #11 is the number of arguments needed for a dispatch json
                if num_fields != 11: 
                    print("error, too few args for dispatch command")
                    return None
                
                media_path = row[1]
                body_text = row[2]
                user_id = row[3]
                audience = row[4].split()
                tags = row[5].split()
                user_tags = row[6].split()
                parent_type = row[7]
                parent_id  = row[8]
                fragmentation = row[9]
                dispatch_id = row[10]
                dispatch_type = row[11] 
                pp.write_dispatch(media_path, body_text, user_id, audience,
                    tags, user_tags, parent_type, parent_id, fragmentation, 
                    dispatch_id, dispatch_type)
    
            elif action == "user": 
                #7 is the number of arguments needed for a user json
                if num_fields != 8: 
                    print("error, too few args for user command")
                    return None

                user_id = row[1]
                username = row[2] 
                image_path = row[3] 
                name = row[4] 
                fragmentation = row[5] 
                followers = row[6].split()
                following = row[7].split()
                ip = str(row[8]).strip(' ')
                pp.write_user(user_id, username, image_path, name, 
                    fragmentation, followers, following, ip)

            elif action == "feed":
                #1 is the number of arguments needed to update a user's feed
                if num_fields != 1:
                    print("error, too few args for update_feed command")
                    return None

                user_id = row[1]
                pp.update_feed(user_id)

            elif action == "profile": 
                #1 is the number of arguments needed to pull a user's profile
                if num_fields != 1:
                    print("error, too few args for view_profile command")
                    return None

                user_id = row[1]
                pp.view_profile(user_id)

            elif action == "search_tags":
                #2 is the number of arguments for search hashtags
                if num_fields != 2:
                    print("error, too few args for search_hashtags")
                    return None

                user_id = row[1]
                tag = row[2]
                pp.search_hashtags(user_id, tag)

            elif action == "lookup":
                #1 is the number of arguments for search user
                if num_fields != 1:
                    print("error, too few args for find_user")
                    return None
                
                username = row[1]
                pp.lookup_user(username)


if __name__ == "__main__":
    main()
