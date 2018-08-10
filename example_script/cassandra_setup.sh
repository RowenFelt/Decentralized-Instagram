#!/bin/bash


# delete data from previous runs
rm -rf nodes/$(hostname)/data/commitlog nodes/$(hostname)/data/data nodes/$(hostname)/data/hints nodes/$(hostname)/data/saved_caches

#setup configuration files
CASSANDRA_CONF=nodes/$(hostname)/conf bin/cassandra

