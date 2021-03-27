import automake

compiler = "clang++ --target=x86_64-pc-win32-gnu"
cflags = "-std=c++20 -Wall -Wno-unused-command-line-argument -Wno-unused-variable -O3 --static -lpthread -pthread -lws2_32 -lwsock32"
shared_src = ["src/common","vendor/vendorCompiled"]

debug_cflags = "-D_GLIBCXX_DEBUG -g"

#debug_cflags = ""

c_include_flags = "-Ivendor -Ivendor_git/asio/asio/include -Ivendor_git/glm -Ivendor_git/glew/include -Ivendor_git/glfw/include"
s_include_flags = "-Ivendor -Ivendor_git/asio/asio/include -Ivendor_git/glm"

opengl_flags = "compiled_libs/windows/libglfw3.a compiled_libs/windows/libglew32.a -lopengl32 -lgdi32 -luser32 -lkernel32"

client_flags  = cflags +  " " + debug_cflags + " -DCLIENT_SIDE " + c_include_flags + " -DGLEW_STATIC"
server_flags  = cflags +  " " + debug_cflags + " -DSERVER_SIDE " + s_include_flags

objectfileDir = automake.objectfileDir + "windows/"

automake.objectfileDir = objectfileDir + "server/"
automake.HeaderFiles_json = "automake/HeaderFiles_server.windows.h.json"
automake.Make_makefile(compiler,server_flags,"",shared_src + ["src/server"],"server","exe")

automake.objectfileDir = objectfileDir + "client/"
automake.HeaderFiles_json = "automake/HeaderFiles_client.windows.h.json"
automake.Make_makefile(compiler,client_flags,opengl_flags,shared_src + ["src/client"],"client","exe")