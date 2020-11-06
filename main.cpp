#include "FileAnalizer.h"
#include "ArgumentManager.h"

#define INDEX_MAIN 0x0

typedef struct _SymbolCombo
{
	const char Symbol;
	const char Replacement;
} SymbolCombo, * PSymbolCombo;

GeneralErrorCast OFCallback(PCallsObject Instance, HANDLE FileHandle)
{
	WriteFile(FileHandle, Instance->Buffer, (Instance->BufferCount - 1), 0, 0);

	CloseHandle(FileHandle);

	return STATUS_SUCCESS;
}

GeneralErrorCast ConditionRR(char* Arguments, PCalls Calls, PArgumentInstance ArgumentInstance)
{
	Calls->CollapseBuffer(Calls->Instance, Arguments, strlen(Arguments), ' ');

	static Argument ParseArguments[] = { {ArgumentType_STRING, 0}, {ArgumentType_STRING, 0}, {ArgumentType_D64, 0} };
	if (!NT_SUCCESS(GENERAL_ERROR_NTSTATUS(SeperateArguments(ArgumentInstance, Calls, Arguments, ParseArguments, GetArraySize(ParseArguments), TRUE))))
	{
		CleanGarbage(&ArgumentInstance->ArgumentGarbage);
		return STATUS_SUCCESS;
	}

	unsigned long Step;

	Step = ParseArguments[2].Argument ? ParseArguments[2].Argument : 1;
	if (!ParseArguments[0].Argument || ((*((char*)ParseArguments[0].Argument) >= '0') && (*((char*)ParseArguments[0].Argument) <= '9')))
	{
		static Argument AltParseArguments[] = { {ArgumentType_D64, 0}, {ArgumentType_D64, 0} };

		FixArgumentTriming(Calls, Arguments, AltParseArguments, GetArraySize(AltParseArguments));
		if (!NT_SUCCESS(GENERAL_ERROR_NTSTATUS(SeperateArguments(ArgumentInstance, Calls, Arguments, AltParseArguments, GetArraySize(AltParseArguments), TRUE))))
		{
			CleanGarbage(&ArgumentInstance->ArgumentGarbage);
			return STATUS_SUCCESS;
		}

		char ReturnBuffer[32];

		long First = AltParseArguments[0].Argument;
		long Second = AltParseArguments[1].Argument;

		First += ((((rand() + (long)pow(rand(), (rand() % 3))) % (First - Second)) / Step) * Step);
		if (First > Second)
			First = Second;

		ltoa(First, ReturnBuffer, 10);
		Calls->Replace(Calls->Instance, ReturnBuffer, FALSE);
	}
	else
	{
		char* FirstArgument = ((char*)ParseArguments[0].Argument);
		char* SecondArgument = ((char*)ParseArguments[1].Argument);

		unsigned long long AlphabeticStart = 0;
		unsigned long long AlphabeticEnd = 0;

		char* Runtime;
		if (((*FirstArgument) >= 'A') && ((*FirstArgument) <= 'Z'))
		{
			Runtime = FirstArgument;
			for (unsigned long i = 0; (*Runtime); i++, Runtime++)
			{
				if (((*Runtime) < 'A') || ((*Runtime) > 'Z'))
				{
					CleanGarbage(&ArgumentInstance->ArgumentGarbage);
					return STATUS_SUCCESS;
				}

				AlphabeticStart += (((*Runtime) - 'A' + 1) * pow((('Z' - 'A') + 2), i));
			}

			Runtime = SecondArgument;
			for (unsigned long i = 0; (*Runtime); i++, Runtime++)
			{
				if (((*Runtime) < 'A') || ((*Runtime) > 'Z'))
				{
					CleanGarbage(&ArgumentInstance->ArgumentGarbage);
					return STATUS_SUCCESS;
				}

				AlphabeticEnd += (((*Runtime) - 'A' + 1) * pow((('Z' - 'A') + 2), i));
			}

			AlphabeticStart += (((((unsigned long long)rand() + (unsigned long long)pow(rand(), (rand() % 3))) % (AlphabeticEnd - AlphabeticStart)) / Step) * Step);
			if (AlphabeticStart > AlphabeticEnd)
				AlphabeticStart = AlphabeticEnd;

			char* RandomBuffer = ((char*)malloc(strlen(SecondArgument) + 1));

			Runtime = RandomBuffer;
			for (; AlphabeticStart; Runtime++, (*Runtime) = '\0', AlphabeticStart /= (('Z' - 'A') + 2))
			{
				if (!(AlphabeticStart % (('Z' - 'A') + 2)))
					AlphabeticStart += 1;

				(*Runtime) = ('A' + (AlphabeticStart % (('Z' - 'A') + 2)) - 1);
			}

			Calls->Replace(Calls->Instance, RandomBuffer, FALSE);
			free(RandomBuffer);
		}
		else if (((*Arguments) >= 'a') && ((*Arguments) <= 'z'))
		{
			Runtime = FirstArgument;
			for (unsigned long i = 0; (*Runtime); i++, Runtime++)
			{
				if (((*Runtime) < 'a') || ((*Runtime) > 'z'))
				{
					CleanGarbage(&ArgumentInstance->ArgumentGarbage);
					return STATUS_SUCCESS;
				}

				AlphabeticStart += (((*Runtime) - 'a' + 1) * pow((('z' - 'a') + 2), i));
			}

			Runtime = SecondArgument;
			for (unsigned long i = 0; (*Runtime); i++, Runtime++)
			{
				if (((*Runtime) < 'a') || ((*Runtime) > 'z'))
				{
					CleanGarbage(&ArgumentInstance->ArgumentGarbage);
					return STATUS_SUCCESS;
				}

				AlphabeticEnd += (((*Runtime) - 'a' + 1) * pow((('z' - 'a') + 2), i));
			}

			AlphabeticStart += (((((unsigned long long)rand()) % (AlphabeticEnd - AlphabeticStart)) / Step) * Step);
			if (AlphabeticStart > AlphabeticEnd)
				AlphabeticStart = AlphabeticEnd;

			char* RandomBuffer = ((char*)malloc(strlen(SecondArgument) + 1));

			Runtime = RandomBuffer;
			for (; (AlphabeticStart % (('z' - 'a') + 2)); Runtime++, (*Runtime) = '\0', AlphabeticStart /= (('z' - 'a') + 2))
				(*Runtime) = ('a' + (AlphabeticStart % (('z' - 'a') + 2)) - 1);

			Calls->Replace(Calls->Instance, RandomBuffer, FALSE);
			free(RandomBuffer);
		}
		else
		{
			CleanGarbage(&ArgumentInstance->ArgumentGarbage);
			return STATUS_SUCCESS;
		}
	}

	CleanGarbage(&ArgumentInstance->ArgumentGarbage);
	return STATUS_SUCCESS;
}

GeneralErrorCast ConditionRA(char* Arguments, PCalls Calls, PArgumentInstance ArgumentInstance)
{
	static Argument ParseArguments = { ArgumentType_STRING | ArgumentType_VARIADIC, 0 };
	if (!NT_SUCCESS(GENERAL_ERROR_NTSTATUS(SeperateArguments(ArgumentInstance, Calls, Arguments, &ParseArguments, 1, TRUE))))
	{
		CleanGarbage(&ArgumentInstance->ArgumentGarbage);
		return STATUS_SUCCESS;
	}

	PVariadicType Variadic;

	Variadic = ((PVariadicType)ParseArguments.Argument);

	const char* String = ((char*)Variadic->Arguments[(rand() + (unsigned long)pow(rand(), (rand() % 3))) % Variadic->ArgumentCount]);
	unsigned long Length = strlen(String) + 1;

	char* Copy = ((char*)malloc(Length));

	memcpy(Copy, String, Length);

	Calls->Replace(Calls->Instance, Copy, FALSE);
	free(Copy);

	CleanGarbage(&ArgumentInstance->ArgumentGarbage);
	return STATUS_SUCCESS;
}

GeneralErrorCast ConditionSF(char* Arguments, PCalls Calls, PArgumentInstance ArgumentInstance)
{
	const SymbolCombo StringCombos[] = { { 'n', '\n' }, { 'r', '\r' }, { '<', '<' }, { '>', '>' }, { '\"', '\"' }, { ',', ',' }, { '\\', '\\' } };
	static Argument ParseArguments[] = { { ArgumentType_STRING, 0 }, { ArgumentType_STRING, 0 }, { ArgumentType_STRING | ArgumentType_VARIADIC, 0 } };
	if (!NT_SUCCESS(GENERAL_ERROR_NTSTATUS(SeperateArguments(ArgumentInstance, Calls, Arguments, ParseArguments, GetArraySize(ParseArguments), TRUE))))
	{
		CleanGarbage(&ArgumentInstance->ArgumentGarbage);
		return STATUS_SUCCESS;
	}

	static char Combination[] = "\\\0";

	const SymbolCombo* RunCombos;
	VariadicType* Variadic;
	PVariable Variables;
	HANDLE FileHandle;

	unsigned long long* RunVariadic;

	unsigned long BreakerCount;
	unsigned long EntryCount;
	unsigned long NameLength;
	unsigned long FileSize;

	char** RunSeperaters;
	char* LocatedCombo;
	char* FileBuffer;
	char* LastEntry;
	char* RunFile;
	
	FileHandle = CreateFileA(((char*)ParseArguments[1].Argument), GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	FileSize = GetFileSize(FileHandle, 0);
	FileBuffer = ((char*)malloc(FileSize + 1));
	UpdateGarbage(&ArgumentInstance->VariableGarbage, FileBuffer, 0);

	ReadFile(FileHandle, FileBuffer, FileSize, 0, 0);
	CloseHandle(FileHandle);

	*(FileBuffer + FileSize) = '\0';

	if (!ArgumentInstance->VariableCount)
	{
		ArgumentInstance->Variables = ((PVariable)malloc(sizeof(Variable)));
		UpdateGarbage(&ArgumentInstance->VariableGarbage, ArgumentInstance->Variables, 0);
	}
	else
	{
		void* OldPointer;

		OldPointer = ArgumentInstance->Variables;
		ArgumentInstance->Variables = ((PVariable)realloc(ArgumentInstance->Variables, sizeof(Variable) * (ArgumentInstance->VariableCount + 1)));
		UpdateGarbage(&ArgumentInstance->VariableGarbage, ArgumentInstance->Variables, OldPointer);
	}

	Variables = &ArgumentInstance->Variables[ArgumentInstance->VariableCount];
	ArgumentInstance->VariableCount++;

	NameLength = strlen((char*)ParseArguments[0].Argument) + 1;
	Variables->VariableType = ArgumentType_STRING;
	Variables->VariableName = ((char*)malloc(NameLength));
	UpdateGarbage(&ArgumentInstance->VariableGarbage, Variables->VariableName, 0);
	memcpy(Variables->VariableName, ((char*)ParseArguments[0].Argument), NameLength);

	if (!ParseArguments[2].Argument)
	{
		EntryCount = 1;
		RunFile = FileBuffer;
		while (TRUE)
		{
			RunFile = strstr(RunFile, "\r\n");
			if (!RunFile)
				break;

			memset(RunFile, 0, 2);

			EntryCount++;
			RunFile += 2;
		}
	}
	else
	{
		Variadic = ((PVariadicType)ParseArguments[2].Argument);

		EntryCount = 1;
		RunSeperaters = ((char**)Variadic->Arguments);
		for (unsigned long i = 0; i < Variadic->ArgumentCount; i++, RunSeperaters++)
		{
			unsigned long RunEntryCount;
			unsigned long Length;
			char* Seperater;
			
			Length = strlen(*RunSeperaters);
			Seperater = ((char*)malloc(Length + 1));
			memcpy(Seperater, *RunSeperaters, (Length + 1));

			RunCombos = StringCombos;
			for (unsigned long ii = 0; ii < GetArraySize(StringCombos); ii++, RunCombos++)
			{
				Combination[1] = RunCombos->Symbol;

				LocatedCombo = Seperater;
				while (TRUE)
				{
					LocatedCombo = strstr(LocatedCombo, Combination);
					if (!LocatedCombo)
						break;

					Calls->CountBlockers(Calls->Instance, LocatedCombo, &BreakerCount);
					if (BreakerCount % 2)
					{
						unsigned long MoveLength;

						MoveLength = strlen(LocatedCombo + 2) + 1;
						memmove(LocatedCombo + 1, LocatedCombo + 2, MoveLength);
						(*LocatedCombo) = RunCombos->Replacement;
					}
					LocatedCombo++;
				}
			}

			Length = strlen(Seperater);

			RunFile = FileBuffer;
			RunEntryCount = EntryCount - 1;
			while (TRUE)
			{
				LastEntry = RunFile;
				RunFile = strstr(RunFile, Seperater);
				if (!RunFile)
				{
					if (!RunEntryCount)
						break;

					for (LastEntry += strlen(LastEntry); !*LastEntry; LastEntry++);
					RunEntryCount--;

					RunFile = LastEntry;
					continue;
				}

				memset(RunFile, 0, Length);
				if (*(RunFile - 1) && *(RunFile + 1))
					EntryCount++;

				RunFile += Length;
			}
			
			free(Seperater);
		}
	}

	Variables->Variable = ((PVariadicType)malloc(sizeof(VariadicType) - sizeof(unsigned long long) + sizeof(unsigned long long) * EntryCount));
	UpdateGarbage(&ArgumentInstance->VariableGarbage, Variables->Variable, 0);

	Variables->Variable->ArgumentCount = EntryCount;

	RunFile = FileBuffer;
	RunVariadic = Variables->Variable->Arguments;
	for (unsigned long i = 0; i < EntryCount; i++, RunVariadic++)
	{
		(*RunVariadic) = ((unsigned long long)RunFile);
		for (RunFile += strlen(RunFile); !*RunFile; RunFile++);
	}

	Calls->Remove(Calls->Instance);
	CleanGarbage(&ArgumentInstance->ArgumentGarbage);
	return STATUS_SUCCESS;
}

GeneralErrorCast ConditionSP(char* Arguments, PCalls Calls, PArgumentInstance ArgumentInstance)
{
	const SymbolCombo StringCombos[] = { { 'n', '\n' }, { '<', '<' }, { '>', '>' }, { '\"', '\"' }, { ',', ',' }, { '\\', '\\' } };

	static Argument ParseArguments[] = { { ArgumentType_STRING, 0}, {ArgumentType_D64, 0} };
	if (!NT_SUCCESS(GENERAL_ERROR_NTSTATUS(SeperateArguments(ArgumentInstance, Calls, Arguments, ParseArguments, GetArraySize(ParseArguments), TRUE))))
	{
		CleanGarbage(&ArgumentInstance->ArgumentGarbage);
		return STATUS_SUCCESS;
	}

	static char Combination[] = "\\\0";

	const SymbolCombo* RunCombos;
	unsigned long CopyLength;
	unsigned long Count;
	char* LocatedCombo;
	char* CopyRuntime;
	char* Copies;

	CopyLength = strlen((char*)ParseArguments[0].Argument);
	Copies = ((char*)malloc(ParseArguments[1].Argument * CopyLength + 1));

	memcpy(Copies, ((char*)ParseArguments[0].Argument), CopyLength);
	(*(Copies + CopyLength)) = '\0';

	RunCombos = StringCombos;
	for (unsigned long i = 0; i < GetArraySize(StringCombos); i++, RunCombos++)
	{
		Combination[1] = RunCombos->Symbol;

		LocatedCombo = Copies;
		while (TRUE)
		{
			LocatedCombo = strstr(LocatedCombo, Combination);
			if (!LocatedCombo)
				break;

			Calls->CountBlockers(Calls->Instance, LocatedCombo, &Count);
			if (Count % 2)
			{
				unsigned long MoveLength;

				MoveLength = strlen(LocatedCombo + 2) + 1;
				memmove(LocatedCombo + 1, LocatedCombo + 2, MoveLength);
				(*LocatedCombo) = RunCombos->Replacement;

				CopyLength--;
			}
			LocatedCombo++;
		}
	}

	CopyRuntime = Copies + CopyLength;
	for (unsigned long i = 1; i < ParseArguments[1].Argument; i++, CopyRuntime += CopyLength)
		memcpy(CopyRuntime, Copies, CopyLength);

	(*CopyRuntime) = '\0';

	Calls->Replace(Calls->Instance, Copies, TRUE);
	free(Copies);

	Calls->Restart(Calls->Instance);

	CleanGarbage(&ArgumentInstance->ArgumentGarbage);
	return STATUS_SUCCESS;
}

GeneralErrorCast ConditionOF(char* Arguments, PCalls Calls, PArgumentInstance ArgumentInstance)
{
	static Argument ParseArguments = { ArgumentType_STRING, 0 };
	if (!NT_SUCCESS(GENERAL_ERROR_NTSTATUS(SeperateArguments(ArgumentInstance, Calls, Arguments, &ParseArguments, 1, TRUE))))
	{
		CleanGarbage(&ArgumentInstance->ArgumentGarbage);
		return STATUS_SUCCESS;
	}

	HANDLE FileHandle;

	FileHandle = CreateFileA(((char*)(ParseArguments.Argument)), GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);

	Calls->QueFinishFunction(Calls->Instance, OFCallback, FileHandle);
	Calls->Remove(Calls->Instance);

	CleanGarbage(&ArgumentInstance->ArgumentGarbage);
	return STATUS_SUCCESS;
}

GeneralErrorCast ConditionCB(char* Arguments, PCalls Calls)
{
	Calls->Remove(Calls->Instance);

	char* LineBreaks = Calls->Instance->CurrentLocation - 1;
	for (; ((*LineBreaks) == '\n') || ((*((unsigned short*)LineBreaks)) == *((unsigned short*)"\r\n")) || ((*LineBreaks) == ' '); LineBreaks--);

	char Replaced = (*Calls->Instance->CurrentLocation);
	(*Calls->Instance->CurrentLocation) = '\0';
	unsigned long ShiftedLength = strlen(LineBreaks + 1);
	(*Calls->Instance->CurrentLocation) = Replaced;

	Calls->ShiftBuffer(Calls->Instance, LineBreaks + 1, ShiftedLength);
	return STATUS_SUCCESS;
}

GeneralErrorCast DFCallback(PCallsObject Instance, char* Path)
{
	ShellExecuteA(0, 0, Path, 0, 0, SW_SHOW);
	free(Path);

	return STATUS_SUCCESS;
}

GeneralErrorCast ConditionDF(char* Arguments, PCalls Calls, PArgumentInstance ArgumentInstance)
{
	static Argument ParseArguments = { ArgumentType_STRING, 0 };
	if (!NT_SUCCESS(GENERAL_ERROR_NTSTATUS(SeperateArguments(ArgumentInstance, Calls, Arguments, &ParseArguments, 1, TRUE))))
	{
		CleanGarbage(&ArgumentInstance->ArgumentGarbage);
		return STATUS_SUCCESS;
	}

	unsigned long PathLength;
	char* Path;

	PathLength = strlen((char*)ParseArguments.Argument) + 1;
	Path = ((char*)malloc(PathLength));

	memcpy(Path, ((void*)ParseArguments.Argument), PathLength);

	Calls->QueFinishFunction(Calls->Instance, ((QueFunction)DFCallback), Path);
	Calls->Remove(Calls->Instance);

	CleanGarbage(&ArgumentInstance->ArgumentGarbage);
	return STATUS_SUCCESS;
}

GeneralErrorCast CallbackDD(PCallsObject Instance, char* Path)
{
	if (Path)
	{
		ShellExecuteA(0, "open", Path, 0, 0, SW_SHOWDEFAULT);
		free(Path);
	}
	else
	{
		char Directory[MAX_PATH];

		GetCurrentDirectoryA(GetArraySize(Directory), Directory);
		ShellExecuteA(0, "open", Directory, 0, 0, SW_SHOWDEFAULT);
	}

	return STATUS_SUCCESS;
}

GeneralErrorCast ConditionDD(char* Arguments, PCalls Calls, PArgumentInstance ArgumentInstance)
{
	static Argument ParseArguments = { ArgumentType_STRING, 0 };
	SeperateArguments(ArgumentInstance, Calls, Arguments, &ParseArguments, 1, TRUE);

	unsigned long PathLength;
	char* Path;

	if (ParseArguments.Argument)
	{
		PathLength = strlen((char*)ParseArguments.Argument) + 1;
		Path = ((char*)malloc(PathLength));

		memcpy(Path, ((void*)ParseArguments.Argument), PathLength);
	}
	else
		Path = 0;

	Calls->QueFinishFunction(Calls->Instance, ((QueFunction)CallbackDD), Path);
	Calls->Remove(Calls->Instance);

	CleanGarbage(&ArgumentInstance->ArgumentGarbage);
	return STATUS_SUCCESS;
}

GeneralErrorCast ConditionSD(char* Arguments, PCalls Calls, PArgumentInstance ArgumentInstance)
{
	static Argument ParseArguments[] = { { ArgumentType_STRING, 0 }, {ArgumentType_DOUBLE | ArgumentType_VARIADIC, 0} };
	if (!NT_SUCCESS(GENERAL_ERROR_NTSTATUS(SeperateArguments(ArgumentInstance, Calls, Arguments, ParseArguments, GetArraySize(ParseArguments), TRUE))))
	{
		CleanGarbage(&ArgumentInstance->ArgumentGarbage);
		return STATUS_SUCCESS;
	}

	if (!ArgumentInstance->VariableCount)
	{
		ArgumentInstance->Variables = ((PVariable)malloc(sizeof(Variable)));
		UpdateGarbage(&ArgumentInstance->VariableGarbage, ArgumentInstance->Variables, 0);
	}
	else
	{
		void* OldPointer;

		OldPointer = ArgumentInstance->Variables;
		ArgumentInstance->Variables = ((PVariable)realloc(ArgumentInstance->Variables, sizeof(Variable) * (ArgumentInstance->VariableCount + 1)));
		UpdateGarbage(&ArgumentInstance->VariableGarbage, ArgumentInstance->Variables, OldPointer);
	}

	unsigned long CopyLength;
	PVariable Variable;

	Variable = &ArgumentInstance->Variables[ArgumentInstance->VariableCount];

	ArgumentInstance->VariableCount++;

	CopyLength = strlen(((char*)ParseArguments->Argument)) + 1;
	Variable->VariableName = ((char*)malloc(CopyLength));
	UpdateGarbage(&ArgumentInstance->VariableGarbage, Variable->VariableName, 0);

	memcpy(Variable->VariableName, ((char*)ParseArguments->Argument), CopyLength);

	Variable->VariableType = ArgumentType_DOUBLE;
	Variable->Variable = ((PVariadicType)malloc(sizeof(VariadicType) - sizeof(unsigned long long) + sizeof(unsigned long long) * ((PVariadicType)ParseArguments[1].Argument)->ArgumentCount));
	UpdateGarbage(&ArgumentInstance->VariableGarbage, Variable->Variable, 0);

	memcpy(Variable->Variable, ((void*)(ParseArguments[1].Argument)), (sizeof(VariadicType) - sizeof(unsigned long long) + sizeof(unsigned long long) * ((PVariadicType)ParseArguments[1].Argument)->ArgumentCount));

	Calls->Remove(Calls->Instance);

	CleanGarbage(&ArgumentInstance->ArgumentGarbage);
	return STATUS_SUCCESS;
}

GeneralErrorCast ConditionSS(char* Arguments, PCalls Calls, PArgumentInstance ArgumentInstance)
{
	static Argument ParseArguments[] = { { ArgumentType_STRING, 0 }, {ArgumentType_STRING | ArgumentType_VARIADIC, 0} };
	if (!NT_SUCCESS(GENERAL_ERROR_NTSTATUS(SeperateArguments(ArgumentInstance, Calls, Arguments, ParseArguments, GetArraySize(ParseArguments), TRUE))))
	{
		CleanGarbage(&ArgumentInstance->ArgumentGarbage);
		return STATUS_SUCCESS;
	}

	if (!ArgumentInstance->VariableCount)
	{
		ArgumentInstance->Variables = ((PVariable)malloc(sizeof(Variable)));
		UpdateGarbage(&ArgumentInstance->VariableGarbage, ArgumentInstance->Variables, 0);
	}
	else
	{
		void* OldPointer;

		OldPointer = ArgumentInstance->Variables;
		ArgumentInstance->Variables = ((PVariable)realloc(ArgumentInstance->Variables, sizeof(Variable) * (ArgumentInstance->VariableCount + 1)));
		UpdateGarbage(&ArgumentInstance->VariableGarbage, ArgumentInstance->Variables, OldPointer);
	}

	unsigned long CopyLength;
	PVariable Variable;

	Variable = &ArgumentInstance->Variables[ArgumentInstance->VariableCount];

	ArgumentInstance->VariableCount++;

	CopyLength = strlen(((char*)ParseArguments->Argument)) + 1;
	Variable->VariableName = ((char*)malloc(CopyLength));
	UpdateGarbage(&ArgumentInstance->VariableGarbage, Variable->VariableName, 0);

	memcpy(Variable->VariableName, ((char*)ParseArguments->Argument), CopyLength);

	Variable->VariableType = ArgumentType_STRING;
	Variable->Variable = ((PVariadicType)malloc(sizeof(VariadicType) - sizeof(unsigned long long) + sizeof(unsigned long long) * ((PVariadicType)ParseArguments[1].Argument)->ArgumentCount));
	UpdateGarbage(&ArgumentInstance->VariableGarbage, Variable->Variable, 0);

	memcpy(Variable->Variable, ((void*)(ParseArguments[1].Argument)), (sizeof(VariadicType) - sizeof(unsigned long long) + sizeof(unsigned long long) * ((PVariadicType)ParseArguments[1].Argument)->ArgumentCount));

	unsigned long long* RunVariadic = Variable->Variable->Arguments;
	for (unsigned long i = 0; i < Variable->Variable->ArgumentCount; i++, RunVariadic++)
	{
		unsigned long long String;
		unsigned long Length;
		
		Length = strlen((char*)*RunVariadic) + 1;
		String = ((unsigned long long)malloc(Length));
		UpdateGarbage(&ArgumentInstance->VariableGarbage, ((void*)String), 0);

		memcpy(((char*)String), ((char*)*RunVariadic), Length);

		(*RunVariadic) = String;
	}

	Calls->Remove(Calls->Instance);

	CleanGarbage(&ArgumentInstance->ArgumentGarbage);
	return STATUS_SUCCESS;
}

GeneralErrorCast ConditionSI(char* Arguments, PCalls Calls, PArgumentInstance ArgumentInstance)
{
	static Argument ParseArguments[] = { { ArgumentType_STRING, 0 }, {ArgumentType_D64 | ArgumentType_VARIADIC, 0} };
	if (!NT_SUCCESS(GENERAL_ERROR_NTSTATUS(SeperateArguments(ArgumentInstance, Calls, Arguments, ParseArguments, GetArraySize(ParseArguments), TRUE))))
	{
		CleanGarbage(&ArgumentInstance->ArgumentGarbage);
		return STATUS_SUCCESS;
	}

	if (!ArgumentInstance->VariableCount)
	{
		ArgumentInstance->Variables = ((PVariable)malloc(sizeof(Variable)));
		UpdateGarbage(&ArgumentInstance->VariableGarbage, ArgumentInstance->Variables, 0);
	}
	else
	{
		void* OldPointer;

		OldPointer = ArgumentInstance->Variables;
		ArgumentInstance->Variables = ((PVariable)realloc(ArgumentInstance->Variables, sizeof(Variable) * (ArgumentInstance->VariableCount + 1)));
		UpdateGarbage(&ArgumentInstance->VariableGarbage, ArgumentInstance->Variables, OldPointer);
	}

	unsigned long CopyLength;
	PVariable Variable;

	Variable = &ArgumentInstance->Variables[ArgumentInstance->VariableCount];

	ArgumentInstance->VariableCount++;

	CopyLength = strlen(((char*)ParseArguments->Argument)) + 1;
	Variable->VariableName = ((char*)malloc(CopyLength));
	UpdateGarbage(&ArgumentInstance->VariableGarbage, Variable->VariableName, 0);

	memcpy(Variable->VariableName, ((char*)ParseArguments->Argument), CopyLength);

	Variable->VariableType = ArgumentType_D64;
	Variable->Variable = ((PVariadicType)malloc(sizeof(VariadicType) - sizeof(unsigned long long) + sizeof(unsigned long long) * ((PVariadicType)ParseArguments[1].Argument)->ArgumentCount));
	UpdateGarbage(&ArgumentInstance->VariableGarbage, Variable->Variable, 0);

	memcpy(Variable->Variable, ((void*)(ParseArguments[1].Argument)), (sizeof(VariadicType) - sizeof(unsigned long long) + sizeof(unsigned long long) * ((PVariadicType)ParseArguments[1].Argument)->ArgumentCount));

	Calls->Remove(Calls->Instance);

	CleanGarbage(&ArgumentInstance->ArgumentGarbage);
	return STATUS_SUCCESS;
}

int main()
{
	HANDLE FileHandle;
	ArgumentInstance ArgumentInstance;

	void* FileBuffer;
	unsigned long FileSize;

	srand(GetTickCount() % RAND_MAX);

	FileHandle = CreateFileA("Settings.txt", GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	FileSize = GetFileSize(FileHandle, 0);
	FileBuffer = malloc(FileSize + 1);
	ReadFile(FileHandle, FileBuffer, FileSize, 0, 0);

	*((char*)FileBuffer + FileSize) = '\0';

	CloseHandle(FileHandle);

	memset(&ArgumentInstance, 0, sizeof(ArgumentInstance));

	static Condition Conditions[11];

	Conditions[0].EndCondition = ">";
	Conditions[0].StartCondition = "<SI";
	Conditions[0].Conditional = ((ConditionCall)ConditionSI);
	Conditions[0].Reserved = &ArgumentInstance;

	Conditions[1].EndCondition = ">";
	Conditions[1].StartCondition = "<SS";
	Conditions[1].Conditional = ((ConditionCall)ConditionSS);
	Conditions[1].Reserved = &ArgumentInstance;

	Conditions[2].EndCondition = ">";
	Conditions[2].StartCondition = "<SD";
	Conditions[2].Conditional = ((ConditionCall)ConditionSD);
	Conditions[2].Reserved = &ArgumentInstance;

	Conditions[3].EndCondition = ">";
	Conditions[3].StartCondition = "<RR";
	Conditions[3].Conditional = ((ConditionCall)ConditionRR);
	Conditions[3].Reserved = &ArgumentInstance;

	Conditions[4].EndCondition = ">";
	Conditions[4].StartCondition = "<RA";
	Conditions[4].Conditional = ((ConditionCall)ConditionRA);
	Conditions[4].Reserved = &ArgumentInstance;

	Conditions[5].EndCondition = ">";
	Conditions[5].StartCondition = "<SP";
	Conditions[5].Conditional = ((ConditionCall)ConditionSP);
	Conditions[5].Reserved = &ArgumentInstance;

	Conditions[6].EndCondition = ">";
	Conditions[6].StartCondition = "<OF";
	Conditions[6].Conditional = ((ConditionCall)ConditionOF);
	Conditions[6].Reserved = &ArgumentInstance;

	Conditions[7].EndCondition = ">";
	Conditions[7].StartCondition = "<SF";
	Conditions[7].Conditional = ((ConditionCall)ConditionSF);
	Conditions[7].Reserved = &ArgumentInstance;

	Conditions[8].EndCondition = ">";
	Conditions[8].StartCondition = "<CB";
	Conditions[8].Conditional = ((ConditionCall)ConditionCB);
	Conditions[8].Reserved = &ArgumentInstance;

	Conditions[9].EndCondition = ">";
	Conditions[9].StartCondition = "<DF";
	Conditions[9].Conditional = ((ConditionCall)ConditionDF);
	Conditions[9].Reserved = &ArgumentInstance;

	Conditions[10].EndCondition = ">";
	Conditions[10].StartCondition = "<DD";
	Conditions[10].Conditional = ((ConditionCall)ConditionDD);
	Conditions[10].Reserved = &ArgumentInstance;

	Analize(((char**)&FileBuffer), Conditions, GetArraySize(Conditions));

	CleanGarbage(&ArgumentInstance.VariableGarbage);

	free(FileBuffer);
}