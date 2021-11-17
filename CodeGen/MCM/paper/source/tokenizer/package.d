module tokenizer;
public import tokenizer.tokens;
import keywords;
import result;

import std.container.array;

/// A generic token
struct Token {
    /// Type of token
    Tok type = void;
    /// Value of the token
    string value = void;
}

alias TokenStream = Array!Token;

private {
    /// Characters which are valid to use in a var or type name
    enum validVarChars =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ" ~
        "0123456789" ~
        "_";

    /// Characters which are valid to use in a numeric literal
    enum numberChars = "01234567890.-f";

    /// Characters that count as valid whitespace
    enum spaceChars = " \r\n\t";

    enum CharacterClass : ushort {
        Decl, // VarChars
        Numeric // NumberChars
    }

    // Generic parser state
    struct TokenState {
        // Expect to read a new token
        bool expect = true;
        // Currently reading a token
        bool reading = false;
        // the start position reading began at
        ulong startPos = 0;
    }

    nothrow:

    /**
     * Look ahead 1 character
     * Params:
     *   source = Input string
     *   curPos = Current position, look ahead curPos+1
     * Returns: Character or 0x0
     */
    char lookAhead()(auto ref const(string) source, ulong curPos) @safe @nogc nothrow {
        if (source.length <= curPos) return 0x0;
        return source[curPos + 1];
    }

    /**
     * Look behind 1 character
     * Params:
     *   source = Input string
     *   curPos = Current position, look behind curPos-1
     * Returns: Character or 0x0
     */
    char lookBehind()(auto ref const(string) source, ulong curPos) @safe @nogc nothrow {
        if (curPos == 0) return 0x0;
        return source[curPos - 1];
    }

    /**
     * Checks if the given character is valid whitespace
     * Params:
     *   c = Input character
     * Returns: Is whitespace
     */
    bool isWhitespace(char c) @safe @nogc nothrow {
        foreach (char space; spaceChars)
            if (space == c) return true;
        return false;
    }

    /** 
     * Checks if the given character is a valid number literal character
     * Params:
     *   c = Input character
     * Returns: Is numeric
     */
    bool isNumeric(char c) @safe @nogc nothrow {
        foreach (char n; numberChars)
            if (c == n) return true;
        return false;
    }

    /** 
     * Checks if the given character is a valid var/decl character
     * Params:
     *   c = Input character
     * Returns: Is var char
     */
    bool isVarChar(char c) @safe @nogc nothrow {
        foreach (char v; validVarChars)
            if (c == v) return true;
        return false;
    }

    /**
     * Return the position of the following new line
     * Params:
     *   source = Input string
     * Returns: Position to continue reading at, after the current line
     */
    Result!ulong nextLine()(auto ref const(string) source) @safe @nogc nothrow {
        foreach (ulong i, char c; source) {
            if (c == '\r' && lookAhead(source, i) == '\n') return Result!(ulong).make(i+2);
            if (c == '\n') return Result!(ulong).make(i+1);
        }
        return Result!(ulong).fail("Expected new line but got end of file!");
    }

    /** 
     * Read until `delim` is found, return the position right after `delim`
     * Params:
     *   source = Inout string
     *   delim = Substring to read to
     * Returns: Position to continue reading at, following `delim`
     */
    Result!ulong readUntil()(auto ref const(string) source, auto ref const(string) delim) @safe nothrow {
        foreach (ulong i, char c; source) {
            bool match = true;

            foreach (ulong j, char d; delim) {
                if (j == 0) {
                    if (d != c) {
                        match = false;
                        break;
                    }
                } else {
                    if (lookAhead(source, i+j-1) != delim[j]) {
                        match = false;
                        break;
                    }
                }
            }

            if (match)
                return Result!(ulong).make(i+delim.length);
        }

        return Result!(ulong).fail("Expected '" ~ delim ~ "' but got end of file!");
    }

    /**
     * Read a string literal
     * Params:
     *   source = Input string
     *   contents = The contents of the string literal
     * Returns: Position after the ending quote to continue reading
     */
    Result!ulong readStringLiteral()(auto ref const(string) source, out string contents) @safe @nogc nothrow {
        foreach (ulong i, char c; source) {
            if (c == '"') {
                if (lookBehind(source, i) == '\\') continue;

                contents = source[0..i];
                return Result!(ulong).make(i);
            }
        }
        return Result!(ulong).fail("Expected '\"' character but got end of file!");
    }

    /** 
     * Read contents of a balanced character pair
     * Params:
     *   source = Input string
     *   open = Open character
     *   close = Close character
     *   contents = Contents of the matched pair
     * Returns: Position after closing pair to continue reading
     */
    Result!ulong readBalanced()(auto ref const(string) source, char open, char close, out string contents)
        @safe nothrow
    {
        ulong nest = 1;
        foreach (ulong i, char c; source) {
            if (c == open)
                ++nest;
            else if (c == close) {
                if (--nest == 0) {
                    contents = source[0..i];
                    return Result!(ulong).make(i+1);
                }
            }
        }
        return Result!(ulong).fail("Expected matching '" ~ close ~ "' but got end of file!");
    }


}

/**
 * Given an input string, write a stream of tokens to the output array
 * Params:
 *   source = Source string to tokenize
 *   tokens = Output token stream
 */
public Result!bool tokenize(ref const(string) source, out Array!Token tokens) @trusted {
    tokens.reserve(2048);
    CharacterClass expectClass;
    TokenState state;
    
    bool jump = false;
    ulong jumpTo = 0;
    void setJump(ulong to) @safe @nogc nothrow {
        jump = true;
        jumpTo = to;
    }

    void insertTok(ulong i) @trusted {
        final switch (expectClass) {
            case CharacterClass.Decl:
                const auto str = (i != 0) ? source[state.startPos..i] : source[state.startPos..$];
                const auto kwd = isKeyword(str);
                const auto ty = isType(str);

                // Bool
                if (str == "true" || str == "false")
                    tokens.insertBack(Token(Tok.BoolValue, str));
                else {
                    const auto tok = kwd.isOk() ? kwd.unwrap() : (ty.isOk() ? ty.unwrap() : Tok.OtherValue);
                    tokens.insertBack(Token(tok, str));
                }
            
                state.expect = true;
                state.reading = false;
                break;

            case CharacterClass.Numeric:
                if (i != 0)
                    tokens.insertBack(Token(Tok.NumericValue, source[state.startPos..i]));
                else
                    tokens.insertBack(Token(Tok.NumericValue, source[state.startPos..$]));
                state.expect = true;
                state.reading = false;
                break;
        }
    }

    bool lastTok(ref Token tok) @safe @nogc nothrow {
        if (tokens.empty()) return false;
        tok = tokens.back();
        return true;
    }

    foreach (ulong i, char c; source) { eval:
        if (jump) {
            if (i != jumpTo) continue;
            jump = false;
        }

        if (state.expect) {
            // We have a number of single char and special tokens, look for those
            switch (c) {
                case ' ':
                    tokens.insertBack(Token(Tok.Space, " "));
                    continue;

                case '\r':
                    tokens.insertBack(Token(Tok.CharReturn, "\r"));
                    continue;

                case '\n':
                    tokens.insertBack(Token(Tok.NewLine, "\n"));
                    continue;

                case '\t':
                    tokens.insertBack(Token(Tok.Tab, "\t"));
                    continue;
                    
                case '{':
                    tokens.insertBack(Token(Tok.OpenCurl, "{"));
                    string contents;
                    const auto loc = readBalanced(source[i+1..$], '{', '}', contents);
                    if (!loc.isOk()) return Result!(bool).failFrom(loc);
                    jump = true;
                    jumpTo = loc.unwrap() + i+1;
                    tokens.insertBack(Token(Tok.MultiLineComment, contents));
                    tokens.insertBack(Token(Tok.CloseCurl, "}"));
                    continue;

                case '}':
                    tokens.insertBack(Token(Tok.CloseCurl, "}"));
                    continue;

                case '(':
                    tokens.insertBack(Token(Tok.OpenParen, "("));
                    continue;

                case ')':
                    tokens.insertBack(Token(Tok.CloseParen, ")"));
                    continue;

                case '[':
                    tokens.insertBack(Token(Tok.OpenBrace, "["));
                    continue;

                case ']':
                    tokens.insertBack(Token(Tok.CloseBrace, "]"));
                    continue;

                case '.':
                    tokens.insertBack(Token(Tok.Dot, "."));
                    continue;

                case ',':
                    tokens.insertBack(Token(Tok.Comma, ","));
                    continue;

                case ':':
                    tokens.insertBack(Token(Tok.Colon, ":"));
                    continue;

                case '#':
                    tokens.insertBack(Token(Tok.Hash, "#"));
                    continue;
                
                case '\\':
                    tokens.insertBack(Token(Tok.Continuation, "\\"));
                    continue;
                
                // Compare operators
                case '=':
                    // Make sure the next token is not also '='
                    if (lookAhead(source, i) != '=' && lookBehind(source, i) != '=')
                        tokens.insertBack(Token(Tok.Assignment, "="));
                    else {
                        tokens.insertBack(Token(Tok.Equals, "=="));
                        setJump(i+2);
                    }
                    continue;

                case '!':
                    // Make sure the next token is not '='
                    if (lookAhead(source, i) != '=')
                        tokens.insertBack(Token(Tok.Exclamation, "!"));
                    else {
                        tokens.insertBack(Token(Tok.NotEquals, "!="));
                        setJump(i+2);
                    }
                    continue;

                case '<':
                    // Make sure the next token is not '='
                    if (lookAhead(source, i) != '=')
                        tokens.insertBack(Token(Tok.OpenAngle, "<"));
                    else {
                        tokens.insertBack(Token(Tok.LEQ, "<="));
                        setJump(i+2);
                    }
                    continue;
                
                case '>':
                    // Make sure the next token is not '='
                    if (lookAhead(source, i) != '=')
                        tokens.insertBack(Token(Tok.CloseAngle, ">"));
                    else {
                        tokens.insertBack(Token(Tok.GEQ, ">="));
                        setJump(i+2);
                    }
                    continue;

                // Comment
                case ';':
                    tokens.insertBack(Token(Tok.SemiColon, ";"));
                    // Check for the wacky multi-line syntax
                    if (lookAhead(source, i) == '/') {
                        tokens.insertBack(Token(Tok.Slash, "/"));
                        const auto err = readUntil(source[i..$], "/;");
                        if (!err.ok) return Result!(bool).failFrom(err);
                        jumpTo = err.value + i;
                        jump = true;

                        tokens.insertBack(Token(Tok.MultiLineComment, source[i..err.value-2]));
                        tokens.insertBack(Token(Tok.Slash, "/"));
                        tokens.insertBack(Token(Tok.SemiColon, ";"));
                    } else {
                        // We are now reading a comment, read until the next line and insert that as a comment
                        const auto err = nextLine(source[i+1..$]);
                        if (!err.ok) return Result!(bool).failFrom(err);
                        jumpTo = err.value + i;
                        jump = true;
                        tokens.insertBack(Token(Tok.Comment, source[i+1..jumpTo]));
                    }
                    continue;

                // String literal
                case '"':
                    string contents;
                    const auto err = readStringLiteral(source[i+1..$], contents);
                    if (!err.ok) return Result!(bool).failFrom(err);
                    jumpTo = err.value + i+2;
                    jump = true;
                    tokens.insertBack(Token(Tok.StringValue, contents));
                    continue;

                case '-':
                    // Check if this is an arrow operator
                    if (lookAhead(source, i) == '>') {
                        jump = true;
                        jumpTo = i+2;
                        tokens.insertBack(Token(Tok.Arrow, "->"));
                        continue;
                    // Or part of a numeric
                    } else {
                        auto a = lookAhead(source, i);
                        if (!isNumeric(a) || a == 'f' || a == '-') {
                            tokens.insertBack(Token(Tok.Minus, "-"));
                            continue;
                        }
                    }
                    break;

                case '+':
                    tokens.insertBack(Token(Tok.Plus, "+"));
                    continue;

                case '*':
                    tokens.insertBack(Token(Tok.Star, "*"));
                    continue;

                default: break;
            }
            
            state.expect = false;
            state.reading = true;
            state.startPos = i;

            if (isNumeric(c)) {
                // For floats ending in 'f' - We need to sanity check
                const auto prev = lookBehind(source, i);
                if (isNumeric(prev) && prev != 'f') {
                    expectClass = CharacterClass.Numeric;
                } else
                    if (c != 'f')
                        expectClass = CharacterClass.Numeric;
                    else
                        goto varChar;

            } else {
varChar:
                if (isVarChar(c))
                    expectClass = CharacterClass.Decl;
                else
                    return Result!(bool).fail("Unexpected character '" ~ c ~ "'!");
            }

        } else {
            final switch (expectClass) {
                case CharacterClass.Decl:
                    if (isVarChar(c)) continue;
                    insertTok(i);
                    goto eval; // We still need to evaluate this character

                case CharacterClass.Numeric:
                    if (isNumeric(c)) continue;
                    insertTok(i);
                    goto eval; // We still need to evaluate this character

            }
        }
    }

    if (state.reading)
        insertTok(0);

    return Result!(bool).make(true);
}