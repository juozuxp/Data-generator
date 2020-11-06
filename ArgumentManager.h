#pragma once
#include "Imports.h"
#include <stdio.h>

#define INDEX_ARGUMENT_MANAGER 0x1

typedef enum _ArgumentType
{
	ArgumentType_B64,
	ArgumentType_D64,
	ArgumentType_H64,
	ArgumentType_DOUBLE,
	ArgumentType_STRING,
	ArgumentType_VARIADIC = (1 << (8 - 1))
} ArgumentType, * PArgumentType;

typedef struct _VariadicType
{
	unsigned long ArgumentCount;
	unsigned long long Arguments[1];
} VariadicType, * PVariadicType;

typedef struct _Argument
{
	unsigned char ArgumentType;
	unsigned long long Argument;
} Argument, * PArgument;

typedef struct _Variable
{
	char* VariableName;
	unsigned char VariableType;
	PVariadicType Variable;
} Variable, * PVariable;

typedef struct _HeapGarbage
{
	unsigned long Count;
	unsigned long Max;
	void** Pointers;
} HeapGarbage, * PHeapGarbage;

typedef struct _ArgumentInstance
{
	PVariable Variables;
	unsigned long VariableCount;

	HeapGarbage ArgumentGarbage;
	HeapGarbage VariableGarbage;
} ArgumentInstance, * PArgumentInstance;

static GeneralErrorCast UpdateGarbage(HeapGarbage* Garbage, void* NewPointer, void* OldPointer)
{
	if (!Garbage->Max)
	{
		Garbage->Pointers = ((void**)malloc(sizeof(void*)));
		(*Garbage->Pointers) = NewPointer;

		Garbage->Max = 1;
		Garbage->Count = 1;

		return STATUS_SUCCESS;
	}

	if (OldPointer)
	{
		void** RunPointers = Garbage->Pointers;
		for (unsigned long i = 0; i < Garbage->Count; i++, RunPointers++)
		{
			if (*RunPointers == OldPointer)
			{
				*RunPointers = NewPointer;

				return STATUS_SUCCESS;
			}
		}
		return GENERAL_ERROR_ASSEMBLE(INDEX_ARGUMENT_MANAGER, STATUS_NOT_FOUND);
	}
	else
	{
		Garbage->Count++;

		if (Garbage->Count > Garbage->Max)
		{
			Garbage->Pointers = ((void**)realloc(Garbage->Pointers, sizeof(void*) * Garbage->Count));
			Garbage->Max = Garbage->Count;
		}

		Garbage->Pointers[Garbage->Count - 1] = NewPointer;
	}

	return STATUS_SUCCESS;
}

static GeneralErrorCast CleanGarbage(HeapGarbage* Garbage)
{
	void** RunPointer;

	RunPointer = Garbage->Pointers;
	for (unsigned long i = 0; i < Garbage->Count; i++, RunPointer++)
		free(*RunPointer);

	Garbage->Count = 0;
	return STATUS_SUCCESS;
}

static GeneralErrorCast CastVariables(PArgumentInstance ParserInstance, unsigned long long Variable, unsigned char VarType, unsigned char CastType, unsigned long long* Casted)
{
	VarType &= (ArgumentType_VARIADIC - 1);
	CastType &= (ArgumentType_VARIADIC - 1);

	switch (VarType)
	{
	case ArgumentType_B64:
	case ArgumentType_D64:
	case ArgumentType_H64:
	{
		switch (CastType)
		{
		case ArgumentType_B64:
		case ArgumentType_D64:
		case ArgumentType_H64:
		{
			(*Casted) = Variable;
		} break;
		case ArgumentType_DOUBLE:
		{
			double Temp;

			Temp = Variable;
			(*Casted) = *((unsigned long long*) & Temp);
		} break;
		case ArgumentType_STRING:
		{
			(*Casted) = ((unsigned long long)malloc(65)); // unsigned long long is 8 bytes which is 64 bits, to display every bit seperatly you'd have to have 64 chars at your disposal + 1 for null trimming
			UpdateGarbage(&ParserInstance->ArgumentGarbage, ((void*)(*Casted)), 0);

			_ui64toa(Variable, ((char*)(*Casted)), CastType == ArgumentType_B64 ? 2 : (CastType == ArgumentType_D64 ? 10 : 16));
		} break;
		}
	} break;
	case ArgumentType_DOUBLE:
	{
		switch (CastType)
		{
		case ArgumentType_B64:
		case ArgumentType_D64:
		case ArgumentType_H64:
		{
			unsigned long long Localized;

			Localized = Variable;
			(*Casted) = *((double*)&Localized);
		} break;
		case ArgumentType_DOUBLE:
		{
			(*Casted) = Variable;
		} break;
		case ArgumentType_STRING:
		{
			(*Casted) = ((unsigned long long)malloc(65)); // I can't be bothered to change the 65, stfu
			UpdateGarbage(&ParserInstance->ArgumentGarbage, ((void*)(*Casted)), 0);

			sprintf(((char*)(*Casted)), "%d", Variable);
		} break;
		}
	} break;
	case ArgumentType_STRING:
	{
		switch (CastType)
		{
		case ArgumentType_B64:
		case ArgumentType_D64:
		case ArgumentType_H64:
		{
			(*Casted) = strtoull(((char*)Variable), 0, CastType == ArgumentType_B64 ? 2 : (CastType == ArgumentType_D64 ? 10 : 16));
		} break;
		case ArgumentType_DOUBLE:
		{
			(*Casted) = strtod(((char*)Variable), 0);
		} break;
		case ArgumentType_STRING:
		{
			(*Casted) = Variable;
		} break;
		}
	} break;
	}

	return STATUS_SUCCESS;
}

static GeneralErrorCast SeperateArguments(PArgumentInstance ParserInstance, PCalls Calls, char* StringArguments, PArgument Arguments, unsigned long ArgumentCount, BOOLEAN TrimArguments)
{
	void* OldPointer;
	char* LastReplace;
	unsigned long BreakCount;

	PArgument RunArguments;

	RunArguments = Arguments;
	for (unsigned long i = 0; i < ArgumentCount; i++, RunArguments++)
		RunArguments->Argument = 0;

	if (!strstr(StringArguments, "="))
		return GENERAL_ERROR_ASSEMBLE(INDEX_ARGUMENT_MANAGER, STATUS_INVALID_MEMBER);

	char* ArgumentStart = strstr(StringArguments, "[");
	if (!ArgumentStart)
		return GENERAL_ERROR_ASSEMBLE(INDEX_ARGUMENT_MANAGER, STATUS_INVALID_MEMBER);

	char* ArgumentEnd = Calls->Instance->EndLocation;
	for (; (*ArgumentEnd != '[') && (*ArgumentEnd != ']'); ArgumentEnd--);

	if (*ArgumentEnd == '[')
		return GENERAL_ERROR_ASSEMBLE(INDEX_ARGUMENT_MANAGER, STATUS_INVALID_MEMBER);

	(*ArgumentEnd) = '\0';

	LastReplace = ArgumentStart;
	RunArguments = Arguments;
	for (unsigned long i = 0; (i < ArgumentCount) && (*ArgumentStart); i++, RunArguments++, ArgumentStart++)
	{
		if (RunArguments->ArgumentType & ArgumentType_VARIADIC)
			break;

		BreakCount = 1;
		LastReplace = ArgumentStart;
		while (BreakCount % 2)
		{
			LastReplace = strstr(LastReplace + 1, ",");
			if (!LastReplace)
				break;

			Calls->CountBlockers(Calls->Instance, LastReplace - 1, &BreakCount);
		}

		if (LastReplace)
			(*LastReplace) = '\0';

		switch (RunArguments->ArgumentType)
		{
		case ArgumentType_B64:
		case ArgumentType_D64:
		case ArgumentType_H64:
		case ArgumentType_DOUBLE:
		{
			BOOLEAN Parse;
			PVariable RunVariables;

			Parse = TRUE;
			RunVariables = ParserInstance->Variables;
			for (unsigned long ii = 0; ii < ParserInstance->VariableCount; ii++, RunVariables++)
			{
				if (strstr(ArgumentStart, RunVariables->VariableName))
				{
					unsigned long long* RunVVariables;
					PVariadicType VVariables;

					VVariables = RunVariables->Variable;
					RunVVariables = VVariables->Arguments;
					for (ii = 0; ii < VVariables->ArgumentCount && i < ArgumentCount; i++, ii++, RunVVariables++, RunArguments++)
					{
						if (RunArguments->ArgumentType & ArgumentType_VARIADIC)
							break;

						CastVariables(ParserInstance, *RunVVariables, RunVariables->VariableType, RunArguments->ArgumentType, &RunArguments->Argument);
					}

					if ((RunArguments->ArgumentType & ArgumentType_VARIADIC) && (ii < VVariables->ArgumentCount))
					{
						PVariadicType VArguments;
						unsigned long long* RunVArguments;

						VArguments = ((PVariadicType)malloc(sizeof(VariadicType) - sizeof(unsigned long long) + sizeof(unsigned long long) * (VVariables->ArgumentCount - ii)));

						UpdateGarbage(&ParserInstance->ArgumentGarbage, VArguments, 0);

						VArguments->ArgumentCount = (VVariables->ArgumentCount - ii);

						RunVArguments = VArguments->Arguments;
						for (; ii < VArguments->ArgumentCount; RunVVariables++, RunVArguments++)
							CastVariables(ParserInstance, *RunVVariables, RunVariables->VariableType, RunArguments->ArgumentType, RunVArguments);

						RunArguments->Argument = ((unsigned long long)VArguments);
					}
					else
					{
						i--;
						RunArguments--;
					}

					break;
				}
			}

			if (Parse)
			{
				for (; (*ArgumentStart) && ((*ArgumentStart < '0') || (*ArgumentStart > '9')); ArgumentStart++);
				if (!*ArgumentStart)
					break;

				CastVariables(ParserInstance, ((unsigned long long)ArgumentStart), ArgumentType_STRING, RunArguments->ArgumentType, &RunArguments->Argument);
			}
		} break;
		case ArgumentType_STRING:
		{
			char* StringEnd;
			unsigned long Count;

			Count = 1;
			RunArguments->Argument = ((unsigned long long)ArgumentStart);
			while (Count % 2)
			{
				RunArguments->Argument = ((unsigned long long)strstr(((char*)RunArguments->Argument), "\""));
				if (!RunArguments->Argument)
					break;

				Calls->CountBlockers(Calls->Instance, ((char*)(RunArguments->Argument - 1)), &Count);
				RunArguments->Argument++;
			}

			if (!RunArguments->Argument)
			{
				PVariable RunVariables;

				RunVariables = ParserInstance->Variables;
				for (unsigned long ii = 0; ii < ParserInstance->VariableCount; ii++, RunVariables++)
				{
					if (strstr(ArgumentStart, RunVariables->VariableName))
					{
						unsigned long long* RunVVariables;
						PVariadicType VVariables;

						VVariables = RunVariables->Variable;
						RunVVariables = VVariables->Arguments;
						for (ii = 0; ii < VVariables->ArgumentCount && i < ArgumentCount; i++, ii++, RunVVariables++, RunArguments++)
						{
							if (RunArguments->ArgumentType & ArgumentType_VARIADIC)
								break;

							CastVariables(ParserInstance, *RunVVariables, RunVariables->VariableType, RunArguments->ArgumentType, &RunArguments->Argument);
						}

						if ((RunArguments->ArgumentType & ArgumentType_VARIADIC) && (ii < VVariables->ArgumentCount))
						{
							PVariadicType VArguments;
							unsigned long long* RunVArguments;

							VArguments = ((PVariadicType)malloc(sizeof(VariadicType) - sizeof(unsigned long long) + sizeof(unsigned long long) * (VVariables->ArgumentCount - ii)));

							UpdateGarbage(&ParserInstance->ArgumentGarbage, VArguments, 0);

							VArguments->ArgumentCount = (VVariables->ArgumentCount - ii);

							RunVArguments = VArguments->Arguments;
							for (; ii < VVariables->ArgumentCount; ii++, RunVVariables++, RunVArguments++)
								CastVariables(ParserInstance, *RunVVariables, RunVariables->VariableType, RunArguments->ArgumentType, RunVArguments);

							RunArguments->Argument = ((unsigned long long)VArguments);
						}
						else
						{
							i--;
							RunArguments--;
						}

						break;
					}
				}
				break;
			}

			Count = 1;
			StringEnd = ((char*)RunArguments->Argument) - 1;
			while (Count % 2)
			{
				StringEnd = strstr(StringEnd + 1, "\"");
				if (!StringEnd)
					break;

				Calls->CountBlockers(Calls->Instance, (StringEnd - 1), &Count);
			}

			if (!StringEnd)
			{
				RunArguments->Argument = 0;
				break;
			}

			if (TrimArguments)
				(*StringEnd) = '\0';

			ArgumentStart += (StringEnd - ((char*)RunArguments->Argument) + 3);
		} break;
		}

		if (!LastReplace)
			break;

		if (!TrimArguments)
			if (LastReplace)
				(*LastReplace) = ',';

		ArgumentStart = LastReplace;
	}

	if ((RunArguments->ArgumentType & ArgumentType_VARIADIC) && LastReplace)
	{
		PVariadicType VAArgs;

		unsigned long long* RunVAArgs;
		unsigned long ArgumentCount;
		unsigned long StartIndex;
		char* RunEvaluation;

		ArgumentCount = 1;
		RunEvaluation = ArgumentStart;
		while (TRUE)
		{
			RunEvaluation = strstr(RunEvaluation + 1, ",");
			if (!RunEvaluation)
				break;

			Calls->CountBlockers(Calls->Instance, RunEvaluation - 1, &BreakCount);
			if (!(BreakCount % 2))
				ArgumentCount++;
		}

		StartIndex = 0;
		if (RunArguments->Argument)
		{
			OldPointer = ((void*)RunArguments->Argument);
			VAArgs = ((PVariadicType)realloc(((char*)RunArguments->Argument), sizeof(VariadicType) - sizeof(unsigned long long) + sizeof(unsigned long long) * (((PVariadicType)(RunArguments->Argument))->ArgumentCount + ArgumentCount)));
			UpdateGarbage(&ParserInstance->ArgumentGarbage, VAArgs, OldPointer);

			StartIndex = VAArgs->ArgumentCount;
			VAArgs->ArgumentCount += ArgumentCount;
		}
		else
		{
			VAArgs = ((PVariadicType)malloc(sizeof(VariadicType) - sizeof(unsigned long long) + sizeof(unsigned long long) * ArgumentCount));
			UpdateGarbage(&ParserInstance->ArgumentGarbage, VAArgs, 0);

			VAArgs->ArgumentCount = ArgumentCount;
		}

		RunVAArgs = VAArgs->Arguments + StartIndex;
		for (unsigned long i = StartIndex; i < VAArgs->ArgumentCount && (*ArgumentStart); i++, RunVAArgs++, ArgumentStart++)
		{
			BreakCount = 1;
			LastReplace = ArgumentStart;
			while (BreakCount % 2)
			{
				LastReplace = strstr(LastReplace + 1, ",");
				if (!LastReplace)
					break;

				Calls->CountBlockers(Calls->Instance, LastReplace - 1, &BreakCount);
			}

			if (LastReplace)
				(*LastReplace) = '\0';

			switch (RunArguments->ArgumentType & (ArgumentType_VARIADIC - 1))
			{
			case ArgumentType_B64:
			case ArgumentType_D64:
			case ArgumentType_H64:
			case ArgumentType_DOUBLE:
			{
				BOOLEAN Parse;
				PVariable RunVariables;

				Parse = TRUE;
				RunVariables = ParserInstance->Variables;
				for (unsigned long ii = 0; ii < ParserInstance->VariableCount; ii++, RunVariables++)
				{
					if (strstr(ArgumentStart, RunVariables->VariableName))
					{
						unsigned long long* RunVVariables;
						PVariadicType VVariables;

						VVariables = RunVariables->Variable;

						OldPointer = VAArgs;
						VAArgs = ((PVariadicType)realloc(VAArgs, sizeof(VariadicType) - sizeof(unsigned long long) + (sizeof(unsigned long long) * ((VVariables->ArgumentCount - 1) + VAArgs->ArgumentCount))));
						UpdateGarbage(&ParserInstance->ArgumentGarbage, VAArgs, OldPointer);

						RunVAArgs = VAArgs->Arguments + i;
						VAArgs->ArgumentCount += (VVariables->ArgumentCount - 1);

						RunVVariables = VVariables->Arguments;
						for (ii = 0; ii < VVariables->ArgumentCount; ii++, i++, RunVVariables++, RunVAArgs++)
							CastVariables(ParserInstance, *RunVVariables, RunVariables->VariableType, RunArguments->ArgumentType, RunVAArgs);

						break;
					}
				}

				if (Parse)
				{
					for (; (*ArgumentStart) && ((*ArgumentStart < '0') || (*ArgumentStart > '9')); ArgumentStart++);
					if (!*ArgumentStart)
						break;

					CastVariables(ParserInstance, ((unsigned long long)ArgumentStart), ArgumentType_STRING, RunArguments->ArgumentType, RunVAArgs);
				}
			} break;
			case ArgumentType_STRING:
			{
				char* StringEnd;

				BreakCount = 1;
				(*RunVAArgs) = ((unsigned long long)ArgumentStart);
				while (BreakCount % 2)
				{
					(*RunVAArgs) = ((unsigned long long)strstr(((char*)*RunVAArgs), "\""));
					if (!*RunVAArgs)
						break;

					Calls->CountBlockers(Calls->Instance, ((char*)((*RunVAArgs) - 1)), &BreakCount);
					(*RunVAArgs)++;
				}

				if (!*RunVAArgs)
				{
					PVariable RunVariables;

					RunVariables = ParserInstance->Variables;
					for (unsigned long ii = 0; ii < ParserInstance->VariableCount; ii++, RunVariables++)
					{
						if (strstr(ArgumentStart, RunVariables->VariableName))
						{
							unsigned long long* RunVVariables;
							PVariadicType VVariables;

							VVariables = RunVariables->Variable;

							OldPointer = VAArgs;
							VAArgs = ((PVariadicType)realloc(VAArgs, sizeof(VariadicType) - sizeof(unsigned long long) + (sizeof(unsigned long long) * ((VVariables->ArgumentCount - 1) + VAArgs->ArgumentCount))));
							UpdateGarbage(&ParserInstance->ArgumentGarbage, VAArgs, OldPointer);

							RunVAArgs = VAArgs->Arguments + i;
							VAArgs->ArgumentCount += (VVariables->ArgumentCount - 1);

							RunVVariables = VVariables->Arguments;
							for (ii = 0; ii < VVariables->ArgumentCount; ii++, i++, RunVVariables++, RunVAArgs++)
								CastVariables(ParserInstance, *RunVVariables, RunVariables->VariableType, RunArguments->ArgumentType, RunVAArgs);

							break;
						}
					}
					break;
				}

				BreakCount = 1;
				StringEnd = ((char*)*RunVAArgs) - 1;
				while (BreakCount % 2)
				{
					StringEnd = strstr(StringEnd + 1, "\"");
					if (!StringEnd)
						break;

					Calls->CountBlockers(Calls->Instance, (StringEnd - 1), &BreakCount);
				}

				if (!StringEnd)
				{
					(*RunVAArgs) = 0;
					break;
				}

				if (TrimArguments)
					(*StringEnd) = '\0';

				ArgumentStart += (StringEnd - ((char*)*RunVAArgs) + 3);
			} break;
			}

			if (!LastReplace)
				break;

			if (!TrimArguments)
				if (LastReplace)
					(*LastReplace) = ',';

			ArgumentStart = LastReplace;
		}

		RunArguments->Argument = ((unsigned long long)VAArgs);
	}

	if (!TrimArguments)
		(*ArgumentEnd) = ']';
}

static GeneralErrorCast FixArgumentTriming(PCalls Calls, char* StringArguments, PArgument Arguments, unsigned long ArgumentCount)
{
	for (unsigned long i = 0; i < ArgumentCount; i++, Arguments++)
	{
		if (Arguments->ArgumentType != ArgumentType_STRING)
			continue;

		if (!Arguments->Argument)
			continue;

		(*((char*)(Arguments->Argument + strlen((char*)Arguments->Argument)))) = '\"';
	}

	for (; (ArgumentCount - 1); ArgumentCount--)
		(*(StringArguments + strlen(StringArguments))) = ',';

	(*(StringArguments + strlen(StringArguments))) = ']';
	return STATUS_SUCCESS;
}