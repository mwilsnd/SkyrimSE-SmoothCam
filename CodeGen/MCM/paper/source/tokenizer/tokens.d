module tokenizer.tokens;

/**
 * List of token types
 */
enum Tok {
    OpenCurl,           // { // Papyrus uses {...} as a multi-line comment
    CloseCurl,          // }
    OpenParen,          // (
    CloseParen,         // )
    OpenBrace,          // [
    CloseBrace,         // ]
    Assignment,         // =
    Dot,                // .
    Comma,              // ,
    Arrow,              // -> Not valid in normal papyrus, used by us
    Colon,              // : Not valid in normal papyrus, used by us
    SemiColon,          // ; (Used as a comment)
    Quote,              // "
    Exclamation,        // !
    Hash,               // # Not valid in normal papyrus, used by us
    OpenAngle,          // <
    CloseAngle,         // >
    Space,              // ' '
    CharReturn,         // \r
    NewLine,            // \n
    Tab,                // \t
    Continuation,       // \
    Slash,              // /
    Plus,               // +
    Minus,              // -
    Star,               // *
    Equals,             // ==
    NotEquals,          // !=
    GEQ,                // >=
    LEQ,                // <=

    Type,               // int, string, float
    RealType,           // real_int, real_string
    Mangle,             // Requires that type is string, will "mangle" the string to prevent string cache messing with it

    NumericValue,       // -1, 2.5
    BoolValue,          // true, false
    StringValue,        // Any character sequence (including escaped quotes), not including the outer quotes
    OtherValue,         // Any other legal character sequence given as an assignment (var name, return of a function call)

    Comment,            // Any comment contents
    MultiLineComment,   // Curly brace or (;/.../;) contents, in normal papyrus this is a multi-line comment

    // Language keywords
    kScriptName,
    kExtends,
    kImport,
    kFunction,
    kEndFunction,
    kEvent,
    kEndEvent,
    kIf,
    kElseIf,
    kElse,
    kEndIf,
    kProperty,
    kEndProperty,
    kState,
    kEndState,
    kAs,
    kGlobal,
    kNative,
    kAuto,
    kConditional,
    kHidden,
    kAutoReadOnly,
    kSelf,
    kWhile,
    kEndWhile,
    kLength,
    kNew,
    kParent,
    kReturn,

    // Builtin types
    tBool,
    tFloat,
    tInt,
    tString,

    // Extended types
    tNone,              // None type
    tLiteral,           // A literal type takes the assignment contents verbatim (copy+paste)
}

/** 
 * Convert a type token to a string
 * Params:
 *   ty = Any Tok.t* token
 * Returns: string
 */
string typeToString(Tok ty) @safe @nogc nothrow {
    switch (ty) {
        case Tok.tBool:
            return "bool";
        case Tok.tInt:
            return "int";
        case Tok.tFloat:
            return "float";
        case Tok.tString:
            return "string";
        case Tok.tLiteral:
            return "literal";
        default: assert(0);
    }
}