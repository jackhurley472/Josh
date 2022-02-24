Before running the program, start the Philips Hue simulator:
  - % sudo hue-simulator

Note: If you're running the simulator on a different host,
      change the HOST definition in client.cpp from "localhost"
      to the correct address.

Instructions to compile and run:
  - % g++ -std=c++14 -o client client.cpp
  - % ./client
  
Note: If the libraries are not in the same directory as client.cpp,
      you'll need to include the path to them.
 
Testing: Tested using webinterface to change on/off and brightness states.

Other notes:
  - Program uses the assumption that Philips Hue uses a brightness range
    of (0, 254) when calculating brightness percentage. 
  - Use "ctrl+c" to exit the program gracefully.
    Also, program exits gracefully when it loses connection.
