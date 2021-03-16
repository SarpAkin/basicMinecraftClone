import automake

compiler = "clang++"
cflags = "-std=c++20 -Wall -Wno-unused-command-line-argument -Wno-unused-variable -Ivendor -lpthread"
shared_src = ["src/common","vendor/vendorCompiled"]

debug_cflags = "-D_GLIBCXX_DEBUG -g -DDEBUG -O0"

opengl_flags = "-lGL -lGLU -lglfw3 -lX11 -lXxf86vm -lXrandr -lXi -ldl -lXinerama -lXcursor -Ivendor/glm/glm  -DGLEW_STATIC /usr/lib64/libGLEW.a"

client_flags  = cflags +  " " + debug_cflags + " " + opengl_flags + " -DCLIENT_SIDE"
server_flags  = cflags +  " " + debug_cflags +                      " -DSERVER_SIDE"

objectfileDir = automake.objectfileDir

automake.objectfileDir = objectfileDir + "server/"
automake.HeaderFiles_json = "automake/HeaderFiles_server.h.json"
automake.Make_makefile(compiler,server_flags,shared_src + ["src/server"],"server")

automake.objectfileDir = objectfileDir + "client/"
automake.HeaderFiles_json = "automake/HeaderFiles_client.h.json"
automake.Make_makefile(compiler,client_flags,shared_src + ["src/client"],"client")