/*

    Project.

        XSC - The XtremeScript Compiler Version 0.8

    Abstract.

        Lexical analyzer module header

    Date Created.

        8.28.2002

    Author.

        Alex Varanese

*/

#ifndef XSC_LEXER
#define XSC_LEXER

// ---- Include Files -------------------------------------------------------------------------

    #include "xsc.h"
    
// ---- Constants -----------------------------------------------------------------------------

    // ---- Lexemes ---------------------------------------------------------------------------

        #define MAX_LEXEME_SIZE                 1024    // Maximum individual lexeme size

    // ---- Operators -------------------------------------------------------------------------

        #define MAX_OP_STATE_COUNT              32      // Maximum number of operator
                                                        // states

    // ---- Delimiters ------------------------------------------------------------------------

        #define MAX_DELIM_COUNT                 24      // Maximum number of delimiters

    // ---- Lexer States ----------------------------------------------------------------------

        #define LEX_STATE_UNKNOWN               0       // Unknown lexeme type

        #define LEX_STATE_START                 1       // Start state

        #define LEX_STATE_INT                   2       // Integer
        #define LEX_STATE_FLOAT                 3       // Float

        #define LEX_STATE_IDENT                 4       // Identifier


        #define LEX_STATE_OP                    5       // Operator
        #define LEX_STATE_DELIM                 6       // Delimiter    

        #define LEX_STATE_STRING                7       // String value
        #define LEX_STATE_STRING_ESCAPE         8       // Escape sequence
        #define LEX_STATE_STRING_CLOSE_QUOTE    9       // String closing quote

    // ---- Token Types -----------------------------------------------------------------------

        #define TOKEN_TYPE_END_OF_STREAM        0       // End of the token stream
        #define TOKEN_TYPE_INVALID              1       // Invalid token

        #define TOKEN_TYPE_INT                  2       // Integer
        #define TOKEN_TYPE_FLOAT                3       // Float

        #define TOKEN_TYPE_IDENT                4       // Identifier

        #define TOKEN_TYPE_RSRVD_VAR            5       // var/var []
        #define TOKEN_TYPE_RSRVD_TRUE           6       // true
        #define TOKEN_TYPE_RSRVD_FALSE          7       // false
        #define TOKEN_TYPE_RSRVD_IF             8       // if
        #define TOKEN_TYPE_RSRVD_ELSE           9       // else
        #define TOKEN_TYPE_RSRVD_BREAK          10      // break
        #define TOKEN_TYPE_RSRVD_CONTINUE       11      // continue
        #define TOKEN_TYPE_RSRVD_FOR            12      // for
        #define TOKEN_TYPE_RSRVD_WHILE          13      // while
        #define TOKEN_TYPE_RSRVD_FUNC           14      // func
        #define TOKEN_TYPE_RSRVD_RETURN         15      // return
        #define TOKEN_TYPE_RSRVD_HOST           16      // host

        #define TOKEN_TYPE_OP                   18      // Operator

        #define TOKEN_TYPE_DELIM_COMMA          19      // ,
        #define TOKEN_TYPE_DELIM_OPEN_PAREN     20      // (
        #define TOKEN_TYPE_DELIM_CLOSE_PAREN    21      // )
        #define TOKEN_TYPE_DELIM_OPEN_BRACE     22      // [
        #define TOKEN_TYPE_DELIM_CLOSE_BRACE    23      // ]
        #define TOKEN_TYPE_DELIM_OPEN_CURLY_BRACE   24  // {
        #define TOKEN_TYPE_DELIM_CLOSE_CURLY_BRACE  25  // }
        #define TOKEN_TYPE_DELIM_SEMICOLON      26      // ;

        #define TOKEN_TYPE_STRING               27      // String

    // ---- Operators -------------------------------------------------------------------------

        // ---- Arithmetic

        #define OP_TYPE_ADD                     0       // +
        #define OP_TYPE_SUB                     1       // -
        #define OP_TYPE_MUL                     2       // *
        #define OP_TYPE_DIV                     3       // /
        #define OP_TYPE_MOD                     4       // %
        #define OP_TYPE_EXP                     5       // ^
        #define OP_TYPE_CONCAT                  35       // $

        #define OP_TYPE_INC                     15      // ++
        #define OP_TYPE_DEC                     17      // --

        #define OP_TYPE_ASSIGN_ADD              14      // +=
        #define OP_TYPE_ASSIGN_SUB              16      // -=
        #define OP_TYPE_ASSIGN_MUL              18      // *=
        #define OP_TYPE_ASSIGN_DIV              19      // /=
        #define OP_TYPE_ASSIGN_MOD              20      // %=
        #define OP_TYPE_ASSIGN_EXP              21      // ^=
        #define OP_TYPE_ASSIGN_CONCAT           36      // $=

        // ---- Bitwise

        #define OP_TYPE_BITWISE_AND             6       // &
        #define OP_TYPE_BITWISE_OR              7       // |
        #define OP_TYPE_BITWISE_XOR             8       // #
        #define OP_TYPE_BITWISE_NOT             9       // ~
        #define OP_TYPE_BITWISE_SHIFT_LEFT      30      // <<
        #define OP_TYPE_BITWISE_SHIFT_RIGHT     32      // >>

        #define OP_TYPE_ASSIGN_AND              22      // &=
        #define OP_TYPE_ASSIGN_OR               24      // |=
        #define OP_TYPE_ASSIGN_XOR              26      // #=
        #define OP_TYPE_ASSIGN_SHIFT_LEFT       33      // <<=
        #define OP_TYPE_ASSIGN_SHIFT_RIGHT      34      // >>=

        // ---- Logical

        #define OP_TYPE_LOGICAL_AND             23      // &&
        #define OP_TYPE_LOGICAL_OR              25      // ||
        #define OP_TYPE_LOGICAL_NOT             10      // !

        // ---- Relational

        #define OP_TYPE_EQUAL                   28      // ==
        #define OP_TYPE_NOT_EQUAL               27      // !=
        #define OP_TYPE_LESS                    12      // <
        #define OP_TYPE_GREATER                 13      // >
        #define OP_TYPE_LESS_EQUAL              29      // <=
        #define OP_TYPE_GREATER_EQUAL           31      // >=

        // ---- Assignment

        #define OP_TYPE_ASSIGN                  11      // =

// ---- Data Structures -----------------------------------------------------------------------

    typedef int Token;                                  // Token type

    typedef struct _LexerState                          // The lexer's state
    {
        int iCurrLineIndex;                             // Current line index
        LinkedListNode * pCurrLine;                     // Current line node pointer
        Token CurrToken;                                // Current token
        char pstrCurrLexeme [ MAX_LEXEME_SIZE ];        // Current lexeme
        int iCurrLexemeStart;                           // Current lexeme's starting index
        int iCurrLexemeEnd;                             // Current lexeme's ending index
        int iCurrOp;                                    // Current operator
    }
        LexerState;

    typedef struct _OpState                             // Operator state
    { 
        char cChar;                                     // State character
        int iSubStateIndex;                             // Index into sub state array where sub
        int iSubStateCount;                             // Number of substates
        int iIndex;                                     // Operator index
    }
        OpState;

// ---- Function Prototypes -------------------------------------------------------------------

    void ResetLexer ();
    void CopyLexerState ( LexerState & pDestState, LexerState & pSourceState );

    int GetOpStateIndex ( char cChar, int iCharIndex, int iSubStateIndex, int iSubStateCount );
    int IsCharOpChar ( char cChar, int iCharIndex );
    OpState GetOpState ( int iCharIndex, int iStateIndex );
    int IsCharDelim ( char cChar );
    int IsCharWhitespace ( char cChar );
    int IsCharNumeric ( char cChar );
    int IsCharIdent ( char cChar );

    char GetNextChar ();
    Token GetNextToken ();
    void RewindTokenStream ();
    Token GetCurrToken ();
    char * GetCurrLexeme ();
    void CopyCurrLexeme ( char * pstrBuffer );
    int GetCurrOp ();
    char GetLookAheadChar ();

    char * GetCurrSourceLine ();
    int GetCurrSourceLineIndex ();
    int GetLexemeStartIndex ();

#endif