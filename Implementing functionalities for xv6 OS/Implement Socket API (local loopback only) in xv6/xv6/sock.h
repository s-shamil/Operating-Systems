
//
// TODO: Define an enumeration to represent socket state.
//
enum sockstate { CLOSED, LISTENING, CONNECTED };
//
// TODO: Define a structure to represent a socket.
//

struct sock {
  int localPort; // local port number , used by all except connect()
  int remotePort; // remote port number
  enum sockstate state; // CLOSED, LISTENING, CONNECTED
  int ownerPID; // process id of owner process
  char buffer[BUFFER_SIZE]; // used as both send and Receive buffer
  int dataPresent; // flag indicating buffer array state
};
