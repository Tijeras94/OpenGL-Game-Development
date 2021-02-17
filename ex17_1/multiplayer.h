/*****************************************************************************************************************
**
**	OpenGL Game Development
**	EX17_1.CPP (Example 17.1)
**
**	Copyright:	2003-2004, Chris Seddon. 
**	Written by: Chris Seddon
**	Date:		Mar 28, 2003
**
******************************************************************************************************************/

/* Libraries ******************************************************************************************************/
#pragma comment(lib,"ws2_32.lib")

/* Constants *****************************************************************************************************/
#define MULTIPLAYER_PROTOCOL	SOCK_DGRAM
#define MULTIPLAYER_PORT		6001
#define PACKET_SIZE				75

#define MM_PACKET_RECEIVED		5000


/* Enumerations **************************************************************************************************/
enum
{	
	MULTIPLAYER_CLIENT = 1,
	MULTIPLAYER_SERVER
};

enum
{
	PACKETTYPE_MOVEMENT = 0,
	PACKETTYPE_SHOOT
}; 


/* Class Definition **********************************************************************************************/
class MULTIPLAYER
{						
		struct	sockaddr_in sock_addr;
		long	connection_type;

	public:
		
		SOCKET	game_socket;
		bool	active;				
		char	incoming_packet[ PACKET_SIZE ];
		char	outgoing_packet[ PACKET_SIZE ];

		MULTIPLAYER();
		~MULTIPLAYER();

		bool InitServer();
		bool InitClient(char *hostname);
		void Release();
		
		bool Send();
		bool Receive();
};


/* Constructor ***************************************************************************************************/
MULTIPLAYER::MULTIPLAYER()
{	
	active = false;
}


/* Destructor ****************************************************************************************************/
MULTIPLAYER::~MULTIPLAYER()
{
}


/* InitServer ****************************************************************************************************/
bool MULTIPLAYER::InitServer()
{
	WSADATA wsaData;

	if (WSAStartup(0x202, &wsaData) == SOCKET_ERROR)
	{
		MessageBox (NULL, "Error: Unable to Start Winsock", NULL, MB_OK);
		WSACleanup();
		return (false);
	}

	sock_addr.sin_family 		= AF_INET;
	sock_addr.sin_addr.s_addr 	= INADDR_ANY;
	sock_addr.sin_port 			= htons(MULTIPLAYER_PORT);
	game_socket 				= socket(AF_INET, MULTIPLAYER_PROTOCOL,0);
	if (game_socket == INVALID_SOCKET)
	{
		MessageBox (NULL, "Error: Unable to Create Listening Socket", NULL, MB_OK);
		WSACleanup();
		return (false);
	}

	if (bind(game_socket, (struct sockaddr*)&sock_addr, sizeof(sock_addr) ) == SOCKET_ERROR)
	{
		MessageBox (NULL, "Error: Failed to Bind Listening Socket", NULL, MB_OK);
		WSACleanup();
		return (false);
	}
	
	active			= true;
	connection_type	= MULTIPLAYER_SERVER;

	return (true);
}


/* InitClient ****************************************************************************************************/
bool MULTIPLAYER::InitClient(char *hostname)
{
	WSADATA wsaData;
	struct 	hostent *hp;

	if (WSAStartup(0x202, &wsaData) == SOCKET_ERROR)
	{
		MessageBox (NULL, "Error: Unable to Start Winsock", NULL, MB_OK);
		WSACleanup();
		return (false);
	}

	hp = gethostbyname(hostname);
	if (hp == NULL )
	{
		MessageBox (NULL, "Error: Cannot resolve address", NULL, MB_OK);
		WSACleanup();
		return (false);
	}
	
	memset(&sock_addr,0,sizeof(sock_addr));
	memcpy(&(sock_addr.sin_addr),hp->h_addr,hp->h_length);
	sock_addr.sin_family	= hp->h_addrtype;
	sock_addr.sin_port 		= htons(MULTIPLAYER_PORT);

	game_socket = socket(AF_INET, MULTIPLAYER_PROTOCOL, 0);
	if (game_socket < 0 )
	{
		MessageBox (NULL, "Error: Cannot open socket", NULL, MB_OK);
		WSACleanup();
		return (false);
	}

	if (connect(game_socket, (struct sockaddr*)&sock_addr, sizeof(sock_addr)) == SOCKET_ERROR)
	{
		MessageBox (NULL, "Error: Failed to connect to server", NULL, MB_OK);
		WSACleanup();
		return (false);
	}
	
	active			= true;
	connection_type	= MULTIPLAYER_CLIENT;

	return (true);
}


/* Receive *******************************************************************************************************/
bool MULTIPLAYER::Receive()
{
	int retval;
	int sock_addr_len = sizeof(sock_addr);

	if (!active) return (false);
	
	if (connection_type == MULTIPLAYER_SERVER) retval = recvfrom(game_socket, incoming_packet, sizeof (incoming_packet), 0, (struct sockaddr *)&sock_addr, &sock_addr_len);
	else retval = recv(game_socket, incoming_packet, sizeof (incoming_packet), 0);

	if (retval == SOCKET_ERROR)
	{
		MessageBox (NULL, "Error: Failed to Receive UDP Packet", NULL, MB_OK);		
		return (false);
	}

	return (true);
}


/* Send **********************************************************************************************************/
bool MULTIPLAYER::Send()
{
	int retval;

	if (!active) return(false);

	if (connection_type == MULTIPLAYER_SERVER) retval = sendto(game_socket, outgoing_packet, sizeof (outgoing_packet),0, (struct sockaddr *)&sock_addr, sizeof(sock_addr));
	else retval = send(game_socket, outgoing_packet, sizeof(outgoing_packet), 0);

	if (retval == SOCKET_ERROR) MessageBox (NULL, "Error: Failed to send packet", NULL, MB_OK);

	return (true);
}


/* Release *******************************************************************************************************/
void MULTIPLAYER::Release()
{
	if (!active) return;

	closesocket(game_socket);
	WSACleanup();

	active = false;
}
