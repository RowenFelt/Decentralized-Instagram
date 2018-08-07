#!/usr/bin/env python3
'''
A file for testing the petepics.py functions on the ellen node
Authors: Rowen Felt and Campbell Boswell
'''

import sys
sys.path.append('./')
import petepics as pp

def main():
    print("Insert ellen as user with user_id 1485, ricker (1435)" +
        " is a follower and following")
    pp.write_user(1485, "ellen", 
        "./nodes/ellen/tests/ellen_user_image",
        "Ellen DeGeneres", 0, [1435], [1435], "140.233.20.153")
    if(pp.search_user(1485)):
        print("SUCCESS: inserted user ellen")
    else:
        print("FAILED: user ellen not found") 
    print("Create post, dispatch id 101")
    pp.write_dispatch("./nodes/ellen/tests/ellen_first_post", "first image",
     1485, [], ["cool shit"], [1435], 0, 1485, 5, 101, "post")
    if(pp.search_dispatch(101)):
        print("SUCCESS: Created post 101")
    else:
        print("FAILED: post 101 not found")
    
    print("Update feed, ellen (1485)")
    pp.update_feed(1485)
    
    print("Search hashtags, ellen (1485), tag 'lol'")
    pp.search_hashtags(1485, "cool shit") #or some other tag that's part of a dispatch
   
    print("View profile, ricker (1435)")
    pp.view_profile(1435)

if __name__ == "__main__":
    main()

