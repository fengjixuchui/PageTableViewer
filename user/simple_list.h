#ifndef __SIMPLE_LIST_H__
#define __SIMPLE_LIST_H__

typedef struct _SIMPLE_LIST{
	struct _SIMPLE_LIST* next;
	struct _SIMPLE_LIST* prev;
	void* key;
} SIMPLE_LIST;

SIMPLE_LIST* GetListItem(void* key, SIMPLE_LIST** ppFirst, unsigned nSize);
void DeleteListItem(void* key, SIMPLE_LIST* pFirst, unsigned nSize);

SIMPLE_LIST* SearchListItem(void* key, SIMPLE_LIST** ppFirst);

#endif /* __SIMPLE_LIST_H__ */
