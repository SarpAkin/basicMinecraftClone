python3 automake/automake_config_windows.py
make -f client.mk -j 12 -s 2> client.log
make -f server.mk -j 12 -s 2> server.log

python3 automake/automake_config.py
make -f client.mk -j 12 -s
make -f server.mk -j 12 -s

rm bin/windows/game.zip bin/linux/game.zip

zip -r bin/windows/game.zip server.exe client.exe res
zip -r bin/linux/game.zip   server.out client.out res