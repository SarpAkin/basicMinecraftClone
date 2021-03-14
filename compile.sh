python3 automake/automake_config.py
make -f client.mk -j 12 -s
make -f server.mk -j 12 -s
#cp out.out ../out
