/* picoc heap memory allocation. This is a complete (but small) memory
 * allocator for embedded systems which have no memory allocator. Alternatively
 * you can define USE_MALLOC_HEAP to use your system's own malloc() allocator */

/* stack grows up from the bottom and heap grows down from the top of heap space */
#include "interpreter.h"

/* initialise the stack and heap storage */
void HeapInit(Picoc *pc, int StackOrHeapSize)
{
	int AlignOffset = 0;

	pc->HeapMemory = (unsigned char*)malloc(StackOrHeapSize);
	pc->HeapBottom = nullptr;						/* the bottom of the (downward-growing) heap */
	pc->StackFrame = nullptr;						/* the current stack frame */
	pc->HeapStackTop = nullptr;						/* the top of the stack */

	while (((unsigned long)&pc->HeapMemory[AlignOffset] & (sizeof(ALIGN_TYPE)-1)) != 0)
		AlignOffset++;

	pc->StackFrame = &(pc->HeapMemory)[AlignOffset];
	pc->HeapStackTop = &(pc->HeapMemory)[AlignOffset];
	*(void **)(pc->StackFrame) = nullptr;
	pc->HeapBottom = &(pc->HeapMemory)[StackOrHeapSize-sizeof(ALIGN_TYPE)+AlignOffset];
}

void HeapCleanup(Picoc *pc)
{
	free(pc->HeapMemory);
}

/* allocate some space on the stack, in the current stack frame
 * clears memory. can return nullptr if out of stack space */
void *HeapAllocStack(Picoc *pc, int Size)
{
	char *NewMem = (char*)pc->HeapStackTop;
	char *NewTop = (char *)pc->HeapStackTop + MEM_ALIGN(Size);
#ifdef DEBUG_HEAP
	printf("HeapAllocStack(%ld) at 0x%lx\n", (unsigned long)MEM_ALIGN(Size), (unsigned long)pc->HeapStackTop);
#endif
	if (NewTop > (char *)pc->HeapBottom)
		return nullptr;

	pc->HeapStackTop = (void *)NewTop;
	memset((void *)NewMem, '\0', Size);
	return NewMem;
}

/* allocate some space on the stack, in the current stack frame */
void HeapUnpopStack(Picoc *pc, int Size)
{
#ifdef DEBUG_HEAP
	printf("HeapUnpopStack(%ld) at 0x%lx\n", (unsigned long)MEM_ALIGN(Size), (unsigned long)pc->HeapStackTop);
#endif
	pc->HeapStackTop = (void *)((char *)pc->HeapStackTop + MEM_ALIGN(Size));
}

/* free some space at the top of the stack */
bool HeapPopStack(Picoc *pc, int Size)
{
	int ToLose = MEM_ALIGN(Size);
	if (ToLose > ((char *)pc->HeapStackTop - (char *)&(pc->HeapMemory)[0]))
		return false;

	pc->HeapStackTop = (void *)((char *)pc->HeapStackTop - ToLose);

	return true;
}

/* push a new stack frame on to the stack */
void HeapPushStackFrame(Picoc *pc)
{
#ifdef DEBUG_HEAP
	printf("Adding stack frame at 0x%lx\n", (unsigned long)pc->HeapStackTop);
#endif
	*(void **)pc->HeapStackTop = pc->StackFrame;
	pc->StackFrame = pc->HeapStackTop;
	pc->HeapStackTop = (void *)((char *)pc->HeapStackTop + MEM_ALIGN(sizeof(ALIGN_TYPE)));
}

/* pop the current stack frame, freeing all memory in the frame. can return nullptr */
int HeapPopStackFrame(Picoc *pc)
{
	if (*(void **)pc->StackFrame != nullptr)
	{
		pc->HeapStackTop = pc->StackFrame;
		pc->StackFrame = *(void **)pc->StackFrame;
#ifdef DEBUG_HEAP
		printf("Popping stack frame back to 0x%lx\n", (unsigned long)pc->HeapStackTop);
#endif
		return true;
	}
	else
		return false;
}

/* allocate some dynamically allocated memory. memory is cleared. can return nullptr if out of memory */
void *HeapAllocMem(Picoc *pc, int Size)
{
	return calloc(Size, 1);
}

/* free some dynamically allocated memory */
void HeapFreeMem(Picoc *pc, void *Mem)
{
	free(Mem);
}
