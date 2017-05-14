#pragma once
/*
  AFrame - Arduino framework library for ASensor and AWind libraries
  Copyright (C)2015 Andrei Degtiarev. All right reserved
  
  You can always find the latest version of the library at 
  https://github.com/AndreiDegtiarev/AFrame

  This library is free software; you can redistribute it and/or
  modify it under the terms of the MIT license.
  Please see the included documents for further information.
*/
#include "Log.h"
///Internal class, implements list entry container.
template <class T> class LinkedEntry
{
template<class U> friend class LinkedList;
	LinkedEntry* Next; //!<Pointer to the next list item
	T* Item;           //!<Pointer to the list element
	LinkedEntry(T* item)
	{
		Item = item;
		Next=NULL;
	}
};
///Implements list container. Examples can be found in ASensor and AWind libraries
template <class T> class LinkedList
{
	LinkedEntry<T> * Head; //!<Pointer to the first item
	int _count;  //!<Number of list items
public:
	LinkedList()
	{
		Head=NULL;
		_count=0;
	};
	///Copy constructor
	/*LinkedList(LinkedList &srcList)
	{
		Head=NULL;
		_count=0;
		for(int i=0;i<srcList.Count();i++)
			Add(srcList[i]);
	};*/
	///Adds new element to the list
	void Add(T *item)
	{
		LinkedEntry<T> *new_entry=new LinkedEntry<T>(item);
		if(Head!=NULL)
		{
			LinkedEntry<T> *cur= Head;
			while(cur->Next!=NULL)
			{
				cur=cur->Next;
			}
			cur->Next = new_entry;
		}
		else
		{
			Head = new_entry;
		}
		_count++;
	}
	///Returns numbr of list items
	int Count()
	{
		return _count;
	}
	///Implements [] operator
	T* operator[](int pos)
	{
		if(pos>=_count)
		{
			out<<F("Error: index is too big: ")<<pos<<endln;
			return NULL;
		}
		else
		{
			int index=0;
			LinkedEntry<T> *cur= Head;
			while(cur!=NULL)
			{
				if(index == pos)
					return cur->Item;
				cur=cur->Next;
				index++;
			}
		}
		return NULL;
	}
};