import automake

compiler = "clang++"
cflags = "-std=c++20 -O3 -Wall -Wno-unused-command-line-argument -Wno-unused-variable -lpthread"
shared_src = ["src/common","vendor/vendorCompiled"]

debug_cflags = "-D_GLIBCXX_DEBUG -g -DDEBUG"

debug_cflags = ""

opengl_flags = "-lGL -lGLU -lglfw3 -lX11 -lXxf86vm -lXrandr -lXi -ldl -lXinerama -lXcursor -DGLEW_STATIC /usr/lib64/libGLEW.a"

c_include_flags = "-Ivendor -Ivendor_git/asio/asio/include -Ivendor_git/mingw-std-threads -Ivendor_git/glm -Ivendor_git/glew/include -Ivendor_git/glfw/include"
s_include_flags = "-Ivendor -Ivendor_git/asio/asio/include -Ivendor_git/mingw-std-threads -Ivendor_git/glm"

client_flags  = cflags +  " " + debug_cflags + " -DCLIENT_SIDE " + c_include_flags
server_flags  = cflags +  " " + debug_cflags + " -DSERVER_SIDE " + s_include_flags

objectfileDir = automake.objectfileDir

automake.objectfileDir = objectfileDir + "linux/server/"
automake.HeaderFiles_json = "automake/HeaderFiles_server.h.json"
automake.Make_makefile(compiler,server_flags,"",shared_src + ["src/server"],"server")

automake.objectfileDir = objectfileDir + "linux/client/"
automake.HeaderFiles_json = "automake/HeaderFiles_client.h.json"
automake.Make_makefile(compiler,client_flags,opengl_flags,shared_src + ["src/client"],"client")
