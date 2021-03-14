import os
import sys
import json


# valid CPP formats
cpp_format = [".cpp", ".c"]

executable_dir = sys.argv[0]
executable_dir = executable_dir[0:executable_dir.rfind("/") + 1]

executable_dir = ""


def findInDir(path: str):
    cpp_files = list()
    files: list = os.listdir(executable_dir + path)
    for file in files:
        fileDir: str = path + "/" + file
        if(os.path.isdir(executable_dir + fileDir)):
            cpp_files += findInDir(fileDir)
        for _format in cpp_format:
            if(os.path.isfile(executable_dir + fileDir)):
                file_format = fileDir[-len(_format):len(fileDir)]
                if(file_format == _format):
                    cpp_files.append(fileDir)
    return cpp_files


def FindAllHeaders(path: str) -> set:
    headers = set()
    filedir: str = path[0:path.rfind("/")] + "/"
    file = open(executable_dir + path, "r")
    fdata: str = file.read() + "\n"
    findex = fdata.find("#include")
    while(findex != -1):
        includeLine = fdata[findex:fdata.find("\n", findex)]
        if(includeLine[-1] != "/"):
            psIndex = includeLine.find("\"")
            if(psIndex != -1):
                headers.add(
                    filedir + includeLine[psIndex + 1:includeLine.find("\"", psIndex + 1)])

        findex = fdata.find("#include", findex + 1)
    header_headers = set()
    for header in headers:
        header_headers.update(FindAllHeaders(header))
    return headers | header_headers


objectfiles = list()

objectfileDir = "objectFiles/"

def cpp_To_o(cpp_file: str) -> str:
    return objectfileDir + (cpp_file + ".o").replace("/", "_")


def list_To_Str(hfiles: list) -> str:
    hstr = ""
    for hf in hfiles:
        hstr += " " + hf
    return hstr


def MakeObfiles(cppfile: str, hstr: str):
    objectfile = cpp_To_o(cppfile)
    objectfiles.append(objectfile)
    return objectfile + ":" + cppfile + hstr + "\n\t$(CC) -c " + cppfile + " -o " + objectfile + " $(CFLAGS)\n"

HeaderFiles_json = "automake/HeaderFiles.json"


def Make_makefile(compiler: str, cflags: str, srcdirs: list, outfile: str):
    global objectfiles
    objectfiles = list()

    writestr = ""
    writestr += "CC = " + compiler + "\n"
    writestr += "CFLAGS = " + cflags + "\n"
    writestr += "OUT = " + outfile + ".out\n"

    # Update this in orignal

    cpp_files = list()
    for src in srcdirs:
        cpp_files += findInDir(src)

    #
    ob_str = ""

    HeaderFiles: dict
    if(os.path.isfile(executable_dir + HeaderFiles_json)):
        HeaderFiles = json.load(open(executable_dir + HeaderFiles_json, "r"))
        for file in cpp_files:
            o_File: str = cpp_To_o(file)
            if(os.path.isfile(executable_dir + o_File)):
                if(os.path.getmtime(executable_dir + o_File) > os.path.getmtime(executable_dir + file)):
                    #print(file + " is up to date")
                    ob_str += MakeObfiles(file, HeaderFiles[o_File])
                    continue
            h_Files = list_To_Str(FindAllHeaders(file))
            ob_str += MakeObfiles(file, h_Files)
            HeaderFiles[o_File] = h_Files
    else:
        HeaderFiles = dict()
        for file in cpp_files:
            h_Files = list_To_Str(FindAllHeaders(file))
            ob_str += MakeObfiles(file, h_Files)
            HeaderFiles[cpp_To_o(file)] = h_Files
    json.dump(HeaderFiles, open(executable_dir + HeaderFiles_json, "w"))

    obFileStr = ""
    for obf in objectfiles:
        obFileStr += " " + obf

    writestr += "\n$(OUT):" + obFileStr + \
        "\n\t$(CC) -o $(OUT) " + obFileStr + " $(CFLAGS)\n\n"
    writestr += ob_str

    mfile = open(executable_dir + outfile + ".mk", "w")
    mfile.write(writestr)
