#!/usr/bin/env python3 
''' 
A file for testing the petepics.py functions on the ricker node
Authors: Rowen Felt and Campbell Boswell
'''

import sys
sys.path.append('./')
import petepics as pp

def main():
    print("Insert ricker as user with user_id 1435")
    pp.write_user(1435, "ricker",
        "./nodes/ricker/tests/ricker_first_image",
        "William T. Ricker", 0, [1485], [1485], "140.233.20.181")
   # if(pp.search_user(1435)):
   #     print("SUCCESS: inserted user ricker")
   # else:
   #     print("FAILED: user ricker not found")
    print("Create post, dispatch id 102")
    pp.write_dispatch("./nodes/ricker/tests/ricker_first_post", "ricker's first image",
     1435, [], ["#nohate"], [ ], 0, 1435, 5, 102, "post")
   # if(pp.search_dispatch(102)):
   #     print("SUCCESS: Created post 102")
   # else:
   #     print("FAILED: post 102 not found")
    pp.write_dispatch("./nodes/ricker/tests/ricker_second_post", "Hey, ellen", 1435, [1485], [ ], [ ], 0, 1435, 5, 103, "message")
    # if(pp.search_dispatch(103)):
   #     print("SUCCESS: Created post 103")
   # else:
   #     print("FAILED: post 103 not found")
    print("Update feed, user 1435")
    pp.update_feed(1435)

    print("Search hashtags, user 1435, tag '#nohate'")
    pp.search_hashtags(1435, "#nohate") #or some other tag that's part of a dispatch

    print("Lookup 'ellen'")
    pp.lookup_user("ellen")

if __name__ == "__main__":
    main()

