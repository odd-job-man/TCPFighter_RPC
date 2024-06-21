from calendar import c
import ctypes
import queue
from ctypes import *
from re import L
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

class packetInfo:
    def __init__(self,name : str,num : int):
        self.name = name
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
g_headerFunc : str = "MAKE_HEADER"
g_sendEnqFunc : str = "EnqPacketUnicast"
Serializer : str = "g_sb"

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

def PreProcessUserInput(input : str) ->str:
    input = input[2:]
    input = input.replace("\r","")
    return input 

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

def DECOSCCppFunc(suf : str,parseRet : str)->str:
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
    suffix = ""
    if(int(suf) > 0):
        suffix = "DEST_NOBUF_";
    else:
        suffix = "DEST_"
    decoRet += suf + "DestParam : " + suffix + "destId;\n"
    return decoRet

def makeParamList(ps : PARSER, paramNum : int, suf : int) -> List:
    paramList : List[paramInfo] = []
    destParamName = GetTextFromParser(ps,suf + "DestParam")
    start = 0
    end = int(paramNum)
    if(destParamName != ""):
        paramList.insert(0,paramInfo("int",destParamName))
        start = 1
        end = int(paramNum) + 1
        
    for i in range(start,end):
        type = GetTextFromParser(ps,suf + "paramType" + str(i-start))
        name = GetTextFromParser(ps,suf + "paramName" + str(i-start))
        temp : paramInfo = paramInfo(type,name)
        paramList.insert(i,temp)
    return paramList
    
def makeSignature(returnType : str,funcName : str, paramList : List[paramInfo]) -> str : 
    signature = returnType + " " + funcName + '(';
    paramNum : int = len(paramList)
    for i in range(0,paramNum):
        temp = "";
        temp += paramList[i].type + " " + paramList[i].name + ", "
        signature += temp

    signature = signature.rstrip(", ") + ')'
    return signature

def makeHeaderFunc(ps : PARSER, index : int) -> str:
    suf : str = str(index)
    returnType : str = GetTextFromParser(ps,suf + "returnType")
    funcName : str = GetTextFromParser(ps, suf + "funcName")
    paramNum : str= GetTextFromParser(ps,suf + "paramNum")
    bomCode : str = GetTextFromParser(ps,suf + "bomCode")

    paramList = makeParamList(ps,paramNum,suf)
        
    signature : str = makeSignature(returnType,funcName,paramList)
    g_forwardDeclQ.put(signature)
    signature += "\n{\n\t"+ Serializer + " << (unsigned char)" + bomCode
    for i in range(0,int(paramNum)):
        temp = ""
        temp += " << (" + paramList[i].type + ')' + paramList[i].name
        signature += temp
    signature += ";\n\treturn sizeof((char)" + bomCode + ")"
    
    for i in range(0,int(paramNum)):
        temp = ""
        temp += " + sizeof(" + paramList[i].name + ")"
        signature += temp
    signature += ";\n}"
    return signature

"""
returnType funcName(SerializeBuffer& Serializer, paramType0 paramName0, paramType1 paramName1,....)
{
    constexpr int size = sizeof(paramName0) + sizeof(paramName1) + ....;
    int headerSize = MAKE_HEADER(Serializer, size, packetName);
    Serializer << paramName0 << paramName1 << ...;
    bool EnqRet = EnqPacketUnicast(NOBUF_destId, Serializer.GetBufferPtr(), headerSize + size);
    Serializer.Clear();
    return EnqRet;
}
"""
def makeSCCppFunc(ps : PARSER ,index : int) -> str:
    suf = str(index);
    returnType : str = GetTextFromParser(ps,suf + "returnType")
    funcName : str = GetTextFromParser(ps, suf + "funcName")
    packetNum : str = GetTextFromParser(ps, suf + "packetNum")
    packetName : str = GetTextFromParser(ps,suf + "packetName")
    paramNum : int = int(GetTextFromParser(ps,suf + "paramNum"))

    paramList = makeParamList(ps,paramNum,suf)
    paramNum = len(paramList)
    pi : packetInfo = packetInfo(packetName,int(packetNum))
    g_packetInfoQ.put(pi)
    
    signature : str = makeSignature(returnType,funcName,paramList)
    g_forwardDeclQ.put(signature)

    signature += "\n"
    signature += "{\n\t" + "constexpr int size = "
    for i in range(0,paramNum):
        temp = ""
        if(paramList[i].isNoBuf == True):
            continue
        temp += "sizeof(" + paramList[i].name + ") + "
        signature += temp
    signature = signature.rstrip(" + ")
    signature += ";\n\t"
    signature += "int headerSize = " + g_headerFunc + "(" + "size, " + pi.name + ");\n\t" + Serializer
    
    for i in range(0,paramNum):
        temp = ""
        if(paramList[i].isNoBuf == True):
            continue;
        temp += " << " + paramList[i].name
        signature += temp

    destID : str = ""
    for i in range(0,paramNum):
        if(paramList[i].isDest == True):
            destID = paramList[i].name
            break

    signature += ";\n\tbool EnqRet = " + g_sendEnqFunc + '(' + destID + ", " + Serializer + ".GetBufferPtr(), headerSize + size);\n\t"
    signature += Serializer + ".Clear();\n\treturn EnqRet;\n}"
    return signature
    
def makeSCCHeader(ps : PARSER) -> str:
    ret = "#pragma once\n"
    while(g_packetInfoQ.empty() == False):
        pi : packetInfo = g_packetInfoQ.get()
        temp : str = "constexpr int " + pi.name + " = " + str(pi.num) + ";\n"
        ret += temp;
    ret += '\n'
    while(g_forwardDeclQ.empty() == False):
        temp : str = g_forwardDeclQ.get()
        temp += ";\n"
        ret += temp
    ret += '\n'
    return ret

    

if __name__ == '__main__':
    protoFile = 'proxyScript.txt'
    protoDecoFile = protoFile.split('.')[0] + "_Deco.txt"
    targetCppFile = "SCContents.cpp"
    targetHeaderFile = targetCppFile.split('.')[0] + ".h"
    ps : PARSER = CreateParser(protoFile)

    funcNum: int = -1;
    fileOutRet = ""

    file = open(protoDecoFile,"w",encoding = "utf-16-le")
    while(True):
        ret0 = GetTextFromParser(ps,str(funcNum))
        if(ret0 == ""):
            break
        fileOutRet += DECOSCCppFunc(str(funcNum),ret0) + "\n\n"
        funcNum += 1
        
    print(fileOutRet)

    file.write('\n' + fileOutRet)
    file.close()
    ReleaseParser(ps)

    CppFileHandle = open(targetCppFile,"w",encoding='utf-8')
    headerFileHandle = open(targetHeaderFile,"w",encoding = 'utf-8')
    CppFileHandle.write("#include \"SerializeBuffer.h\"\n#include \"" + targetHeaderFile + "\"\nextern SerializeBuffer g_sb;\n\nbool EnqPacketUnicast(const int id, char* pPacket, const size_t packetSize);\n\n")
  

    ps = CreateParser(protoDecoFile)

    CppFileHandle.write(makeHeaderFunc(ps,-1) + "\n\n")

    for i in range(0,funcNum):
        CppFileHandle.write(makeSCCppFunc(ps,i) + "\n\n")
    
    headerFileHandle.write(makeSCCHeader(ps))

    CppFileHandle.close()
    headerFileHandle.close()
    ReleaseParser(ps)
    
    

    
    
