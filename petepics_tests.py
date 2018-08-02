'''
A file for testing the functionality of the petepics.py functions
Authors: Rowen Felt and Campbell Boswell
'''

import petepics as pp

def main():
    print("Create post, dispatch id 101")
    pp.write_dispatch("pjohnson_fake_path", "first image",
     1485, [], ["cool shit"], [1435], 0, 1485, 5, 101, "post")
    print("Update feed, user 1435")
    pp.update_feed(1435) 


if __name__ == "__main__":
    main()
