/**
 * func_memory.cpp - the module implementing the concept of
 * programer-visible memory space accesing via memory address.
 * @author Alexander Titov <alexander.igorevich.titov@gmail.com>
 * Copyright 2012 uArchSim iLab project
 */

/*******************************************************

    Reducted by:

    @date: October 13, 2014
    @author: Kirill Korolev <kirill.korolef@gmail.com>
    @vertion: 1.0 (October 13, 2014) 

*******************************************************/


// Generic C

// Generic C++
#include <iostream>
#include <string>
#include <sstream>
#include <bitset>

// uArchSim modules
#include <func_memory.h>
#include <elf_parser.h>

using namespace std;

uint64 SetBytes(uint64 num, uint64 length)
{
    assert(num + length <= 64);

    uint64 res = 0;
    for (int i = 0; i < num; i++)
        res |= 1 << (length + i);
    return res;
}

FuncMemory::FuncMemory(const char* executable_file_name,
                       uint64 addr_size,
                       uint64 page_num_size,
                       uint64 offset_size):
_addr_bits   (addr_size), 
_page_bits   (page_num_size), 
_offset_bits (offset_size),
_file_name   ((char*) executable_file_name)
{
    assert(executable_file_name != NULL);
    
    vector<ElfSection> sections;
    ElfSection::getAllElfSections(executable_file_name, sections);
    
    _set_templ = SetBytes(_addr_bits - _page_bits - _offset_bits, _page_bits + _offset_bits);
    _page_templ = SetBytes(_page_bits, _offset_bits);
    _offset_templ = SetBytes(_offset_bits);

    _sets_num = SetBytes(_addr_bits - _page_bits - _offset_bits);
    _pages_num = SetBytes(_page_bits);
    _offsets_num = SetBytes(_offset_bits);

    if (sections.size() == 0) {
        _begin_addr = 0;
        _end_addr = 0;
        _text_start = 0;
        _memory = NULL;
        return;
    }

    _begin_addr = sections[0].start_addr;
    _end_addr = sections[sections.size() - 1].start_addr + 
                sections[sections.size() - 1].size - 1;

    _memory = new uint8**[_sets_num];

    for (int i = 0; i < sections.size(); i++) {
        if (sections[i].name == ".text") 
            _text_start = sections[i].start_addr;
        for (int j = 0; j < sections[i].size; j++)
            write(sections[i].content[j], sections[i].start_addr + j, 1);
    }
}

FuncMemory::~FuncMemory()
{
    for (int i = 0; i < _sets_num; i++) {
        if (_memory[i])
            DestroySet(_memory[i]);
    }
    delete [] _memory;
}

void FuncMemory::DestroySet(uint8 **set_addr)
{
    assert(set_addr != 0);

    for (int i = 0; i < _pages_num; i++) {
        if (!set_addr[i])
            delete [] set_addr[i];
    }
    delete [] set_addr;
}

uint8& FuncMemory::Search(const uint64 addr, const char mode) const
{
    assert(addr != 0);
    assert(mode == 'r' || mode == 'w');

    uint64 set_addr = (addr & _set_templ) >> (_page_bits + _offset_bits);
    uint64 page_addr = (addr & _page_templ) >> _offset_bits;
    uint64 offset_addr = addr & _offset_templ;

    if (mode == 'r') {
        assert(_memory[set_addr]);
        assert(_memory[set_addr][page_addr]);
    } else {
        if (!_memory[set_addr])
            _memory[set_addr] = new uint8*[_pages_num];
        if (!_memory[set_addr][page_addr])
            _memory[set_addr][page_addr] = new uint8[_offsets_num];
    }

    uint8 &res = _memory[set_addr][page_addr][offset_addr];
    return res;
}
            
uint64 FuncMemory::read(uint64 addr, unsigned short num_of_bytes) const
{
    assert(num_of_bytes <= 8);
    assert(addr != 0);

    uint64 res = 0;
    for (int i = 0; i < num_of_bytes; i++) {
        res <<= 8;
        res += Search(addr + i);
    }
    
    return res; 
}

void FuncMemory::write(uint64 value, uint64 addr, unsigned short num_of_bytes)
{
    assert(num_of_bytes <= 8);
    assert(addr != 0);
    
    if (value != 0) {
        uint64 templ = SetBytes(8);
        for (int i = num_of_bytes; i > 0; i--) {
            Search(addr + i - 1, 'w') = value & templ;
            templ <<= 8;
        }
    }
}

string FuncMemory::dump(string indent) const
{
    ostringstream oss;

    oss << indent << "Functional Memory Dump" << endl
        << indent << "Executable file name: " << _file_name << endl
        << indent << "First address: 0x" << hex << _begin_addr << dec << endl
        << indent << "Last address: 0x" << hex << _end_addr << dec << endl
        << indent << "Content:" << endl
        << indent << "|SET       |PAGE      |OFFSET:  VALUE     |" << endl;

    indent.push_back(' ');
    for (int i = 0; i < _sets_num; i++) {
        if (_memory[i]) {
            oss << indent << "0x" << hex << i << dec << endl;
            string set_indent = indent;
            set_indent.push_back(' ');
            oss << SetDump(_memory[i], set_indent);
        }
    }
    
    return oss.str();               
}

string FuncMemory::SetDump(uint8 **set_addr, string indent) const
{
    ostringstream oss;

    uint64 i = 0;
    while (!set_addr[i] && i < _pages_num) i++;

    char sym = '\0';
    while (i != _pages_num) {
        uint64 curr_page = i;
        i++;
        while (!set_addr[i] && i < _pages_num) i++;

        if (i == _pages_num) 
            sym = ' ';
        else 
            sym = '|';
    
        oss << indent << sym << "`--------0x" << hex << curr_page << dec << endl;
        string page_indent = indent;
        page_indent.push_back(sym);
        page_indent.append("          ");
        oss << PageDump(set_addr[curr_page], page_indent);
    }

    oss << endl;
    return oss.str();
}       
            
string FuncMemory::PageDump(uint8 *page_addr, string indent) const
{
    ostringstream oss;

    uint64 i = 0;
    while (!page_addr[i] && i < _offsets_num) i++;

    char sym = '\0';
    while (i != _offsets_num) {
        uint64 curr_offset = i;
        i++;
        while (!page_addr[i] && i < _offsets_num) i++;

        if (i == _offsets_num)
            sym = ' ';
        else
            sym = '|';

        oss << indent << sym << "`--------0x" << hex << curr_offset << ":  "
            << bitset<8>(page_addr[curr_offset]) << endl;
    }

    oss << indent << endl;
    return oss.str();
}
