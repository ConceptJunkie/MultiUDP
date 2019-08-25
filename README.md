MultiUDP

This nothing more than a simple learning exercise for doing basic UDP communication and multi-threaded data processing.

There are three components:

server - server takes three arguments:  the input port, the output hostname, and the output port.

The server will listen on the input port, and process each packet that it receives and send the results to the output port.  Currently the function to process each packet doesn't actually transform the packet, but it delays for a random amount of time up to 5 seconds.  This tests the multithreaded processing by making sure that the packets are output in the same order in which they were received.

sender - sender takes three arguments:  the target host name, the target port, and a file to be used as a source of data.

The sender reads the contents of the file and sends them to the target host name and port via UDP connection.

listener - listener takes two arguments:  the port, and a file name to write the data

As currrently programmed, the output from listener should be identical to the input for sender.  

listener will keep listening until ctrl-c is pressed, at which time it will close the connection and the output file.
