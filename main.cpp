#include <iostream>
#include <list>
#include <SFML/Network.hpp>

using namespace std;

void error(const char* msg)
{
    cout<<"[ERROR] "<<msg<<endl;
    exit(1);
}

int main()
{
    while(true)
    {

        int playerCount = 1;

        vector<string> keys(playerCount);
        vector<sf::TcpSocket*> sockets (playerCount);

        int playerConnectedCount = 0;

        // Create a socket to listen to new connections
        sf::TcpListener listener;
        unsigned short port = 6174;
        listener.listen(port);

        cout<<"Listening for clients on port "<<port << endl;
        // Create a selector
        sf::SocketSelector selector;

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
                        sockets[playerConnectedCount] = client;
                        playerConnectedCount++;
                        cout << "Connected player "<<playerConnectedCount << " of " << playerCount <<  "! From "<<client->getRemoteAddress()<<endl;

                        if(playerConnectedCount == playerCount)
                            cout<<"Starting game!"<<endl;
                    }
                }
                else
                {
                    // The listener socket is not ready, test all other sockets (the clients)

                    for(int i = 0; i < playerCount; i++)
                    {
                        sf::TcpSocket& client = *sockets[i];
                        if (selector.isReady(client))
                        {
                            // The client has sent some data, we can receive it
                            sf::Packet packet;
                            sf::Socket::Status res = client.receive(packet);
                            if (res == sf::Socket::Done)
                            {
                                string s;
                                packet >> s;
                                keys[i] = s;

                                bool allDone = true;
                                for(int j = 0; j < playerCount; j++)
                                    if(keys[j] == "") allDone = false;

                                if(allDone)
                                {

                                    sf::Packet keyPacket;
                                    for(int j = 0; j < playerCount; j++)
                                        keyPacket << keys[j];

                                    for(int j = 0; j < playerCount; j++)
                                        sockets[j] -> send(keyPacket);

                                    keys = vector<string> (playerCount);
                                }
                            }
                            else if(res != sf::Socket::NotReady)
                            {
                                cout<<"Error!!"<<endl;
                                running = false;
                            }
                        }
                    }
                }
            }
        }

        listener.close();

        cout << "Killing game..."<<endl;

        for(int j = 0; j < playerConnectedCount; j++)
        {
            sockets[j]->disconnect();
            delete sockets[j];
        }

//        exit(1);
    }
    return 0;
}

