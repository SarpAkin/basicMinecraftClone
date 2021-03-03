import automake

compiler = "clang++"
cflags = "-std=c++17 -Wall -Wno-unused-command-line-argument -Ivendor"
src_dirs = ["src","vendor/vendorCompiled"]

debug_cflags = "-D_GLIBCXX_DEBUG -g -DDEBUG -O3"

opengl_flags = "-lGL -lGLU -lglfw3 -lX11 -lXxf86vm -lXrandr -lpthread -lXi -ldl -lXinerama -lXcursor -Ivendor/glm/glm  -DGLEW_STATIC /usr/lib64/libGLEW.a"

cflags += " " + debug_cflags + " " + opengl_flags

automake.Make_makefile(compiler,cflags,src_dirs,"out")