from calendar import c
import ctypes
import queue
from ctypes import *
from re import L
from symbol import namedexpr_test
from xml.dom.expatbuilder import ParseEscape
from collections import namedtuple
from dataclasses import dataclass
from typing import List

dll_path = '.\TextParser.dll'
lib = ctypes.windll.LoadLibrary(dll_path);

PARSER = ctypes.c_void_p
PVOID = PARSER
PPVOID = POINTER(PVOID)
LPCWSTR = ctypes.c_wchar_p
INT = ctypes.c_int
PINT = POINTER(INT)

CreateParser =lib['CreateParser']
CreateParser.restype = PARSER
CreateParser.argtypes = [LPCWSTR]

GetValue = lib['GetValue']
GetValue.restype = ctypes.c_bool
GetValue.argtypes = [PARSER,LPCWSTR,PPVOID,PPVOID]

ReleaseParser = lib["ReleaseParser"]
ReleaseParser.restype = None
ReleaseParser.argtypes = [PARSER]

def GetFloatFromParser(ps,key) ->float:
    pStart = PVOID();
    pEnd= PVOID();
    GetValue(ps,key,pointer(pStart),pointer(pEnd));
    length = int((pEnd.value - pStart.value) / ctypes.sizeof(ctypes.c_wchar))

    start : ctypes.c_wchar_p = ctypes.cast(pStart,ctypes.c_wchar_p)
    ret = float(start.value[0:length]);
    return ret

def GetIntFromParser(ps,key)->int:
    pStart = PVOID()
    pEnd= PVOID()
    GetValue(ps,key,pointer(pStart),pointer(pEnd))
    length = int((pEnd.value - pStart.value) / ctypes.sizeof(ctypes.c_wchar))

    start : ctypes.c_wchar_p = ctypes.cast(pStart,ctypes.c_wchar_p)
    ret = int(start.value[0:length])
    return ret

def GetTextFromParser(ps : PARSER,key : str)->str:
    pStart = PVOID()
    pEnd= PVOID()
    GetValue(ps,key,pointer(pStart),pointer(pEnd))
    if(pStart.value == None):
        return ""
    length = int((pEnd.value - pStart.value) / ctypes.sizeof(ctypes.c_wchar))
    start : ctypes.c_wchar_p = ctypes.cast(pStart,ctypes.c_wchar_p)
    ret = (start.value[0:length])
    return ret

class funcNamePair:
    def __init__(self,name : str,num : str):
        self.packetName = name
        self.recv_funcName= num
class packetNumInfo:
    def __init__(self, name : str, num : str):
        self.packetName = name
        self.num = num

class paramInfo:
    def __init__(self,type : str, name : str):
        self.type : str = type
        self.name = self.register(name)

    def register(self,name : str) -> str:
        self.isNoBuf : bool = False
        self.isDest : bool = False
        if(name.count("DEST_") > 0):
            self.isDest : bool = True
            name = name.replace("DEST_","")
        if(name.count("NOBUF_") > 0):
            self.isNoBuf = True
            name = name.replace("NOBUF_","")
        return name

    def nameToSerialize(self) -> str:
        if(self.isNoBuf == False):
            return self.name
        else:
            return ""

g_packetInfoQ : queue = queue.Queue()
g_forwardDeclQ : queue = queue.Queue()
g_packetProcQ : queue = queue.Queue()
g_packetInfoQ : queue = queue.Queue()
Serializer : str = "g_sb"

def PreProcessUserInput(input : str) ->str:
    input = input[2:]
    input = input.replace("\r","")
    return input

def ProcessParamType(suf : int,count : int ,key : str, value : str) -> str:
    param = str(suf) + key + str(count) + " : " + value + ';\n'
    return param

def ProcessParamName(suf: int , count: int , key : str, value : str) -> str:
    param = str(suf) + key + str(count) + " : " + value + ';\n'
    return param

def ProcessOther(suf : int, line : str) -> str:
    isFuncName : bool = False
    param = str(suf) + line + ';\n'
    return param

def DecoUserInput(suf : int, paramNum : int, decoRet : str) -> str:
    ret = ""
    funcName : str = ""
    lines = decoRet.splitlines()
    subHold : int = paramNum
    for line in lines:
        key , value = line.split(" : ")
        if(key == 'paramType'):
            line = ProcessParamType(suf,paramNum - subHold,key,value)
        elif(key == 'paramName'):
            line = ProcessParamName(suf,paramNum- subHold,key,value)
            subHold -= 1
        else:
            line = ProcessOther(suf,line)
        ret += line
    return ret

def MakePacketName(decoRet : str) -> str:
    temp = decoRet.split("funcName : ")[1]
    index : int = temp.find(';')
    return temp[ : index]

def makeParamList(ps : PARSER, paramNum : int, suf : int) -> List:
    paramList : List[paramInfo] = []
    destParamName = GetTextFromParser(ps,str(suf) + "DestParam")
    start = 0
    end = int(paramNum)
    if(destParamName != ""):
        paramList.insert(0,paramInfo("int",destParamName))
        start = 1
        end = int(paramNum) + 1
        
    for i in range(start,end):
        type = GetTextFromParser(ps,str(suf) + "paramType" + str(i-start))
        name = GetTextFromParser(ps,str(suf) + "paramName" + str(i-start))
        temp : paramInfo = paramInfo(type,name)
        paramList.insert(i,temp)
    return paramList

def DECOStubClassMember(suf : str,parseRet : str)->str:
    preRet = PreProcessUserInput(parseRet) 
    typeNum = parseRet.count("paramType")
    nameNum = parseRet.count("paramName")
    if(typeNum != nameNum):
        exit(-1)

    decoRet = DecoUserInput(suf,typeNum,preRet)
    decoRet += suf + "paramNum : " + str(typeNum) + ";\n"
    if(int(suf) < 0):
        return decoRet

    packetName = MakePacketName(decoRet)
    decoRet += suf + "packetName : "  + "dfPACKET_" + MakePacketName(decoRet) + ";\n"
    decoRet += suf + "RECV_Param : "  + "fromId;\n"
    decoRet += suf + "RECV_funcName : " + MakePacketName(decoRet) + "_RECV" + ";\n"
    return decoRet

def makePacketProc() -> str:
    ret : str = "bool PacketProc(int fromId, char packetType)\n\t{\n\t\tswitch(packetType)\n\t\t{\n"
    len : int = g_packetProcQ.qsize()
    for i in range(0,len):
        temp : funcNamePair = g_packetProcQ.get()
        ret += "\t\tcase " + temp.packetName + ":\n\t\t\treturn " + temp.recv_funcName + "(fromId);\n"
    ret += "\t\tdefault:\n\t\t\treturn false;\n\t\t}\n\t}\n\t"
    return ret

def makeMemberFunc(ps : PARSER, index : int) -> str :
    suf = str(index)
    returnType : str = GetTextFromParser(ps,suf + "returnType")
    recvParamName : str = GetTextFromParser(ps,suf + "RECV_Param")
    funcName : str = GetTextFromParser(ps, suf + "funcName")
    packetNum : str = GetTextFromParser(ps, suf + "packetNum")
    packetName : str = GetTextFromParser(ps,suf + "packetName")
    paramNum : int = int(GetTextFromParser(ps,suf + "paramNum"))
    recvFuncName : str = GetTextFromParser(ps,suf + "RECV_funcName")
    paramList : List[paramInfo] = makeParamList(ps,paramNum,suf)
    ret = returnType + " " + recvFuncName + "(int " + recvParamName + ")\n\t{"
    
    for i in range(0,paramNum):
        ret += "\n\t\t" + paramList[i].type + " " + paramList[i].name + ";"
    ret += "\n\t\t" + Serializer
    for i in range(0,paramNum):
        ret += " >> " + paramList[i].name
    ret += ';\n\t\t' + Serializer + ".Clear();\n\t\t" + funcName + '(' + recvParamName
    for i in range(0,paramNum):
        ret += ", " + paramList[i].name
    ret += ");\n\t\treturn true;\n\t}\n\n\t"
    ret += "virtual bool " + funcName + '(int ' + recvParamName

    for i in range(0,paramNum):
        ret += ", " + paramList[i].type + " " + paramList[i].name 
    ret += ")\n\t{\n\t\treturn false;\n\t}"
    return ret

def makeCSProcClass(ps : PARSER, funcNum : int) -> str:
    ret = "class CSProc : public CStub\n{"
    for i in range(0,funcNum):
        ret += "\n\tvirtual " + GetTextFromParser(ps,str(i) + "returnType") + " " + GetTextFromParser(ps,str(i) + "funcName") + '(int ' + GetTextFromParser(ps,str(i)+ "RECV_Param")
        paramNum : int = int(GetTextFromParser(ps,str(i) + "paramNum"))
        paramList : List[paramInfo] = makeParamList(ps,paramNum,i)
        for i in range(0,paramNum):
            ret += ", " + paramList[i].type + " " + paramList[i].name 
        ret += ");"
    ret += "\n};"
    return ret

def makeHeaderFile(ps : PARSER) -> str:
    funcNum : int = g_packetInfoQ.qsize()
    ret : str = "#pragma once\n#include \"SerializeBuffer.h\"\n"
    ret += "extern SerializeBuffer " + Serializer + ";\n"
    ret += "#pragma warning(disable : 4700)\n\n"
    ret += "class CStub\n{\nprivate:\n\tenum PacketNum\n\t{"
    len : int = g_packetInfoQ.qsize()
    for i in range(0,len):
        temp : packetNumInfo = g_packetInfoQ.get()
        ret += "\n\t\t" + temp.packetName + " = " + temp.num + ','
    ret += "\n\t};\n\npublic:\n\t"

    ret += makePacketProc() +'\n\t'
    for i in range(0,funcNum):
        ret += makeMemberFunc(ps,str(i)) + "\n\n\t"
    ret = ret.rstrip("\n\n\t") + "\n};\n\n"

    ret += makeCSProcClass(ps,funcNum) + "\n\n" + "extern CSProc g_CSProc;\n\n#pragma warning(default : 4700)"
    ret += ""
    return ret


if __name__ == '__main__':
    stubScriptFile = 'stubScript.txt'
    stubScriptDecoFile = stubScriptFile.split('.')[0] + "_Deco.txt"
    targetHeaderFile = "CStub.h"

    ps : PARSER = CreateParser(stubScriptFile)

    index : int = 0;
    DecoFileHandle = open(stubScriptDecoFile,"w",encoding = "utf-16-le")
    fileOutRet : str = ""
    while(True):
        ret0 = GetTextFromParser(ps,str(index))
        if(ret0 == ""):
            break
        fileOutRet += DECOStubClassMember(str(index),ret0) + "\n\n"
        index += 1
    DecoFileHandle.write("\n" + fileOutRet)
    DecoFileHandle.close()
    ReleaseParser(ps)


    ps = CreateParser(stubScriptDecoFile)
    for i in range(0,index):
        g_packetProcQ.put(funcNamePair(GetTextFromParser(ps,str(i) + "packetName"),GetTextFromParser(ps,str(i) + "RECV_funcName")))
        g_packetInfoQ.put(packetNumInfo(GetTextFromParser(ps,str(i) + "packetName"),GetTextFromParser(ps,str(i) + "packetNum")))
        
    headerFileHandle = open(targetHeaderFile,"w",encoding = "utf-8")
    headerRet : str = makeHeaderFile(ps)
    headerFileHandle.write(headerRet)
    headerFileHandle.close()
    ReleaseParser(ps)


