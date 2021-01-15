/*	COSC 3360 - Fundamentals of Operating Systems
 * 	Assignment 2 - Benjamin Berryman
 * 	client.cpp
 *
 *  This is the client side of my client-server model.
 *
 *  The program starts by prompting the user for a server host name and port number.
 *  It uses these to create a socket and connect to the server via TCP. The program then
 *  prompts the user again, this time for a city name (corresponding to the one the user
 *  wants weather info for). The program will send that city name in a message through the socket
 *  to the server and wait for a reply. Once the server responds with a message that has the
 *  corresponding weather info in it, the client program will output it to the user and again
 *  ask for a city name. This process repeats until the program is terminated with Ctrl-C.
 */

#include <errno.h>
#include <signal.h>
#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <stdexcept>
#include <cstring>
#include <stdlib.h>


int call_socket(char *host_name, int portnum)
{
	struct sockaddr_in s_address; //Socket info
	int s; //Used for the creation of the actual socket
	struct hostent *hp; //Host info

	if ((hp= gethostbyname(host_name)) == nullptr)
	{
		errno= ECONNREFUSED;
		return(-1);
	}

	memset(&s_address, 0, sizeof(struct sockaddr_in)); //Clears address space
	memcpy((char *)&s_address.sin_addr,hp->h_addr,hp->h_length);
	s_address.sin_family= hp->h_addrtype;
	s_address.sin_port= htons(portnum);

	if ((s= socket(hp->h_addrtype,SOCK_STREAM,0)) < 0)   /* get socket */
		return(-1);
	if (connect(s,(sockaddr *)&s_address,sizeof (s_address)) < 0)
	{
		close(s);
	    return -1;
	}
	std::cout << "Connection successful!" << std::endl;
	std::cout << "NOTE: To exit the program, press Ctrl-C on your keyboard." << std::endl << std::endl;

	return s;
}

int read_data(int s, //Socket descriptor
			  char *buf, //Pointer to buffer for data
			  int n) //Number of bytes we want
{
	int bytes_read = 0;
	int bytes_in_pass = 0;
	while (bytes_read < n)
	{
		if ((bytes_in_pass = read(s, buf, n-bytes_read)) > 0)
		{
			bytes_read += bytes_in_pass;
			buf += bytes_in_pass;
		}
		else if (bytes_in_pass < 0)
			return -1;
	}
	return bytes_read;
}

int write_data(int s, //Socket descriptor
			  char *buf, //Pointer to buffer for data
			  int n) //Number of bytes we want
{
	int bytes_written = 0;
	int bytes_in_pass = 0;
	while (bytes_written < n)
	{
		if ((bytes_in_pass = write(s, buf, n-bytes_written)) > 0)
		{
			bytes_written += bytes_in_pass;
			buf += bytes_in_pass;
		}
		else if (bytes_in_pass < 0)
			return -1;
	}
	return bytes_written;
}

/* =============
 * MAIN FUNCTION
 * =============
 */

int main()
{
	int s; //Socket descriptor
	std::string input;
	int port;

	/* ==============================================================
	 * FIRST, prompt the user for a server host name and port number.
	 * ==============================================================
	 */
	while (true)
	{
		std::cout << "NOTE: If running solely on one system, type 'localhost' as the server host name." << std::endl;
		std::cout << "Enter the server host name:";
		try
		{
			std::getline(std::cin, input);
			stoi(input);
			std::cout << "Not a valid input! \n";
		}
		catch (std::invalid_argument&)
		{
			break;
		}
	}

	int len = input.size();
	char *host_name = new char[len+1];
	copy(input.begin(), input.end(), host_name);

	while (true)
	{
		std::cout << "Enter the server port number:";
		try
		{
			getline(std::cin, input);
			port = stoi(input);
			break;
		}
		catch (std::invalid_argument&)
		{
			std::cout << "Not a valid input! \n";
		}
	}

	if ((s = call_socket(host_name, port)) < 0)
	{
		perror("connect");
		exit(1);
	}
	char read_buffer[1024], write_buffer[64];

	/* ==============================================================
	 * SECOND, prompt the user for a city name to send to the server.
	 * ==============================================================
	 */
	while (true)
	{
		while (true)
		{
			std::cout << "Enter a city name:";
			try
			{
				std::getline(std::cin, input);
				std::stoi(input);
				std::cout << "Not a valid input! \n";
			}
			catch (std::invalid_argument)
			{
				break;
			}
		}
		memset(&write_buffer,0,64);
		input.copy(write_buffer, 64);

		for (int i = 0; i < 64; i++) //Convert to lowercase before sending
		{
			write_buffer[i] = tolower(write_buffer[i]);
		}
		int byte_count;
		if ((byte_count = write_data(s, write_buffer, 64)) < 0)
		{
			perror("read");
			exit(1);
		}

	/* ====================================================================================
	 * THIRD, wait for a message from the connected server. When received, display to user.
	 * ====================================================================================
	 */
		memset(&read_buffer,0,1024);
		if ((byte_count = read_data(s, read_buffer, 1024)) < 0)
		{
			perror("read");
			exit(1);
		}
		std::cout << read_buffer << std::endl;

		//Repeat steps 2 and 3 until the client is terminated with Ctrl-C
	}
	return 0;
}
