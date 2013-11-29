/**
 * set.cpp - the module implementing the concept of
 * programer-visible memory space accesing via memory address.
 * @author Anton Mitrokhin <anton.v.mitrokhin@gmail.com>
 * Copyright 2013 uArchSim iLab project
 */

// Generic C
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cerrno>
#include <cassert>

// Generic C++
#include <iostream>
#include <string>
#include <sstream>

// uArchSim modules
#include <func_memory.h>


Set::Set( uint64 start_addr,
          uint64 size,
          uint64 page_size)
{
    this->start_addr = start_addr;
    this->size = size;
    this->page_size = page_size;

    this->content = new Page* [ this->size];

    for( size_t iterator = 0; iterator < ( this->size); iterator++)
        ( this->content)[ iterator] = NULL;
}

Set::~Set()
{
    for( size_t iterator = 0; iterator < ( this->size); iterator++)
        delete [] content[ iterator];
    delete [] this->content;
}

uint64 Set::read( uint64 page_num, uint64 page_offset, unsigned short num_of_bytes) const
{
    if( page_num >= size)
    {
        cerr << "ERROR: unable to read "
             << " - the page number is too big" << endl;
        exit( EXIT_FAILURE);
    }

    if( content[ page_num] == NULL)
    {
        cerr << "ERROR: unable to read "
             << " - the page does not exist" << endl;
        exit( EXIT_FAILURE);
    }

    Page* page = content[ page_num];
    return page->read( page_offset, num_of_bytes);
}

void   Set::write( uint64 value, uint64 page_num, uint64 page_offset, unsigned short num_of_bytes)
{
    if( page_num >= size)
    {
        cerr << "ERROR: unable to write "
             << " - the page number is too big" << endl;
        exit( EXIT_FAILURE);
    }

    if( content[ page_num] == NULL)
    {
        content[ page_num] = new Page( page_num, this->page_size);
    }

    content[ page_num]->write( value, page_offset, num_of_bytes);
}

string Set::dump( string indent) const
{
    ostringstream oss;

    oss << indent << "Set start_addr = 0x" << hex << this->start_addr << dec << endl
        << indent << "  size = " << this->size << " Pages" << endl
        << indent << "  Content:" << endl;

    for( size_t iterator = 0; iterator < ( this->size); iterator++)
    {
        if( this->content[ iterator] != NULL)
        {
            oss << (this->content)[ iterator]->dump( "  ");
        }
    }

    return oss.str();
}