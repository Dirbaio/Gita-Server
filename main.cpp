#include <iostream>
#include <list>
#include <SFML/Network.hpp>
#include <sstream>
#include <queue>

using namespace std;

void error(const char* msg)
{
	cout<<"[ERROR] "<<msg<<endl;
	exit(1);
}

sf::SocketSelector selector;

class Player
{
	public:
		sf::TcpSocket* socket;
		string keys;
		bool keysReceived;
		bool disconnected;

		Player(sf::TcpSocket* s)
		{
			this->socket = s;
			keysReceived = false;
			disconnected = false;
		}
};


int playerCount = 3;

class Game
{
	public:
		vector<Player> players;
		bool running;
		bool ended;
		int targetLag;
		int currentLag;

		Game()
		{
			running = false;
			ended = false;
			currentLag = 0;
			targetLag = 3;
		}

		void sendPlayerCount()
		{
			sf::Packet p;
			p << int(0) << int(players.size()) << playerCount;

			for(int j = 0; j < players.size(); j++)
				if(players[j].socket != NULL)
					players[j].socket -> send(p);
		}

		void addPlayer(sf::TcpSocket* s)
		{
			players.push_back(Player(s));
			sendPlayerCount();
		}

		void start()
		{
			int mapSize = 2;
			int personCount = 300;
			int policeCount = 60;
			int seed = rand();
			for(int i = 0; i < players.size(); i++)
			{
				sf::Packet doStart;
				doStart << int(1);
				players[i].socket->send(doStart);
				sf::Packet start;
				start << i << int(players.size()) << mapSize << personCount << policeCount << seed;
				players[i].socket->send(start);
			}

			running = true;
		}

		void updatePregame()
		{
			bool updated = false;
			for(int i = 0; i < players.size(); i++)
			{
				if(players[i].socket == NULL) continue;

				sf::TcpSocket& client = *players[i].socket;
				if (selector.isReady(client))
				{
					// The client has sent some data, we can receive it
					sf::Packet packet;
					sf::Socket::Status res = client.receive(packet);
					if (res == sf::Socket::Done)
					{
						cout<<"Wat da fuq"<<endl;
					}
					else if(res != sf::Socket::NotReady)
					{
						cout<<"Player "<<i<< " disconnected in pre-game."<<endl;
						selector.remove(*players[i].socket);
						players[i].disconnected = true;
						updated = true;
					}
				}
			}
			for(int i = 0; i < players.size(); i++)
			{
				if(players[i].disconnected)
				{
					players.erase(players.begin()+i);
					i--;
				}
			}

			if(updated)
				sendPlayerCount();
		}

		void update()
		{
			for(int i = 0; i < players.size(); i++)
			{
				if(players[i].socket == NULL) continue;

				sf::TcpSocket& client = *players[i].socket;
				if (selector.isReady(client))
				{
					// The client has sent some data, we can receive it
					sf::Packet packet;
					sf::Socket::Status res = client.receive(packet);
					if (res == sf::Socket::Done)
					{
						cout<<">>"<<i<<endl;
						string s;
						packet >> s;

						players[i].keys = s;
						players[i].keysReceived = true;

					}
					else if(res != sf::Socket::NotReady)
					{
						cout<<"Player "<<i<< " disconnected."<<endl;
						selector.remove(*players[i].socket);
						players[i].socket = NULL;

						bool allGone = true;
						for(int j = 0; j < players.size(); j++)
							if(players[j].socket != NULL)
								allGone = false;

						if(allGone)
						{
							ended = true;
							running = false;

							cout<<"Game ended: all players disconnected."<<endl;
						}
					}


					bool allDone = true;
					for(int j = 0; j < players.size(); j++)
						if(!players[j].keysReceived && players[j].socket != NULL) allDone = false;

					if(allDone)
					{
						cout<<"SEND"<<endl;
						sf::Packet keyPacket;
						for(int j = 0; j < players.size(); j++)
							keyPacket << players[j].keys;

						for(int j = 0; j < players.size(); j++)
							if(players[j].socket != NULL)
							{
								players[j].keysReceived = false;
								if(targetLag >= currentLag)
									players[j].socket -> send(keyPacket);
								if(targetLag > currentLag)
									players[j].socket -> send(keyPacket);
							}

						if(targetLag < currentLag)
							currentLag--;
						if(targetLag > currentLag)
							currentLag++;
					}
				}
			}
		}
};

int main()
{
	srand(time(0));
	list<Game> games;
	Game newGame;

	// Create a socket to listen to new connections
	sf::TcpListener listener;
	unsigned short port = 6174;
	if(listener.listen(port) != sf::Socket::Done)
	{
		cerr<<"Can't listen :'("<<endl;
		return 1;
	}

	cout<<"Listening for clients on port "<<port << endl;


	// Add the listener to the selector
	selector.add(listener);

	bool running = true;
	// Endless loop that waits for new connections
	while (running)
	{
		// Make the selector wait for data on any socket
		if (selector.wait())
		{
			// Test the listener
			if (selector.isReady(listener))
			{
				// The listener is ready: there is a pending connection
				sf::TcpSocket* client = new sf::TcpSocket;
				if (listener.accept(*client) == sf::Socket::Done)
				{
					// Add the new client to the selector so that we will
					// be notified when he sends something
					selector.add(*client);

					newGame.addPlayer(client);
					cout << "Connected player from "<<client->getRemoteAddress()<<endl;

					if(newGame.players.size() == playerCount)
					{
						cout<<"Starting game!"<<endl;
						games.push_back(newGame);
						games.back().start();
						newGame = Game();
					}
				}
			}
			else
			{
				newGame.updatePregame();
				// The listener socket is not ready, test all other sockets (the clients)
				for(list<Game>::iterator it = games.begin(); it != games.end(); it++)
					it->update();

			}
		}
	}

	listener.close();

	cout << "Exiting..."<<endl;

	return 0;
}

