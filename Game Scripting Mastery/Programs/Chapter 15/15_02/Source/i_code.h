/*

    Project.

        XSC - The XtremeScript Compiler Version 0.8

    Abstract.

        I-code module header

    Date Created.

        9.2.2002

    Author.

        Alex Varanese

*/

#ifndef XSC_I_CODE
#define XSC_I_CODE

// ---- Include Files -------------------------------------------------------------------------

    #include "xsc.h"
    #include "func_table.h"
    
// ---- Constants -----------------------------------------------------------------------------

    // ---- I-Code Node Types -----------------------------------------------------------------

        #define ICODE_NODE_INSTR        0               // An I-code instruction
        #define ICODE_NODE_SOURCE_LINE  1               // Source-code annotation
        #define ICODE_NODE_JUMP_TARGET  2               // A jump target

    // ---- I-Code Instruction Opcodes --------------------------------------------------------

        #define INSTR_MOV               0

        #define INSTR_ADD               1
        #define INSTR_SUB               2
        #define INSTR_MUL               3
        #define INSTR_DIV               4
        #define INSTR_MOD               5
        #define INSTR_EXP               6
        #define INSTR_NEG               7
        #define INSTR_INC               8
        #define INSTR_DEC               9

        #define INSTR_AND               10
        #define INSTR_OR                11
        #define INSTR_XOR               12
        #define INSTR_NOT               13
        #define INSTR_SHL               14
        #define INSTR_SHR               15

        #define INSTR_CONCAT            16
        #define INSTR_GETCHAR           17
        #define INSTR_SETCHAR           18

        #define INSTR_JMP               19
        #define INSTR_JE                20
        #define INSTR_JNE               21
        #define INSTR_JG                22
        #define INSTR_JL                23
        #define INSTR_JGE               24
        #define INSTR_JLE               25

        #define INSTR_PUSH              26
        #define INSTR_POP               27

        #define INSTR_CALL              28
        #define INSTR_RET               29
        #define INSTR_CALLHOST          30

        #define INSTR_PAUSE             31
        #define INSTR_EXIT              32

    // ---- Operand Types ---------------------------------------------------------------------

        #define OP_TYPE_INT                 0           // Integer literal value
        #define OP_TYPE_FLOAT               1           // Floating-point literal value
        #define OP_TYPE_STRING_INDEX        2           // String literal value
        #define OP_TYPE_VAR                 3           // Variable
        #define OP_TYPE_ARRAY_INDEX_ABS     4           // Array with absolute index
        #define OP_TYPE_ARRAY_INDEX_VAR     5           // Array with relative index
        #define OP_TYPE_JUMP_TARGET_INDEX   6           // Jump target index
        #define OP_TYPE_FUNC_INDEX          7           // Function index
        #define OP_TYPE_REG                 9           // Register

// ---- Data Structures -----------------------------------------------------------------------

    typedef struct _Op                                  // An I-code operand
    {
        int iType;                                      // Type
        union                                           // The value
        {
            int iIntLiteral;                            // Integer literal
            float fFloatLiteral;                        // Float literal
            int iStringIndex;                           // String table index
            int iSymbolIndex;                           // Symbol table index
            int iJumpTargetIndex;                       // Jump target index
            int iFuncIndex;                             // Function index
            int iRegCode;                               // Register code
        };
        int iOffset;                                    // Immediate offset
        int iOffsetSymbolIndex;                         // Offset symbol index
    }
        Op;

    typedef struct _ICodeInstr                          // An I-code instruction
    {
        int iOpcode;                                    // Opcode
        LinkedList OpList;                              // Operand list
    }
        ICodeInstr;

    typedef struct _ICodeNode                           // An I-code node
    {
        int iType;                                      // The node type
        union
        {
            ICodeInstr Instr;                           // The I-code instruction
            char * pstrSourceLine;                      // The source line with which this
                                                        // instruction is annotated
            int iJumpTargetIndex;                       // The jump target index
        };
    }
        ICodeNode;

// ---- Function Prototypes -------------------------------------------------------------------

    ICodeNode * GetICodeNodeByImpIndex ( int iFuncIndex, int iInstrIndex );

    void AddICodeSourceLine ( int iFuncIndex, char * pstrSourceLine );

    int AddICodeInstr ( int iFuncIndex, int iOpcode );
    Op * GetICodeOpByIndex ( ICodeNode * pInstr, int iOpIndex );
    void AddICodeOp ( int iFuncIndex, int iInstrIndex, Op Value );

    void AddIntICodeOp ( int iFuncIndex, int iInstrIndex, int iValue );
    void AddFloatICodeOp ( int iFuncIndex, int iInstrIndex, float fValue );
    void AddStringICodeOp ( int iFuncIndex, int iInstrIndex, int iStringIndex );
    void AddVarICodeOp ( int iFuncIndex, int iInstrIndex, int iSymbolIndex );
    void AddArrayIndexAbsICodeOp ( int iFuncIndex, int iInstrIndex, int iArraySymbolIndex, int iOffset );
    void AddArrayIndexVarICodeOp ( int iFuncIndex, int iInstrIndex, int iArraySymbolIndex, int iOffsetSymbolIndex );
    void AddFuncICodeOp ( int iFuncIndex, int iInstrIndex, int iOpFuncIndex );
    void AddRegICodeOp ( int iFuncIndex, int iInstrIndex, int iRegCode );
    void AddJumpTargetICodeOp ( int iFuncIndex, int iInstrIndex, int iTargetIndex );

    int GetNextJumpTargetIndex ();
    void AddICodeJumpTarget ( int iFuncIndex, int iTargetIndex );

#endif