#!/usr/bin/env python3 
''' 
A file for testing the petepics.py functions on the equinox node
Authors: Rowen Felt and Campbell Boswell
'''

import sys
sys.path.append('./')
import petepics as pp

def main():
    print("Insert equinox as user with user_id 1475")
    pp.write_user(1475, "equinox",
        "./nodes/equinox/tests/equinox_first_image",
        "Vernal Equinox", 0, [ ], [1435, 1485], "140.233.20.154")
   # if(pp.search_user(1475)):
   #     print("SUCCESS: inserted user equinox")
   # else:
   #     print("FAILED: user equinox not found")
    print("Create post, dispatch id 105")
    pp.write_dispatch("./nodes/equinox/tests/equinox_first_post", 
    "equinox's first image", 1475, [], ["#stillsummer"], 
    [ ], 0, 1475, 5, 105, "post")
   # if(pp.search_dispatch(105)):
   #     print("SUCCESS: Created post 105")
   # else:
   #     print("FAILED: post 105 not found")
    pp.write_dispatch("./nodes/equinox/tests/equinox_second_post", "Our first group message", 1475, [1485, 1435], [ ], [ ], 0, 1475, 5, 106, "message")
    # if(pp.search_dispatch(106)):
   #     print("SUCCESS: Created post 106")
   # else:
   #     print("FAILED: post 106 not found")
    print("Update feed, user 1475")
    pp.update_feed(1475)
    
    print("Search hashtags, user 1475, tag '#nohate'")
    pp.search_hashtags(1475, "#nohate") #or some other tag that's part of a dispatch

    print("Lookup 'ricker'")
    pp.lookup_user("ricker")

if __name__ == "__main__":
    main()

