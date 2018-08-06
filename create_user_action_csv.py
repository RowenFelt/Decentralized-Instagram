#!/usr/bin/env python3

from csv import writer 
from numpy.random import choice

def create_csv(user, lines):
    '''
    takes a dictionary representing a user object and 
    populates a csv with random user actions, starting
    with creating a user object
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
    csvfile.writerow(['user', user['username'], user['image_path'],
        user['name'], str(user['fragmentation']), followers, 
        following, user['ip']])

    options = ['message', 'feed', 'profile', 'search_tags', 'lookup']
    probability = [.1, .7, .1, .05, .05,]
    for i in range(0,lines):
        draw = choice(options, p=probability)
        print(draw) 
