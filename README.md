# Intro to Client-Server Model
A simple weather reporting application to illustrate the client-server model.  
**NOTE:** This application was part of a university assignment and uses library functions that may be deprecated. Adjustments are being worked on.

## Getting Started

### Prerequisites
- GNU Compiler Collection (installed either directly or through a Linux-based runtime environment like Cygwin)

### Usage
- A small list of cities with their matching weather conditions is given in the file *weather.txt*. To make any changes to this list, make sure there is only one entry per line and use the format:
```text
city,temp,conditions
```
#### Starting the Server
- Open a new Terminal window
- Navigate to the Server folder
- To build the server-side executable, type:
```bash
g++ server.cpp -std=c++14
```
- Now you can start/restart the server at any time by typing (on Windows/Linux):
```bash
a.exe
```
- If on Mac, instead type:
```bash
./a.out
```

#### Interacting with the Server
- Open a new Terminal window
- Navigate to the Client folder
- To build the client-side executable, type:
```bash
g++ client.cpp -std=c++14
```
- Now you can start/restart the client-side application at any time by typing (on Windows/Linux):
```bash
a.exe
```
- If on Mac, instead type:
```bash
./a.out
```
- Enter your server host name and port (NOTE: if running solely on one system, type 'localhost' as the server host name)
- If all was successful, you should see a message that says "Connection successful!" along with a prompt "Enter a city name:"
- Type in any city name that you would like the weather information for. If the city has information found in the
  *weather.txt* file, it will be taken from the server and displayed. Otherwise, "No data." will be displayed 

## Outline
### The Client
+ client.cpp : main client-side application

### The Server
+ server.cpp : main server application
+ weather.txt : contains the data to be sent to the client
