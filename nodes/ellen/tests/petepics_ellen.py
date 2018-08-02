'''
A file for testing the petepics.py functions on the ellen node
Authors: Rowen Felt and Campbell Boswell
'''

import sys
sys.path.append('../../../')
import petepics as pp

def main():
    print("Create post, dispatch id 101")
    pp.write_dispatch("ellen_first_post", "first image",
     1485, [], ["cool shit"], [1435], 0, 1485, 5, 101, "post")
    if(pp.search_dispatch(101)):
        print("SUCCESS: Created post 101")
    else:
        print("FAILED: post 101 not found")
    
    print("Update feed, user 1435")
    pp.update_feed(1435)


if __name__ == "__main__":
    main()

