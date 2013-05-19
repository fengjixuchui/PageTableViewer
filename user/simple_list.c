#include <windows.h>
#include "simple_list.h"


SIMPLE_LIST* SearchListItem(void* key, SIMPLE_LIST** ppFirst)
{
	SIMPLE_LIST* p;

	for (p = *ppFirst; p; p = p->next){
		if (p->key == key)
			return p;
	}
	return NULL;
}

SIMPLE_LIST* GetListItem(void* key, SIMPLE_LIST** ppFirst, unsigned nSize)
{
	SIMPLE_LIST* p;

	p = SearchListItem(key, ppFirst);
	if (p)
		return p;
	
	// if it was not found then alloc
	p = *ppFirst;
	*ppFirst = VirtualAlloc(NULL, nSize ? nSize : sizeof(SIMPLE_LIST), MEM_COMMIT, PAGE_READWRITE);
	(*ppFirst)->key = key;
	if (p){
		p->prev = *ppFirst;
		(*ppFirst)->next = p;
	}
	return *ppFirst;
}

void DeleteListItem(void* key, SIMPLE_LIST* pFirst, unsigned nSize)
{
	SIMPLE_LIST* p;

	for (p = pFirst; p; p = p->next){
		if (p->key == key){
			if (p->prev)
				(p->prev)->next = p->next;
			if (p->next)
				(p->next)->prev = p->prev;
			VirtualFree(p, nSize ? nSize : sizeof(SIMPLE_LIST), MEM_DECOMMIT);
			return;
		}
	}
}
