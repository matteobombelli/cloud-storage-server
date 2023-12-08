# cloud-storage-server

Launcher for a cloud storage server, runs with settings in 'config.txt' or creates default 'config.txt' if run without. <br />
Intended connections should be from cloud-storage-client. // TODO link here <br />
<br />
Server first asks clients to login, then stores, sends, and deletes files requested by the client in the user's directory.