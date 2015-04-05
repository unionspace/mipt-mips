/*
 * rf.h - mips register file
 * @author Pavel Kryukov pavel.kryukov@phystech.edu
 * Copyright 2015 MIPT-MIPS
 */

#ifndef RF_H
#define RF_H

#include <func_instr.h>

class RF {
        struct Reg {
            uint32 value;
            bool   is_valid;
            Reg() : value(0ull), is_valid(true) { }
        } array[REG_MAX_NUM];
    public:
        //uint32 read( Reg_Num);
        /**************/
        bool check( Reg_Num num) const { return array[(size_t)num].is_valid; }
        void invalidate( Reg_Num num) { array[(size_t)num].is_valid = false; }

        /**************/
        void write ( Reg_Num num, uint32 val) {
             assert( array[(size_t)num].is_valid == false);
             array[(size_t)num].is_valid = true;
             if ( REG_NUM_ZERO != reg_num)
                array[(size_t)num].value = val;
        }

        /**************/
        inline bool read_src1( FuncInstr& instr) const
        {
           size_t reg_num = instr.get_src1_num();
           instr.set_v_src1( array[reg_num].value);
           return array[reg_num].is_valid;
        }

        /**************/
        inline bool read_src2( FuncInstr& instr) const
        {
           size_t reg_num = instr.get_src2_num();
           instr.set_v_src2( array[reg_num].value);
           return array[reg_num].is_valid;
        }

        /**************/
        inline void write_dst( const FuncInstr& instr)
        {
            size_t reg_num = instr.get_dst_num();
            write(reg_num, instr.get_v_dst);
        }

        /**************/
        inline void reset( RegNum reg)
        {
            array[reg].value = 0;
        }

        /**************/
        RF()
        {
            for ( size_t i = 0; i < REG_NUM_MAX; ++i)
                reset((RegNum)i);
        }
    };

#endif
