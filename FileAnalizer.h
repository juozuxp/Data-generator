#pragma once
#include "Imports.h"
#include "BasicUtilities.h"

typedef GeneralErrorCast(*QueFunction)(struct _CallsObject* Instance, void* Reserved);

typedef struct _CallsObject
{
	BOOLEAN Restart;

	char* Buffer;
	unsigned long BufferCount;
	unsigned long MaxBuffer;

	char* CurrentLocation;
	char* EndLocation;
	struct _Condition* CurrentCondition;

	struct _Condition* Conditions;
	unsigned long ConditionCount;

	FunctionCallBack* QueFunctions;
	unsigned long QueCount;

	char ReplacedChar;
	char* ReplacedLocation;
} CallsObject, * PCallsObject;

typedef struct _Calls
{
	PCallsObject Instance;
	GeneralErrorCast(*Remove)(PCallsObject Instance);
	GeneralErrorCast(*Replace)(PCallsObject Instance, const char* Replacement, BOOLEAN RemoveLB);
	GeneralErrorCast(*ShiftBuffer)(PCallsObject Instance, char* ShiftStart, unsigned long ShiftLength);
	GeneralErrorCast(*CollapseBuffer)(PCallsObject Instance, char* CollapseLocation, unsigned long CollapseLength, char CharColapse);
	GeneralErrorCast(*Restart)(PCallsObject Instance);
	GeneralErrorCast(*ReplaceByString)(PCallsObject Instance, const char* ToFind, const char* Replacement);
	GeneralErrorCast(*QueFinishFunction)(PCallsObject Instance, QueFunction Function, void* Reserved);
	GeneralErrorCast(*CountBlockers)(PCallsObject Instance, char* StartLocation, unsigned long* Count);
} Calls, * PCalls;

typedef GeneralErrorCast(*ConditionCall)(char* Arguments, PCalls CallsObject, void* Reserved);

typedef struct _Condition
{
	const char* StartCondition;
	const char* EndCondition;
	ConditionCall Conditional;
	void* Reserved;
} Condition, * PCondition;

static GeneralErrorCast Replace(PCallsObject Instance, const char* Replacement, BOOLEAN RemoveLB)
{
	(*Instance->ReplacedLocation) = Instance->ReplacedChar;
	unsigned long ReplacementLegth = strlen(Replacement);

	char* ReplacementLocation = Instance->EndLocation + strlen(Instance->CurrentCondition->EndCondition);
	if (RemoveLB)
	{
		if (*((unsigned short*)ReplacementLocation) == *((unsigned short*)"\r\n"))
			ReplacementLocation += 2;
		else if (*ReplacementLocation == '\n')
			ReplacementLocation++;
	}

	if (((long long)((Instance->CurrentLocation + ReplacementLegth) - ReplacementLocation)) < 0)
		Instance->BufferCount -= ReplacementLocation - (Instance->CurrentLocation + ReplacementLegth);
	else
	{
		Instance->BufferCount += ((Instance->CurrentLocation + ReplacementLegth) - ReplacementLocation);

		if (Instance->MaxBuffer < Instance->BufferCount)
		{
			void* OldBuffer = Instance->Buffer;

			Instance->Buffer = ((char*)realloc(Instance->Buffer, Instance->BufferCount));
			Instance->MaxBuffer = Instance->BufferCount;

			Instance->EndLocation = ((char*)Instance->Buffer) + ((((char*)Instance->EndLocation) - ((char*)OldBuffer)));
			Instance->ReplacedLocation = ((char*)Instance->Buffer) + ((((char*)Instance->ReplacedLocation) - ((char*)OldBuffer)));
			Instance->CurrentLocation = ((char*)Instance->Buffer) + ((((char*)Instance->CurrentLocation) - ((char*)OldBuffer)));

			ReplacementLocation = ((char*)Instance->Buffer) + ((((char*)ReplacementLocation) - ((char*)OldBuffer)));
		}
	}

	memmove(Instance->CurrentLocation + ReplacementLegth, ReplacementLocation, strlen(ReplacementLocation) + 1);
	memcpy(Instance->CurrentLocation, Replacement, ReplacementLegth);

	Instance->EndLocation = Instance->CurrentLocation + ReplacementLegth;

	Instance->ReplacedChar = *Instance->EndLocation;
	Instance->ReplacedLocation = Instance->EndLocation;

	*(Instance->Buffer + Instance->BufferCount - 1) = '\0';

	return STATUS_SUCCESS;
}

static GeneralErrorCast ReplaceByString(PCallsObject Instance, const char* ToFind, const char* Replacement)
{
	unsigned long ToFindLength = strlen(Replacement);
	unsigned long ReplacementLength = strlen(Replacement);

	unsigned long LengthDifference = abs((((long long)ToFindLength) - ((long long)ReplacementLength)));
	if (ReplacementLength <= ToFindLength)
	{
		char* Found = strstr(Instance->CurrentLocation + strlen(Instance->CurrentCondition->StartCondition), ToFind);
		while (Found)
		{
			memmove(Found, Replacement, ReplacementLength);
			memmove(Found + ReplacementLength, (Found + ToFindLength), strlen(Found));

			Instance->BufferCount -= LengthDifference;
			Instance->EndLocation -= LengthDifference;
			Instance->ReplacedLocation -= LengthDifference;

			Found = strstr(Found, ToFind);
		}
	}
	else
	{
		char* Found = strstr(Instance->CurrentLocation + strlen(Instance->CurrentCondition->StartCondition), ToFind);
		while (Found)
		{
			Instance->BufferCount += LengthDifference;
			if (Instance->BufferCount > Instance->MaxBuffer)
			{
				void* OldBuffer = Instance->Buffer;

				Instance->Buffer = ((char*)realloc(Instance->Buffer, Instance->BufferCount));
				Instance->MaxBuffer = Instance->BufferCount;

				Instance->EndLocation = ((char*)Instance->Buffer) + ((((char*)Instance->EndLocation) - ((char*)OldBuffer)));
				Instance->ReplacedLocation = ((char*)Instance->Buffer) + ((((char*)Instance->ReplacedLocation) - ((char*)OldBuffer)));
				Instance->CurrentLocation = ((char*)Instance->Buffer) + ((((char*)Instance->CurrentLocation) - ((char*)OldBuffer)));
			}

			Instance->EndLocation += LengthDifference;
			Instance->ReplacedLocation += LengthDifference;

			memmove(Found + ReplacementLength, (Found + ToFindLength), strlen(Found));
			memmove(Found, Replacement, ReplacementLength);

			Found = strstr(Found, ToFind);
		}
	}

	*(Instance->Buffer + Instance->BufferCount - 1) = '\0';

	return STATUS_SUCCESS;
}

static GeneralErrorCast Remove(PCallsObject Instance)
{
	(*Instance->ReplacedLocation) = Instance->ReplacedChar;

	char* ReplaceLocation = Instance->EndLocation + strlen(Instance->CurrentCondition->EndCondition);

	if (*((unsigned short*)ReplaceLocation) == *((unsigned short*)"\r\n"))
		ReplaceLocation += 2;
	else if (*ReplaceLocation == '\n')
		ReplaceLocation++;

	memmove(Instance->CurrentLocation, ReplaceLocation, strlen(ReplaceLocation) + 1);
	Instance->BufferCount -= ReplaceLocation - Instance->CurrentLocation;

	Instance->EndLocation = Instance->CurrentLocation;

	Instance->ReplacedChar = *Instance->EndLocation;
	Instance->ReplacedLocation = Instance->EndLocation;

	*(Instance->Buffer + Instance->BufferCount - 1) = '\0';

	return STATUS_SUCCESS;
}

static GeneralErrorCast Restart(PCallsObject Instance)
{
	Instance->Restart = TRUE;

	return STATUS_SUCCESS;
}

static GeneralErrorCast QueFinishFunction(PCallsObject Instance, QueFunction Function, void* Reserved)
{
	Instance->QueCount++;
	if (!Instance->QueFunctions)
		Instance->QueFunctions = ((FunctionCallBack*)malloc(sizeof(FunctionCallBack)));
	else
		Instance->QueFunctions = ((FunctionCallBack*)realloc(Instance->QueFunctions, (sizeof(FunctionCallBack) * Instance->QueCount)));

	Instance->QueFunctions[Instance->QueCount - 1].Function = Function;
	Instance->QueFunctions[Instance->QueCount - 1].Reserved = Reserved;

	return STATUS_SUCCESS;
}

static GeneralErrorCast CollapseBuffer(PCallsObject Instance, char* CollapseLocation, unsigned long CollapseLength, char CharColapse)
{
	(*Instance->ReplacedLocation) = Instance->ReplacedChar;
	for (unsigned long i = 0; i < CollapseLength; i++, CollapseLocation++)
	{
		if ((*CollapseLocation) == CharColapse)
		{
			memmove(CollapseLocation, CollapseLocation + 1, strlen(CollapseLocation + 1) + 1);
			if (Instance->EndLocation >= CollapseLocation)
			{
				Instance->EndLocation--;
				Instance->ReplacedLocation--;
			}
			if (Instance->CurrentLocation >= CollapseLocation)
				Instance->CurrentLocation--;

			CollapseLocation--;

			Instance->BufferCount--;
		}
	}

	*(Instance->Buffer + Instance->BufferCount - 1) = '\0';

	Instance->ReplacedChar = *Instance->ReplacedLocation;
	(*Instance->ReplacedLocation) = '\0';

	return STATUS_SUCCESS;
}

static GeneralErrorCast ShiftBuffer(PCallsObject Instance, char* ShiftStart, unsigned long ShiftLength)
{
	char* ReplaceLocation = ShiftStart + ShiftLength;

	memmove(ShiftStart, ReplaceLocation, strlen(ReplaceLocation) + 1);

	if (Instance->CurrentLocation >= ReplaceLocation)
		Instance->CurrentLocation -= ShiftLength;

	if (Instance->EndLocation >= ReplaceLocation)
		Instance->EndLocation -= ShiftLength;

	if (Instance->ReplacedLocation >= ShiftStart && Instance->ReplacedLocation < (ShiftStart + ShiftLength))
	{
		Instance->ReplacedLocation = ShiftStart;
		Instance->ReplacedChar = *ShiftStart;
	}
	else if (Instance->ReplacedLocation >= ShiftStart)
		Instance->ReplacedLocation -= ShiftLength;

	Instance->BufferCount -= ShiftLength;

	return STATUS_SUCCESS;
}

static GeneralErrorCast CountBlockers(PCallsObject Instance, char* StartLocation, unsigned long* Count)
{
	for ((*Count) = 0; (*StartLocation) == '\\'; StartLocation--, (*Count)++);
	return STATUS_SUCCESS;
}

static GeneralErrorCast Analize(char** Buffer, PCondition Conditions, unsigned long ConditionCount)
{
	PCondition RunConiditions;
	unsigned long* RunSCLengthMap;
	unsigned long* RunECLengthMap;

	unsigned long* SCLengthMap;
	unsigned long* ECLengthMap;
	CallsObject CallsInstance;
	Calls Calls;

	char* RuntimeBuffer;

	SCLengthMap = ((unsigned long*)malloc(sizeof(unsigned long) * ConditionCount));
	ECLengthMap = ((unsigned long*)malloc(sizeof(unsigned long) * ConditionCount));

	RunConiditions = Conditions;
	RunSCLengthMap = SCLengthMap;
	RunECLengthMap = ECLengthMap;
	for (unsigned long i = 0; i < ConditionCount; i++, RunSCLengthMap++, RunECLengthMap++, RunConiditions++)
	{
		(*RunSCLengthMap) = strlen(RunConiditions->StartCondition);
		(*RunECLengthMap) = strlen(RunConiditions->EndCondition);
	}

	Calls.Remove = Remove;
	Calls.Replace = Replace;
	Calls.CollapseBuffer = CollapseBuffer;
	Calls.ReplaceByString = ReplaceByString;
	Calls.Restart = Restart;
	Calls.QueFinishFunction = QueFinishFunction;
	Calls.ShiftBuffer = ShiftBuffer;
	Calls.CountBlockers = &CountBlockers;
	Calls.Instance = &CallsInstance;

	CallsInstance.Conditions = Conditions;
	CallsInstance.ConditionCount = ConditionCount;

	CallsInstance.Restart = FALSE;
	CallsInstance.Buffer = *Buffer;
	CallsInstance.MaxBuffer = strlen(CallsInstance.Buffer) + 1;
	CallsInstance.BufferCount = strlen(CallsInstance.Buffer) + 1;

	CallsInstance.QueCount = 0;
	CallsInstance.QueFunctions = 0;

	RuntimeBuffer = CallsInstance.Buffer;
	while (TRUE)
	{
		char* StartLocation;
		char* EndLocation;
		unsigned long BreakCount;

		BreakCount = 1;
		for (; *RuntimeBuffer; RuntimeBuffer++)
		{
			RunConiditions = Conditions;
			RunSCLengthMap = SCLengthMap;
			RunECLengthMap = ECLengthMap;
			for (unsigned long i = 0; i < ConditionCount; i++, RunConiditions++, RunSCLengthMap++, RunECLengthMap++)
			{
				if (memcmp(RuntimeBuffer, RunConiditions->StartCondition, *RunSCLengthMap))
					continue;

				CountBlockers(&CallsInstance, RuntimeBuffer, &BreakCount);
				if (!(BreakCount % 2))
					break;

				RuntimeBuffer += *RunSCLengthMap;
				break;
			}

			if (!(BreakCount % 2))
				break;
		}
		if (!*RuntimeBuffer)
			break;

		StartLocation = RuntimeBuffer + *RunSCLengthMap;

		BreakCount = 1;
		EndLocation = StartLocation;
		while (BreakCount % 2)
		{
			EndLocation = strstr(EndLocation, RunConiditions->EndCondition);
			if (!EndLocation)
			{
				EndLocation += *RunECLengthMap;
				break;
			}

			CountBlockers(&CallsInstance, (EndLocation - 1), &BreakCount);
			EndLocation += *RunECLengthMap;
		}

		EndLocation -= *RunECLengthMap;
		if (!EndLocation)
			break;

		CallsInstance.EndLocation = EndLocation;
		CallsInstance.CurrentLocation = RuntimeBuffer;
		CallsInstance.CurrentCondition = RunConiditions;

		CallsInstance.ReplacedLocation = EndLocation;

		CallsInstance.ReplacedChar = *CallsInstance.ReplacedLocation;
		(*CallsInstance.ReplacedLocation) = '\0';
		RunConiditions->Conditional(StartLocation, &Calls, RunConiditions->Reserved);
		(*CallsInstance.ReplacedLocation) = CallsInstance.ReplacedChar;

		RuntimeBuffer = CallsInstance.EndLocation;
		if (CallsInstance.Restart)
		{
			RuntimeBuffer = CallsInstance.Buffer;
			CallsInstance.Restart = FALSE;
		}
	}

	if (CallsInstance.QueFunctions)
	{
		for (unsigned long i = 0; i < CallsInstance.QueCount; i++)
			((QueFunction)CallsInstance.QueFunctions[i].Function)(&CallsInstance, CallsInstance.QueFunctions[i].Reserved);

		free(CallsInstance.QueFunctions);
	}

	(*Buffer) = CallsInstance.Buffer;

	return STATUS_SUCCESS;
}