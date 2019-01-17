all:
	g++ -o server.exe server.cpp
	g++ -o client.exe client.cpp

clean:
	rm -r server.exe
	rm -r client.exe
