rm server.out client.out

python3 automake/automake_config_windows.py
make -f client.mk -j 12 -s 2> client.log
make -f server.mk -j 12 -s 2> server.log

mv client.out client.exe
mv server.out server.exe

python3 automake/automake_config.py
make -f client.mk -j 12 -s
make -f server.mk -j 12 -s
