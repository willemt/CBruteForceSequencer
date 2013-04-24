Brute Force Sequencer
==========

What?
-----
The Brute Force Sequencer's (BFS) job:
Ensure that the client has the exact same state we have for a list of objects.

Method:
a. All "sequenced objects" have multiple states.
b. All states have a sequence_number.
c. States are ordered by their sequence_numer (ie. this is a proxy for history/age).
d. The server (AKA BFS) sends a list of the most recent object states, including a sequence number.
e. The client ACKs the sequence_number it has received.
f. BFS retains the states that have not been ACKd by the client.
g. BFS discards client ACKd states.
h. BFS allows us to provide a delta between the client's last ACKd state and the state we are sending

This is loosely based off the networking model used by Quake3.

Written in C with a BSD license. 100% test coverage.

Building
--------

$make
