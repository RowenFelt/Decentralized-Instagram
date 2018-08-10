#!/usr/bin/env python3

from csv import writer 
from numpy.random import choice
import random

def create_csv(user, lines, post_ids, usernames):
    '''
    takes a dictionary representing a user object and 
    populates a csv with random user actions, starting
    with creating a user object, all dispatches and
    user object updates will use the same image path,
    post_ids is an accumulating list of all posts, and
    usernames is a static list of all usernames in the experiment set
    '''
    filename = user['username'] + '_actions.csv'
    f = open(filename, "w", newline='')
    
    #first write user row
    followers = ''
    if(len(user['followers']) > 0):
        for i in user['followers']:
            followers = followers + ' ' + str(i)
    following = ''
    if(len(user['following']) > 0):
        for i in user['following']:
            following = following + ' ' + str(i)
    csvfile = writer(f, delimiter= ',') 
    csvfile.writerow(['user', user['user_id'], user['username'], user['image_path'],
        user['name'], str(user['fragmentation']), followers, 
        following, user['ip']])

    options = ['message', 'feed', 'profile', 'search_tags', 'lookup']
    probability = [.1, .7, .1, .05, .05,]
    overlap = set(user['followers']).intersection(set(user['following']))
    words_file = open('common_words', 'r')
    words = words_file.readlines()
    words = [line.strip() for line in words]
    for i in range(0,lines):
        draw = choice(options, p=probability)
        if draw == 'message':
            write_message_row(csvfile, user, overlap, words, post_ids)
        elif draw == 'feed':
            write_feed_row(csvfile, user)
        elif draw == 'profile':
            write_profile_row(csvfile, user)  
        elif draw == 'search_tags':
            write_search_tags_row(csvfile, user, words)
        elif draw == 'lookup':
            write_lookup_row(csvfile, user, usernames) 

def write_message_row(csvfile, user, overlap, words, post_ids):  
    '''
    writes a row to the csv representing a dispatch,
    randomly picks between "types", chooses random tags
    from "words" for posts, generates random dispatch_ids,
    selects random groups or individuals for messages,
    only capable of commenting on dispatches found in post_ids
    '''
    types = ['post', 'comment', 'message', 'user_tag']
    ptypes = [.05, .35, .35, .25]
    draw = choice(types, p=ptypes)
    
    #using 48 bits instead of 64 bits because of python int size
    dispatch_id = int(random.getrandbits(48))
    
    if draw == 'post':
        tags = split_tags(random.sample(words, random.randint(0, 4)))
        user_tags = split_tags(random.sample(user['following'], 
            random.randint(0, len(user['following']) % 4)))
        csvfile.writerow(['message', user['image_path'], 'TEST POST',
            str(user['user_id']), ' ', tags, user_tags, str(0), str(user['user_id']),
            str(5), str(dispatch_id), 'post']) 
        post_ids.append(dispatch_id) 
    
    elif draw == 'comment':
        if len(post_ids) == 0:
            write_message_row(csvfile, user, overlap, words, post_ids)
            return
        parent_id = random.choice(post_ids)
        csvfile.writerow(['message', 'no image', 'TEST COMMENT',
        str(user['user_id']), ' ', ' ', ' ', str(1), str(parent_id),
        str(5), str(dispatch_id), 'comment'])              
    
    elif draw == 'message':
        #If True, group message. If False, individual message
        if random.choice([True, False]):
            # audience is any sample of overlapping followers and following
            # of size 10 or less
            audience = split_tags(random.sample(overlap, 
                random.randint(0, len(overlap) % 10 )))
        else:
            if len(overlap) == 0:
                write_message_row(csvfile, user, overlap, words, post_ids)
                return   
            audience = str(random.choice(list(overlap)))   
        csvfile.writerow(['message', 'no image', 'TEST MESSAGE',
            str(user['user_id']), audience, ' ', ' ', str(0), str(user['user_id']),
            str(5), str(dispatch_id), 'message']) 
    
    elif draw == 'user_tag':
        user_tag = random.choice(user['following'])
        if len(post_ids) == 0:
            write_message_row(csvfile, user, overlap, words, post_ids)
            return
        parent = random.choice(post_ids)
        csvfile.writerow(['message', 'no image', 'TEST USER_TAG',
            str(user['user_id']), ' ', ' ', str(user_tag), str(0), str(parent),
            str(5), str(dispatch_id), 'user_tag'])


def split_tags(tags):
    '''
    A simple function to split the user_tags and tags
    into the format we are using for build_json python functions,
    which take a space separated list of strings
    '''
    csvtags = ' '
    for i in tags:
        csvtags = csvtags + str(i) + ' '
    return csvtags
 

def write_feed_row(csvfile, user):
    '''
    write a row of the csv indicating a feed update
    '''
    csvfile.writerow(['feed', str(user['user_id'])])   


def write_profile_row(csvfile, user):
    '''
    write a row of the csv indicating a profile view
    '''
    options = list(set(user['following'] + user['followers']))
    if len(options) == 0:
        return
    user_id = random.choice(options)
    csvfile.writerow(['profile', str(user_id)]) 


def write_search_tags_row(csvfile, user, words):
    '''
    writes a row of the csv indicating a search for
    a tag to be chosen randomly from "words"
    '''
    tag = random.choice(words)
    user_id = str(user['user_id'])
    csvfile.writerow(['search_tags', user_id, tag ])


def write_lookup_row(csvfile, user, usernames):
    '''
    writes a row of the csv indiciating a search
    for a user who is neither a follower nor followee
    of the given user, but because we don't know what
    usernames are in the system based on one user's information,
    we instead populate this field with random words
    '''
    name = user['username']
    while(name == user['username']):
        name = random.choice(usernames)
    csvfile.writerow(['lookup', name])

def main():
    post_ids = []
    usernames = ['ellen', 'ricker', 'equinox']
    ellen = {'user_id': 1485, 'username': 'ellen', 'image_path': 'steve.jpg', 'name': 'Ellen Degeneres', 'fragmentation': 0, 'followers': [1435, 1475], 'following': [1435, 1475], 'ip': '140.233.20.153'}
    create_csv(ellen, 1000, post_ids, usernames)
    ricker = {'user_id': 1435, 'username': 'ricker', 'image_path': 'steve.jpg', 'name': 'William T. Ricker', 'fragmentation': 0, 'followers': [1485, 1475], 'following': [1485, 1475], 'ip': '140.233.20.181'}
    create_csv(ricker, 1000, post_ids, usernames)   
    equinox = {'user_id': 1475, 'username': 'equinox', 'image_path': 'steve.jpg', 'name': 'Noxy Knox', 'fragmentation': 0, 'followers': [1435, 1485], 'following': [1435, 1485], 'ip': '140.233.20.154'}
    create_csv(equinox, 1000, post_ids, usernames)


if __name__ == "__main__":
    main() 
