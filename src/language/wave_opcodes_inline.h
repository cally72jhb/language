#ifndef OPCODE_ENTRY
#define OPCODE_ENTRY(...)
#endif

// Opcodes (maximum of 256 opcodes can be defined):

////////////////////////////////////////////////////////////////
// Special Opcodes                                            //
////////////////////////////////////////////////////////////////

OPCODE_ENTRY(END)                       /* stops the execution of the bytecode */
OPCODE_ENTRY(NOP)                       /* no-operation (nop) - does nothing - is usually skipped by the vm, though the compiler should remove every nop instruction, as it's used in the compilation process for aligning bytecode blocks */

OPCODE_ENTRY(DEBUG)                     /* used in debugging operations */

////////////////////////////////////////////////////////////////
// Instruction Pointer                                        //
////////////////////////////////////////////////////////////////

OPCODE_ENTRY(JUMP)                      /* jumps by the value at the top of the stack (i32; @stack_top)  */
OPCODE_ENTRY(CJUMP)                     /* [ opcode | 32bit branch_offset (i32) ] - jumps by @branch_offset */

OPCODE_ENTRY(CJUMP_8_IF_0)              /* [ opcode | 16bit branch_offset ] - jumps by @branch_offset if the 8bit boolean value at the top of the stack is false  */
OPCODE_ENTRY(CJUMP_8_IF_1)              /* [ opcode | 16bit branch_offset ] - jumps by @branch_offset if the 8bit boolean value at the top of the stack is true   */
OPCODE_ENTRY(CJUMP_16_IF_0)             /* [ opcode | 16bit branch_offset ] - jumps by @branch_offset if the 16bit boolean value at the top of the stack is false */
OPCODE_ENTRY(CJUMP_16_IF_1)             /* [ opcode | 16bit branch_offset ] - jumps by @branch_offset if the 16bit boolean value at the top of the stack is true  */
OPCODE_ENTRY(CJUMP_32_IF_0)             /* [ opcode | 16bit branch_offset ] - jumps by @branch_offset if the 32bit boolean value at the top of the stack is false */
OPCODE_ENTRY(CJUMP_32_IF_1)             /* [ opcode | 16bit branch_offset ] - jumps by @branch_offset if the 32bit boolean value at the top of the stack is true  */
OPCODE_ENTRY(CJUMP_64_IF_0)             /* [ opcode | 16bit branch_offset ] - jumps by @branch_offset if the 64bit boolean value at the top of the stack is false */
OPCODE_ENTRY(CJUMP_64_IF_1)             /* [ opcode | 16bit branch_offset ] - jumps by @branch_offset if the 64bit boolean value at the top of the stack is true  */

OPCODE_ENTRY(TABLESWITCH)               /* [ opcode | 16bit field : (2bit value_size, 14bit length) | array jump_table : (16bit branch_offset...) ] - jumps by the @branch_offset in the @table at the index of the value at @stack_top */
OPCODE_ENTRY(LOOKUPSWITCH)              /* [ opcode | 16bit field : (2bit value_size, 14bit length) | array jump_table : (value, 16bit branch_offset...) ] - searches for the value at @stack_top in @table and jumps by the corresponding @branch_offset */

////////////////////////////////////////////////////////////////
// Functions                                                  //
////////////////////////////////////////////////////////////////

OPCODE_ENTRY(CALL_NATIVE)               /* [ opcode | 16bit function_index ] - calls the native function at index @function_index in the function stack of the vm */
OPCODE_ENTRY(CALL_NATIVE_ERR)           /* [ opcode | 16bit function_index ] - calls the native function at index @function_index in the function stack of the vm and throws all occurring error codes */

OPCODE_ENTRY(CALL)                      /* [ opcode | 32bit branch_offset (i32) ] - pushes the current instruction pointer to the callstack and jumps by @branch_offset */

OPCODE_ENTRY(CALL_DYN)                  /* calls a function (native functions, or normal functions) that is stored in the stack */
OPCODE_ENTRY(CALL_DYN_ERR)              /* calls a function (native functions, or normal functions) that is stored in the stack and throws all occurring error codes */

OPCODE_ENTRY(RETURN)                    /* returns from a function, by jumping to the instruction pointer at the top of the callstack, which is then popped from the callstack */

//OPCODE_ENTRY(RETURN_0)                /* returns a nothing from a function, by jumping to the instruction pointer stored at the top of the stack, which is then popped from the stack */
//OPCODE_ENTRY(RETURN_8)                /* returns a 8bit  value from a function, by jumping to the instruction pointer stored in front of the 8bit  value in the stack, which is then popped from the stack */
//OPCODE_ENTRY(RETURN_16)               /* returns a 16bit value from a function, by jumping to the instruction pointer stored in front of the 16bit value in the stack, which is then popped from the stack */
//OPCODE_ENTRY(RETURN_32)               /* returns a 32bit value from a function, by jumping to the instruction pointer stored in front of the 32bit value in the stack, which is then popped from the stack */
//OPCODE_ENTRY(RETURN_64)               /* returns a 64bit value from a function, by jumping to the instruction pointer stored in front of the 64bit value in the stack, which is then popped from the stack */

////////////////////////////////////////////////////////////////
// Error Handling                                             //
////////////////////////////////////////////////////////////////

OPCODE_ENTRY(ERR_THROW)                 /* reads the top 16bit value from the stack, pushes it to the error stack and handles the error accordingly */

OPCODE_ENTRY(ERR_TRY_START)             /* [ opcode | 32bit branch_offset ] - starts the error catching until @OPCODE_ERR_CATCH is hit, when an error occurs the instruction pointer is moved to @branch_offset */
OPCODE_ENTRY(ERR_CATCH)                 /* pops the top value (u16) off the error stack */
OPCODE_ENTRY(ERR_READ)                  /* pushes the top value (u16) from the error stack to the stack */
OPCODE_ENTRY(ERR_CHECK)                 /* reads the top value from the error stack, if there is any and then throws the corresponding exception */

////////////////////////////////////////////////////////////////
// Stack                                                      //
////////////////////////////////////////////////////////////////

OPCODE_ENTRY(PUSH_8)                    /* [ opcode | 8bit  value ] - pushes @value (8bit)  to the stack */
OPCODE_ENTRY(PUSH_16)                   /* [ opcode | 16bit value ] - pushes @value (16bit) to the stack */
OPCODE_ENTRY(PUSH_32)                   /* [ opcode | 32bit value ] - pushes @value (32bit) to the stack */
OPCODE_ENTRY(PUSH_64)                   /* [ opcode | 64bit value ] - pushes @value (64bit) to the stack */

OPCODE_ENTRY(POP_8)                     /* pops 1 byte (8bit)  off the stack */
OPCODE_ENTRY(POP_16)                    /* pops 2 byte (16bit) off the stack */
OPCODE_ENTRY(POP_32)                    /* pops 4 byte (32bit) off the stack */
OPCODE_ENTRY(POP_64)                    /* pops 8 byte (64bit) off the stack */
OPCODE_ENTRY(POP_128)                   /* pops 16 byte (128bit) off the stack */

OPCODE_ENTRY(POP_N)                     /* [ opcode | 16bit amount_bytes ] - pops as many bytes off the stack as @amount_bytes, not including @amount_bytes */
OPCODE_ENTRY(POP_FREE)                  /* deallocates the address at the top of the stack (addr; @stack_top) and pops it off the stack */

OPCODE_ENTRY(SWAP_8)                    /* swaps the 8bit  value at the top of the stack with the 8bit  value before that */
OPCODE_ENTRY(SWAP_16)                   /* swaps the 16bit value at the top of the stack with the 16bit value before that */
OPCODE_ENTRY(SWAP_32)                   /* swaps the 32bit value at the top of the stack with the 32bit value before that */
OPCODE_ENTRY(SWAP_64)                   /* swaps the 64bit value at the top of the stack with the 64bit value before that */

//OPCODE_ENTRY(DUP_8)                   /* duplicates the 8bit  value at the top of the stack, by copying and pushing it */
//OPCODE_ENTRY(DUP_16)                  /* duplicates the 16bit value at the top of the stack, by copying and pushing it */
//OPCODE_ENTRY(DUP_32)                  /* duplicates the 32bit value at the top of the stack, by copying and pushing it */
//OPCODE_ENTRY(DUP_64)                  /* duplicates the 64bit value at the top of the stack, by copying and pushing it */

////////////////////////////////////////////////////////////////
// Local Stack                                                //
////////////////////////////////////////////////////////////////

OPCODE_ENTRY(LOAD_8)                    /* [ opcode | 16bit offset ] - pushes a 8bit  value from the local stack (function stack frame) at the offset @offset to the top of the stack */
OPCODE_ENTRY(LOAD_16)                   /* [ opcode | 16bit offset ] - pushes a 16bit value from the local stack (function stack frame) at the offset @offset to the top of the stack */
OPCODE_ENTRY(LOAD_32)                   /* [ opcode | 16bit offset ] - pushes a 32bit value from the local stack (function stack frame) at the offset @offset to the top of the stack */
OPCODE_ENTRY(LOAD_64)                   /* [ opcode | 16bit offset ] - pushes a 64bit value from the local stack (function stack frame) at the offset @offset to the top of the stack */
OPCODE_ENTRY(STORE_8)                   /* [ opcode | 16bit offset ] - pops a 8bit  value off the stack and sets the 8bit  variable at @offset in the local stack (function stack frame) to that value */
OPCODE_ENTRY(STORE_16)                  /* [ opcode | 16bit offset ] - pops a 16bit value off the stack and sets the 16bit variable at @offset in the local stack (function stack frame) to that value */
OPCODE_ENTRY(STORE_32)                  /* [ opcode | 16bit offset ] - pops a 32bit value off the stack and sets the 32bit variable at @offset in the local stack (function stack frame) to that value */
OPCODE_ENTRY(STORE_64)                  /* [ opcode | 16bit offset ] - pops a 64bit value off the stack and sets the 64bit variable at @offset in the local stack (function stack frame) to that value */

////////////////////////////////////////////////////////////////
// Global Stack                                               //
////////////////////////////////////////////////////////////////

OPCODE_ENTRY(GET_GLOB_8)                /* [ opcode | 32bit offset ] - pushes the 8bit  value at @offset bytes in the globals array to the stack */
OPCODE_ENTRY(GET_GLOB_16)               /* [ opcode | 32bit offset ] - pushes the 16bit value at @offset bytes in the globals array to the stack */
OPCODE_ENTRY(GET_GLOB_32)               /* [ opcode | 32bit offset ] - pushes the 32bit value at @offset bytes in the globals array to the stack */
OPCODE_ENTRY(GET_GLOB_64)               /* [ opcode | 32bit offset ] - pushes the 64bit value at @offset bytes in the globals array to the stack */
OPCODE_ENTRY(SET_GLOB_8)                /* [ opcode | 32bit offset ] - sets the 8bit  value at @offset bytes in the globals array to the 8bit  value at the top of the stack */
OPCODE_ENTRY(SET_GLOB_16)               /* [ opcode | 32bit offset ] - sets the 16bit value at @offset bytes in the globals array to the 16bit value at the top of the stack */
OPCODE_ENTRY(SET_GLOB_32)               /* [ opcode | 32bit offset ] - sets the 32bit value at @offset bytes in the globals array to the 32bit value at the top of the stack */
OPCODE_ENTRY(SET_GLOB_64)               /* [ opcode | 32bit offset ] - sets the 64bit value at @offset bytes in the globals array to the 64bit value at the top of the stack */

////////////////////////////////////////////////////////////////
// Bit / Bitwise Operations                                   //
////////////////////////////////////////////////////////////////

// Bit Shift

OPCODE_ENTRY(SHIFT_L_8)                 /* bitwise shift left */
OPCODE_ENTRY(SHIFT_L_16)                /* bitwise shift left */
OPCODE_ENTRY(SHIFT_L_32)                /* bitwise shift left */
OPCODE_ENTRY(SHIFT_L_64)                /* bitwise shift left */

OPCODE_ENTRY(SHIFT_R_8)                 /* bitwise shift right */
OPCODE_ENTRY(SHIFT_R_16)                /* bitwise shift right */
OPCODE_ENTRY(SHIFT_R_32)                /* bitwise shift right */
OPCODE_ENTRY(SHIFT_R_64)                /* bitwise shift right */

// Other Bitwise Operations

OPCODE_ENTRY(BAND_8)    OPCODE_ENTRY(BAND_16)    OPCODE_ENTRY(BAND_32)    OPCODE_ENTRY(BAND_64)    /* bitwise and */
OPCODE_ENTRY(BOR_8)     OPCODE_ENTRY(BOR_16)     OPCODE_ENTRY(BOR_32)     OPCODE_ENTRY(BOR_64)     /* bitwise or */
OPCODE_ENTRY(XOR_8)     OPCODE_ENTRY(XOR_16)     OPCODE_ENTRY(XOR_32)     OPCODE_ENTRY(XOR_64)     /* bitwise xor */
OPCODE_ENTRY(BNOT_8)    OPCODE_ENTRY(BNOT_16)    OPCODE_ENTRY(BNOT_32)    OPCODE_ENTRY(BNOT_64)    /* bitwise not */

// Comparing

OPCODE_ENTRY(NOT_8)                     /* logical not */
OPCODE_ENTRY(NOT_16)                    /* logical not */
OPCODE_ENTRY(NOT_32)                    /* logical not */
OPCODE_ENTRY(NOT_64)                    /* logical not */

OPCODE_ENTRY(EQU_8)     OPCODE_ENTRY(EQU_16)     OPCODE_ENTRY(EQU_32)     OPCODE_ENTRY(EQU_64)     /* equals */
OPCODE_ENTRY(NEQ_8)     OPCODE_ENTRY(NEQ_16)     OPCODE_ENTRY(NEQ_32)     OPCODE_ENTRY(NEQ_64)     /* not equals */

OPCODE_ENTRY(AND_8)     OPCODE_ENTRY(AND_16)     OPCODE_ENTRY(AND_32)     OPCODE_ENTRY(AND_64)     /* logical and */
OPCODE_ENTRY(OR_8)      OPCODE_ENTRY(OR_16)      OPCODE_ENTRY(OR_32)      OPCODE_ENTRY(OR_64)      /* logical or */

////////////////////////////////////////////////////////////////
// Unsigned Integer Math Functions                            //
////////////////////////////////////////////////////////////////

OPCODE_ENTRY(U8_ADD)    OPCODE_ENTRY(U16_ADD)    OPCODE_ENTRY(U32_ADD)    OPCODE_ENTRY(U64_ADD)    /* add */
OPCODE_ENTRY(U8_SUB)    OPCODE_ENTRY(U16_SUB)    OPCODE_ENTRY(U32_SUB)    OPCODE_ENTRY(U64_SUB)    /* subtract */
OPCODE_ENTRY(U8_MUL)    OPCODE_ENTRY(U16_MUL)    OPCODE_ENTRY(U32_MUL)    OPCODE_ENTRY(U64_MUL)    /* multiply */
OPCODE_ENTRY(U8_DIV)    OPCODE_ENTRY(U16_DIV)    OPCODE_ENTRY(U32_DIV)    OPCODE_ENTRY(U64_DIV)    /* divide */
OPCODE_ENTRY(U8_MOD)    OPCODE_ENTRY(U16_MOD)    OPCODE_ENTRY(U32_MOD)    OPCODE_ENTRY(U64_MOD)    /* modulo */
OPCODE_ENTRY(U8_POW)    OPCODE_ENTRY(U16_POW)    OPCODE_ENTRY(U32_POW)    OPCODE_ENTRY(U64_POW)    /* power */

OPCODE_ENTRY(U8_INC)    OPCODE_ENTRY(U16_INC)    OPCODE_ENTRY(U32_INC)    OPCODE_ENTRY(U64_INC)    /* increment */
OPCODE_ENTRY(U8_DEC)    OPCODE_ENTRY(U16_DEC)    OPCODE_ENTRY(U32_DEC)    OPCODE_ENTRY(U64_DEC)    /* decrement */

OPCODE_ENTRY(U8_LT)     OPCODE_ENTRY(U16_LT)     OPCODE_ENTRY(U32_LT)     OPCODE_ENTRY(U64_LT)     /* less than */
OPCODE_ENTRY(U8_LE)     OPCODE_ENTRY(U16_LE)     OPCODE_ENTRY(U32_LE)     OPCODE_ENTRY(U64_LE)     /* less than or equal */
OPCODE_ENTRY(U8_GT)     OPCODE_ENTRY(U16_GT)     OPCODE_ENTRY(U32_GT)     OPCODE_ENTRY(U64_GT)     /* greater than */
OPCODE_ENTRY(U8_GE)     OPCODE_ENTRY(U16_GE)     OPCODE_ENTRY(U32_GE)     OPCODE_ENTRY(U64_GE)     /* greater than or equal */

////////////////////////////////////////////////////////////////
// Signed Integer Math Functions                              //
////////////////////////////////////////////////////////////////

OPCODE_ENTRY(I8_ADD)    OPCODE_ENTRY(I16_ADD)    OPCODE_ENTRY(I32_ADD)    OPCODE_ENTRY(I64_ADD)    /* add */
OPCODE_ENTRY(I8_SUB)    OPCODE_ENTRY(I16_SUB)    OPCODE_ENTRY(I32_SUB)    OPCODE_ENTRY(I64_SUB)    /* subtract */
OPCODE_ENTRY(I8_MUL)    OPCODE_ENTRY(I16_MUL)    OPCODE_ENTRY(I32_MUL)    OPCODE_ENTRY(I64_MUL)    /* multiply */
OPCODE_ENTRY(I8_DIV)    OPCODE_ENTRY(I16_DIV)    OPCODE_ENTRY(I32_DIV)    OPCODE_ENTRY(I64_DIV)    /* divide */
OPCODE_ENTRY(I8_MOD)    OPCODE_ENTRY(I16_MOD)    OPCODE_ENTRY(I32_MOD)    OPCODE_ENTRY(I64_MOD)    /* modulo */
OPCODE_ENTRY(I8_POW)    OPCODE_ENTRY(I16_POW)    OPCODE_ENTRY(I32_POW)    OPCODE_ENTRY(I64_POW)    /* power */

OPCODE_ENTRY(I8_INC)    OPCODE_ENTRY(I16_INC)    OPCODE_ENTRY(I32_INC)    OPCODE_ENTRY(I64_INC)    /* increment */
OPCODE_ENTRY(I8_DEC)    OPCODE_ENTRY(I16_DEC)    OPCODE_ENTRY(I32_DEC)    OPCODE_ENTRY(I64_DEC)    /* decrement */

OPCODE_ENTRY(I8_NEG)    OPCODE_ENTRY(I16_NEG)    OPCODE_ENTRY(I32_NEG)    OPCODE_ENTRY(I64_NEG)    /* negative */
OPCODE_ENTRY(I8_ABS)    OPCODE_ENTRY(I16_ABS)    OPCODE_ENTRY(I32_ABS)    OPCODE_ENTRY(I64_ABS)    /* absolute */

OPCODE_ENTRY(I8_LT)     OPCODE_ENTRY(I16_LT)     OPCODE_ENTRY(I32_LT)     OPCODE_ENTRY(I64_LT)     /* less than */
OPCODE_ENTRY(I8_LE)     OPCODE_ENTRY(I16_LE)     OPCODE_ENTRY(I32_LE)     OPCODE_ENTRY(I64_LE)     /* less than or equal */
OPCODE_ENTRY(I8_GT)     OPCODE_ENTRY(I16_GT)     OPCODE_ENTRY(I32_GT)     OPCODE_ENTRY(I64_GT)     /* greater than */
OPCODE_ENTRY(I8_GE)     OPCODE_ENTRY(I16_GE)     OPCODE_ENTRY(I32_GE)     OPCODE_ENTRY(I64_GE)     /* greater than or equal */

////////////////////////////////////////////////////////////////
// Floating Math Functions                                    //
////////////////////////////////////////////////////////////////

// F32 Math (IEEE-754 compliant) */

OPCODE_ENTRY(F32_ADD)                   /* add */
OPCODE_ENTRY(F32_SUB)                   /* subtract */
OPCODE_ENTRY(F32_MUL)                   /* multiply */
OPCODE_ENTRY(F32_DIV)                   /* divide */
OPCODE_ENTRY(F32_MOD)                   /* modulo */
OPCODE_ENTRY(F32_POW)                   /* to the power of */

OPCODE_ENTRY(F32_EQU)                   /* equals */
OPCODE_ENTRY(F32_NEQ)                   /* not equals */

OPCODE_ENTRY(F32_NEG)                   /* negative */
OPCODE_ENTRY(F32_ABS)                   /* absolute */

OPCODE_ENTRY(F32_LT)                    /* less than */
OPCODE_ENTRY(F32_LE)                    /* less than or equal */
OPCODE_ENTRY(F32_GT)                    /* greater than */
OPCODE_ENTRY(F32_GE)                    /* greater than or equal */

// F64 Math (IEEE-754 compliant)

OPCODE_ENTRY(F64_ADD)                   /* add */
OPCODE_ENTRY(F64_SUB)                   /* subtract */
OPCODE_ENTRY(F64_MUL)                   /* multiply */
OPCODE_ENTRY(F64_DIV)                   /* divide */
OPCODE_ENTRY(F64_MOD)                   /* modulo */
OPCODE_ENTRY(F64_POW)                   /* to the power of */

OPCODE_ENTRY(F64_EQU)                   /* equals */
OPCODE_ENTRY(F64_NEQ)                   /* not equals */

OPCODE_ENTRY(F64_NEG)                   /* negative */
OPCODE_ENTRY(F64_ABS)                   /* absolute */

OPCODE_ENTRY(F64_LT)                    /* less than */
OPCODE_ENTRY(F64_LE)                    /* less than or equal */
OPCODE_ENTRY(F64_GT)                    /* greater than */
OPCODE_ENTRY(F64_GE)                    /* greater than or equal */

////////////////////////////////////////////////////////////////
// Complex Data Structure Opcodes                             //
////////////////////////////////////////////////////////////////

// String

OPCODE_ENTRY(STR_NEW)                   /* [ opcode | 32bit length | str (string_data...) ] - pushes the address of a new string with the length @length and content @string_data to the stack */

OPCODE_ENTRY(STR_CONCAT)                /* concatenates the string at @stack_top - sizeof(address) with the topmost string (addr; @stack_top), reallocating the first string if required */
OPCODE_ENTRY(STR_DUP)                   /* creates a copy of the topmost string (addr; @stack_top) in the stack */

OPCODE_ENTRY(STR_EQU)                   /* pushes bool with 0, if the strings are not matching or 1 if they are matching, to stack */
OPCODE_ENTRY(STR_GET)                   /* pushes an 8 bit character from the string at @stack_top - sizeof(u32) at the given index (u32; @stack_top) */
OPCODE_ENTRY(STR_SET)                   /* sets the character at the index (u32; @stack_top) to the given value (char; @stack_top - sizeof(u32)) in the string (addr; @stack_top - sizeof(u32) - sizeof(u8)) */
OPCODE_ENTRY(STR_LEN)                   /* pushes the length (u32) of the topmost string (addr; @stack_top) to the stack */

// Array

OPCODE_ENTRY(ARR_NEW)                   /* [ opcode | 32bit field : (1bit has_data, 2bit value_size, 29bit length) | array data : (value...) ] - pushes the address of a new array to the stack and initializes it to @data if present */

OPCODE_ENTRY(ARR_GET)                   /* pushes the value (with the size of array.@type) in the array (addr; @stack_top - sizeof(u32)) at the index at @stack_top (u32; @stack_top) to the top of the stack */
OPCODE_ENTRY(ARR_SET)                   /* sets the element at the index (u32; @stack_top - sizeof(array.@type)) to the value at the top of the stack (array.@type; @stack_top) */
OPCODE_ENTRY(ARR_LEN)                   /* pushes the length (u32) of the array (addr; @stack_top) to the stack */

// Struct

OPCODE_ENTRY(STRUCT_NEW)                /* [ opcode | 16bit field : (1bit has_data, 15bit size) | array data : (value...) ] - pushes the address of a new struct to the stack and initializes its fields to @data if present */

OPCODE_ENTRY(STRUCT_GET_8)              /* [ opcode | 16bit offset ] - pushes the 8bit  value at @offset in the struct (addr; @stack_top) to the top of the stack */
OPCODE_ENTRY(STRUCT_GET_16)             /* [ opcode | 16bit offset ] - pushes the 16bit value at @offset in the struct (addr; @stack_top) to the top of the stack */
OPCODE_ENTRY(STRUCT_GET_32)             /* [ opcode | 16bit offset ] - pushes the 32bit value at @offset in the struct (addr; @stack_top) to the top of the stack */
OPCODE_ENTRY(STRUCT_GET_64)             /* [ opcode | 16bit offset ] - pushes the 64bit value at @offset in the struct (addr; @stack_top) to the top of the stack */

OPCODE_ENTRY(STRUCT_SET_8)              /* [ opcode | 16bit offset ] - sets the 8bit  variable in the struct (addr; @stack_top - sizeof(u8))  to the 8bit  value at @stack_top */
OPCODE_ENTRY(STRUCT_SET_16)             /* [ opcode | 16bit offset ] - sets the 16bit variable in the struct (addr; @stack_top - sizeof(u16)) to the 16bit value at @stack_top */
OPCODE_ENTRY(STRUCT_SET_32)             /* [ opcode | 16bit offset ] - sets the 32bit variable in the struct (addr; @stack_top - sizeof(u32)) to the 32bit value at @stack_top */
OPCODE_ENTRY(STRUCT_SET_64)             /* [ opcode | 16bit offset ] - sets the 64bit variable in the struct (addr; @stack_top - sizeof(u64)) to the 64bit value at @stack_top */

////////////////////////////////////////////////////////////////
// Other Opcodes                                              //
////////////////////////////////////////////////////////////////

// Type Conversion

OPCODE_ENTRY(TYPE_CONV_STATIC)          /* [ opcode | 8bit type : (4bit type_from, 4bit type_to) ] - pops @type_from off the stack, converts it to @type_to and pushes the converted value to the stack */
OPCODE_ENTRY(TYPE_CONV_REINTERPRET)     /* [ opcode | 8bit type : (4bit type_from, 4bit type_to) ] - pops @type_from off the stack resizes it to the size of @type_to and pushes the converted value to the stack */
