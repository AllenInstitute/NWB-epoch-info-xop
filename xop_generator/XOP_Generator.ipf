#pragma TextEncoding = "UTF-8"
#pragma rtGlobals=3 // Use modern global access method and strict wave access.
#pragma rtFunctionErrors=1

static Constant FIRST_XOP_ERR = 10000
static StrConstant LINE_ENDING = "\n"

/// Hook function which is executed after opening a file
static Function AfterFileOpenHook(refNum, file, pathName, type, creator, kind)
	variable refNum, kind
	string file, pathName, type, creator

	string funcList

	// do nothing if the opened file was not an Igor packed/unpacked experiment
	if(kind != 1 && kind != 2)
		return 0
	endif

	funcList = FunctionList("Generate", ";", "KIND:2,NPARAMS:0,WIN:[ProcGlobal]")
	if(ItemsInList(funcList) != 1)
		Abort "Could not find Generate function for autostart."
	endif

	FuncRef AUTOSTART_PROTO f = $StringFromList(0, funcList)
	f()

	Execute/P "Quit/N"
End

Function AUTOSTART_PROTO()
End

Function Generate()

	string xopName
	variable numOps, opNr, numLines, lineNr
	string opList, opStr, codeStr, n, line
	variable size, srcSize, fnum

	SetOutputPath()

	xopName = GetProjectName()

	WAVE/T out = Load("Operations.inp")

	Make/FREE/T/N=0 inOperations

	numLines = DimSize(out, 0)
	for(lineNr = 0; lineNr < numLines; lineNr += 1)
		line = out[lineNr]
		if(CmpStr(line, "") && CmpStr(line[0], "#"))
			ParseOperationTemplate/TS/T/S=1/C=6 line
			if(V_Flag)
				Abort "Error in Template: " + line
			endif
			AddLineToTextWave(inOperations, line)
			numOps += 1
		endif
	endfor
	Make/FREE/T/N=0 outOperations
	Make/FREE/T/N=0 outFunctions
	Make/FREE/T/N=0 outRegistrations
	Make/FREE/T/N=(numOps) outNames

	for(opNr = 0; opNr < numOps; opNr += 1)
		opStr = inOperations[opNr]
		ParseOperationTemplate/TS/T/S=1/C=6 opStr
		WAVE/T code = ListToTextWave(GetScrapText() + "\r", "\r")
		n = StringFromList(0, opStr, " ")
		outNames[opNr] = n

		// copy parts to waves
		FindValue/TEXT="#pragma pack()\t// Reset structure alignment to default."/TXOP=4 code
		if(V_Value == -1)
			Abort "Could not find end of operation structure"
		endif
		srcSize = V_Value + 2
		size = DimSize(outOperations, 0)
		Redimension/N=(size + srcSize) outOperations
		outOperations[size,] = code[p - size]
		DeletePoints 0, srcSize, code

		// replace by Handler
		DeletePoints 0, 1, code
		code[0] = "void Handler::" + n + "(" + n + "RuntimeParamsPtr p)"
		// remove int err = 0; and empty line
		DeletePoints 2, 2, code

		FindValue/TEXT="\treturn err;"/TXOP=4 code
		if(V_Value == -1)
			Abort "Could not find end of function template structure"
		endif
		// remove return err; and emtpy line
		DeletePoints V_Value - 1, 2, code
		srcSize = V_Value + 1
		size = DimSize(outFunctions, 0)
		Redimension/N=(size + srcSize) outFunctions
		outFunctions[size,] = code[p - size]
		DeletePoints 0, srcSize, code
// TODO patch in return VarList
		srcSize = DimSize(code, 0)
		size = DimSize(outRegistrations, 0)
		Redimension/N=(size + srcSize) outRegistrations
		outRegistrations[size,] = code[p - size]

	endfor
	WAVE/T errors = GetErrors()
	WAVE/T functionNames = GetFunctionNames()

	WAVE/T errorConstants = CreateIgorConsts(xopName, errors)
	// xop_errors.ipf
	WriteWave(errorConstants, "xop_errors.ipf")

	WAVE/T operationCode = CreateOperationHeader(outOperations)
	// Operation.h
	WriteWave(operationCode, "operations.h")

	WAVE/T errorCode = CreateErrorsHeader(errors)
	// xop_errors.h
	WriteWave(errorCode, "xop_errors.h")

	WAVE/T mainRes = CreateMainResource(xopname)
	// xxxx_xop.rc
	WriteWave(mainRes, xopName + "_xop.rc")

	WAVE/T winCustRes = CreateWinCustomsResource(errors, outNames, xopName)
	// xxxx_xopWinCustom.rc
	WriteWave(winCustRes, xopName + "_xopWinCustom.rc")

	WAVE/T macRes = CreateMacResource(errors, outNames, xopName)
	// xxxx_xop.r
	WriteWave(macRes, xopName + "_xop.r")

	// xxxx_xop.cpp
	WAVE/T mainCode = CreateMainCode(outNames, outRegistrations, functionNames, xopName)
	WriteWave(mainCode, xopName + "_xop.cpp")

	// xxxx_xop.h
	WAVE/T mainHeader = Load("mainFileHeader.def")
	WriteWave(mainHeader, xopName + "_xop.h")

	// xxxx_handler.cpp
	WAVE/T handlerCode = CreateHandlerCode(outFunctions, functionNames, xopName)
	WriteWave(handlerCode, xopName + "_handler.cpp")

	// xxxx_handler.h
	WAVE/T handlerHeader = CreateHandlerHeader(outNames, functionNames)
	WriteWave(handlerHeader, xopName + "_handler.h")
End

static Function SetOutputPath()

	string s

	s = GetEnvironmentVariable("CURPATH")
	if(IsEmpty(s))
		Abort "Environment variable CURPATH for output path not set"
	endif
	NewPath/Q/O outputPath, s
End

static Function/WAVE Load(fName)
	string fName

	LoadWave/Q/J/D/K=2/V={"\r", "", 0, 8}/P=home/O/N=opFile fName
	if(!V_flag)
		Abort "Could not load " + fName
	endif
	WAVE/T out = opFile0
	Duplicate/FREE/T out, outfree

	return outfree
End

static Function/S LoadString(fName)
	string fName

	variable i, numLines
	string s = ""

	WAVE/T out = Load(fName)
	numLines = DimSize(out, 0)
	for(i = 0; i < numLines; i += 1)
		s = s + out[i] + LINE_ENDING
	endfor

	return s
End

static Function/WAVE CreateMainResource(xopName)
	string xopName

	string fiBlock = LoadString("mainFileResourceFileInfoBlock.def")
	Make/FREE/T fileinfoBlock = { ReplaceString("%%XOPNAME%%", fiBlock, xopName) }

	string tiBlock = LoadString("mainFileResourceTextIncludeBlock.def")
	Make/FREE/T textInclude = { ReplaceString("%%XOPNAME%%", tiBlock, xopName) }

	string inclBlock = LoadString("mainFileResourceIncludeBlock.def")
	Make/FREE/T include = { ReplaceString("%%XOPNAME%%", inclBlock, xopName) }

	WAVE/T out = Load("mainFileResource.def")
	WAVE/T out1 = ReplaceKeyByTextWave(out, "%%STRINGFILEINFO%%", fileinfoBlock)
	WAVE/T out2 = ReplaceKeyByTextWave(out1, "%%TEXTINCLUDE%%", textInclude)
	return ReplaceKeyByTextWave(out2, "%%INCLUDE%%", include)
end

static Function/WAVE CreateIgorConsts(xopName, errors)
	string xopName
	WAVE/T errors

	variable i, numErrs
	string s

	xopName = UpperStr(xopName)

	WAVE/T out = Load("IgorConsts.def")
	string commonBlock = LoadString("IgorConstsCommonErrorBlock.def")
	Make/FREE/T errCommonBlock = { ReplaceString("%%XOPNAME%%", commonBlock, xopName) }
	string errorNrBlock = LoadString("IgorConstsErrorNumberBlock.def")

	Make/FREE/T/N=0 errNumberBlock
	numErrs = DimSize(errors, 0)
	for(i = 0; i < numErrs; i += 1)
		s = ReplaceString("%%XOPNAME%%", errorNrBlock, xopName)
		s = ReplaceString("%%ERRORTEXT%%", s, errors[i][1])
		s = ReplaceString("%%ERRORDEFINE%%", s, errors[i][0])
		s = ReplaceString("%%ERRORNUMBER%%", s, num2str(4 + FIRST_XOP_ERR + i))
		AddLineToTextWave(errNumberBlock, s)
	endfor
	WAVE/T errConstBlock = AppendTextWave(errCommonBlock, errNumberBlock)

	return ReplaceKeyByTextWave(out, "%%ERRORCONSTANTS%%", errConstBlock)
End

static Function/WAVE CreateWinCustomsResource(errors, names, xopName)
	WAVE/T errors
	WAVE/T names
	string xopName

	variable numNames, nameNr

	WAVE/T out = Load("ResourceWinCustom.def")
	string opBlock = LoadString("ResourceWinCustomOperationsBlock.def")
	string commErrBlock = LoadString("ResourceWinCustomCommonErrorBlock.def")

	Make/FREE/T commonErrBlock = { ReplaceString("%%XOPNAME%%", commErrBlock, xopName) }
	Make/FREE/T/N=(DimSize(errors, 0)) errMsgBlock
	errMsgBlock[] = PadString("  \"" + errors[p][1] + "\\0\",", 103, 0x20) + "// " + errors[p][0]
	WAVE/T errMsgBlock1 = AppendTextWave(commonErrBlock, errMsgBlock)
	WAVE/T out1 = ReplaceKeyByTextWave(out, "%%ERRORS%%", errMsgBlock1)

	Make/FREE/T/N=0 operationsBlock
	numNames = DimSize(names, 0)
	for(nameNr = 0; nameNr < numNames; nameNr += 1)
		AddLineToTextWave(operationsBlock, ReplaceString("%%OPERATIONNAME%%", opBlock, names[nameNr]))
	endfor
	return ReplaceKeyByTextWave(out1, "%%OPERATIONS%%", operationsBlock)
End

static Function/WAVE CreateMacResource(errors, names, xopName)
	WAVE/T errors
	WAVE/T names
	string xopName

	variable numNames, nameNr

	WAVE/T out = Load("ResourceMac.def")
	string opBlock = LoadString("ResourceMacOperationsBlock.def")
	string commErrBlock = LoadString("ResourceMacCommonErrorBlock.def")
	Make/FREE/T commonErrBlock = { ReplaceString("%%XOPNAME%%", commErrBlock, xopName) }

	Make/FREE/T/N=(DimSize(errors, 0)) errMsgBlock
	errMsgBlock[] = PadString("  \"" + errors[p][1] + "\",", 103, 0x20) + "// " + errors[p][0]
	WAVE/T errMsgBlock1 = AppendTextWave(commonErrBlock, errMsgBlock)
	WAVE/T out1 = ReplaceKeyByTextWave(out, "%%ERRORS%%", errMsgBlock1)

	Make/FREE/T/N=0 operationsBlock
	numNames = DimSize(names, 0)
	for(nameNr = 0; nameNr < numNames; nameNr += 1)
		AddLineToTextWave(operationsBlock, ReplaceString("%%OPERATIONNAME%%", opBlock, names[nameNr]))
	endfor
	return ReplaceKeyByTextWave(out1, "%%OPERATIONS%%", operationsBlock)
End

static Function/WAVE CreateErrorsHeader(errors)
	WAVE/T errors

	WAVE/T out = Load("ErrorsHeader.def")

	Make/FREE/T/N=(DimSize(errors, 0)) code
	code[] = PadString("#define " + errors[p][0], p < 10 ? 32 : 31, 0x20) + " " + num2str(4 + p) + " + FIRST_XOP_ERR"
	return ReplaceKeyByTextWave(out, "%%ERRORS%%", code)
End

static Function/WAVE GetErrors()

	variable numLines, lineNr, errorNr

	WAVE/T out = Load("errors.inp")

	numLines = DimSize(out, 0)
	if(mod(numLines, 2))
		Abort "Expected even number of lines for error defines and messages"
	endif
	Make/FREE/T/N=(numLines / 2) errorDef, errorMsg
	for(lineNr = 0; lineNr < numLines; lineNr += 2)
		errorNr = trunc(lineNr / 2)
		errorDef[errorNr] = out[lineNr]
		errorMsg[errorNr] = out[lineNr + 1]
	endfor

	Make/FREE/T/N=(DimSize(errorDef, 0), 2) result
	result[][0] = errorDef[p]
	result[][1] = errorMsg[p]

	return result
End

static Function/WAVE GetFunctionNames()

	variable pos, elem
	string funName

	WAVE/T out = Load("functionBodys.cpp")

	Make/FREE/T/N=0 funNames
	do
		FindValue/TEXT="extern \"C\" int "/TXOP=2 out
		if(V_Value == -1)
			break
		endif
		elem = V_Value
		pos = strsearch(out[elem], "(", 0)
		funName = out[elem]
		funName = funName[15, pos - 1]
		AddLineToTextWave(funNames, funName)
		DeletePoints 0, elem + 1, out
	while(1)

	return funNames
End

static Function/S GetProjectName()

	string pName
	variable pos1, pos2

	WAVE/T out = Load("CMakeLists.txt")

	FindValue/TEXT="PROJECT("/TXOP=0 out
	if(V_Value == -1)
		Abort "Could not find Project name in CMakeLists.txt"
	endif
	pName = out[V_Value]
	pos1 = strsearch(pName, "(", 0)
	pos2 = strsearch(pName, ")", 0)
	pName = pName[pos1 + 1, pos2 - 1]
	return trimString(pName)
End

static Function/WAVE CreateHandlerHeader(names, functionNames)
	WAVE/T names
	WAVE/T functionNames

	variable i, numNam
	string n

	WAVE/T out = Load("HandlerHeader.def")
	string opBlock = LoadString("HandlerHeaderOperationBlock.def")
	string funBlock = LoadString("HandlerHeaderFunctionBlock.def")

	Make/FREE/T/N=0 prototypesOp
	numNam = DimSize(names, 0)
	for(i = 0; i < numNam; i += 1)
		AddLineToTextWave(prototypesOp, ReplaceString("%%OPERATIONNAME%%", opBlock, names[i]))
	endfor

	Make/FREE/T/N=0 prototypesFun
	numNam = DimSize(functionNames, 0)
	for(i = 0; i < numNam; i += 1)
		AddLineToTextWave(prototypesFun, ReplaceString("%%FUNCTIONNAME%%", funBlock, functionNames[i]))
	endfor

	WAVE/T out1 = ReplaceKeyByTextWave(out, "%%OPERATIONS%%", prototypesOp)

	return ReplaceKeyByTextWave(out1, "%%FUNCTIONS%%", prototypesFun)
End

static Function/WAVE CreateHandlerCode(opFunBlock, functionNames, xopName)
	WAVE/T opFunBlock
	WAVE/T functionNames
	string xopName

	WAVE/T out = Load("HandlerTemplate.def")
	string headerBlock = LoadString("HandlerTemplateHeaderBlock.def")
	Make/FREE/T inc = { ReplaceString("%%XOPNAME%%", headerBlock, xopName) }

	WAVE/T functionMethods = CreateHandlerFunctions(functionNames)

	WAVE/T out1 = ReplaceKeyByTextWave(out, "%%HEADERINCLUDE%%", inc)
	WAVE/T out2 = ReplaceKeyByTextWave(out1, "%%OPFUNCTIONS%%", opFunBlock)
	return ReplaceKeyByTextWave(out2, "%%FUNCTIONS%%", functionMethods)
End

static Function/WAVE CreateOperationHeader(opBlock)
	WAVE/T opBlock

	WAVE/T out = Load("OperationsTemplate.def")

	return ReplaceKeyByTextWave(out, "%%OPERATIONSTRUCTS%%", opBlock)
End

static Function/WAVE CreateMainCode(names, regBlock, functionNames, xopName)
	WAVE/T names
	WAVE/T regBlock
	WAVE/T functionNames
	string xopName

	WAVE/T operationCalls = CreateOperationCalls(names)
	WAVE/T functionCalls = CreateFunctionCalls(functionNames)
	WAVE/T registerCalls = CreateRegisterCalls(names)

	WAVE/T out = Load("mainFileTemplate.def")
	string headerBlock = LoadString("mainFileTemplateHeaderIncludesBlock.def")

	Make/FREE/T includes = { ReplaceString("%%XOPNAME%%", headerBlock, xopName) }
	WAVE/T out1 = ReplaceKeyByTextWave(out, "%%HEADERINCLUDES%%", includes)
	WAVE/T out2 = ReplaceKeyByTextWave(out1, "%%OPERATIONS%%", operationCalls)
	WAVE/T out3 = ReplaceKeyByTextWave(out2, "%%OPREGISTRATIONFUNCTIONS%%", regBlock)
	WAVE/T out4 = ReplaceKeyByTextWave(out3, "%%FUNCTIONS%%", functionCalls)
	return ReplaceKeyByTextWave(out4, "%%REGISTRATION%%", registerCalls)
End

static Function/WAVE CreateOperationCalls(names)
	WAVE/T names

	variable i, numNam
	string opcBlock = LoadString("mainFileTemplateOperationCallsBlock.def")
	Make/FREE/T/N=0 out

	numNam = DimSize(names, 0)
	for(i = 0; i < numNam; i += 1)
		AddLineToTextWave(out, ReplaceString("%%OPERATIONNAME%%", opcBlock, names[i]))
	endfor

	return out
End

static Function/WAVE CreateFunctionCalls(names)
	WAVE/T names

	variable i, numNam
	string funcBlock = LoadString("mainFileTemplateFunctionCallsBlock.def")
	Make/FREE/T/N=0 out

	numNam = DimSize(names, 0)
	for(i = 0; i < numNam; i += 1)
		AddLineToTextWave(out, ReplaceString("%%FUNCTIONNAME%%", funcBlock, names[i]))
	endfor

	return out
End

static Function/WAVE CreateHandlerFunctions(names)
	WAVE/T names

	variable i, numNam
	string n
	string funcBlock = LoadString("HandlerTemplateFunctionsBlock.def")
	Make/FREE/T/N=0 out

	numNam = DimSize(names, 0)
	for(i = 0; i < numNam; i += 1)
		AddLineToTextWave(out, ReplaceString("%%FUNCTIONNAME%%", funcBlock, names[i]))
	endfor

	return out
End

static Function/WAVE CreateRegisterCalls(names)
	WAVE/T names

	variable i, numNam
	string oprBlock = LoadString("mainFileTemplateRegisterOperationsBlock.def")
	Make/FREE/T/N=0 out

	numNam = DimSize(names, 0)
	for(i = 0; i < numNam; i += 1)
		AddLineToTextWave(out, ReplaceString("%%OPERATIONNAME%%", oprBlock, names[i]))
	endfor

	return out
End

static Function/WAVE AddLineToTextWave(w, s)
	WAVE/T w
	string s

	variable size = DimSize(w, 0)
	Redimension/N=(size + 1) w
	w[size] = s
	return w
End

static Function/WAVE ReplaceKeyByTextWave(in, key, elem)
	WAVE/T in
	string key
	WAVE/T elem

	FindValue/TEXT=key/TXOP=4 in
	if(V_Value == -1)
		Abort "Could not find key to replace"
	endif
	if(V_Value - 1 <= 0)
		Make/FREE/T/N=0 out
	else
		Duplicate/FREE/T/R=[0, V_Value - 1] in, out
	endif
	if(V_Value + 1 >= DimSize(in, 0))
		Make/FREE/T/N=0 part2
	else
		Duplicate/FREE/T/R=[V_Value + 1,] in, part2
	endif

	WAVE/T out1 = AppendTextWave(out, elem)
	return AppendTextWave(out1, part2)
End

static Function/WAVE AppendTextWave(w1, w2)
	WAVE/T w1
	WAVE/T w2

	variable size1 = DimSize(w1, 0)
	variable size2 = DimSize(w2, 0)
	if(!size2)
		return w1
	endif
	Redimension/N=(size1 + size2) w1
	w1[size1,] = w2[p - size1]
	return w1
End

static Function WriteWave(code, fname)
	WAVE/T code
	string fname

	Save/J/M=LINE_ENDING/E=0/O/P=outputPath code as fname
End

/// @brief Returns one if str is empty or null, zero otherwise.
/// @param str must not be a SVAR
///
/// @hidecallgraph
/// @hidecallergraph
threadsafe static Function IsEmpty(str)
	string& str

	variable len = strlen(str)
	return numtype(len) == 2 || len <= 0
End
