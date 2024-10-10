# ChatOnRing RCI

## Introduction
ChatOnRing is the final project of the Computer Networks and the Internet course. The aim of this cli application is to communicates with other nodes running the same application using TCP protocol. Each application running is a node in a ring topology where each node is doubly linked to its direct neighbours, the is also the possibility of forming chords between nodes there are in the ring but are not direct neighbours, i.e., adjacent to each other. The nodes should also be capable of communicate with a remote server through UDP to get/provide data on the ring state.

## Compilation

Firstly, let's compile the application. Simply type make on your terminal in the project's file directory.

```sh
make
```

## Program Execution

### Invocation

The COR application is executed from the command line using the following format:

```sh
./COR IP TCP [regIP] [regUDP]
```
- `COR`: The executable name.
- `IP`: Our IP address. Can use 127.0.0.1 to build a ring on local machine with different terminals.
- `TCP`: Port number.
- `regIP`: IP of the remote server that keeps track of the ring state, how many nodes there are.
- `regUDP`: The port of the UDP connection.
If you want to use a different server to keep track of the ring state, specify regIP and regUDP (it must use UDP protocol). 

### Commands

Once running, the application accepts the following commands:
- `j ring id`: joins ring from 000 to 999, with id from 00 to 99.
- `dj id succid succIP succTCP`: directly joins the ring without communicating with remote server. The node need to know the succid, succIP and succTCP (id, ip and port of the successor node it wants to join respectively).
- `c i`: Create chord with node i.
- `rc`: Remove its chord (doesn't need id because a node only is able to create 1 node).
- `st`: Shows topology of the ring.
- `sr dest`: Shows routing table for dest node.
- `sp path`: Shows the shortest path from node to dest node.
- `sf`: Shows expedition table for a node.
- `m dest message`: Sends a message (up to 128 characters) to node dest.
- `l`: Leaves the ring.
- `x`: Closes application.
