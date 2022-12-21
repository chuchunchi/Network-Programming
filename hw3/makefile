all:
	sudo gcc -o ./build/server server.cpp -lstdc++
	sudo mount -t nfs4 -o nfsvers=4.1,rsize=1048576,wsize=1048576,hard,timeo=600,retrans=2,noresvport fs-02210488dd33f643b.efs.us-east-1.amazonaws.com:/ /efs

