module tokenizer.utils;
import tokenizer : Token, TokenStream;
import tokenizer.tokens : Tok;
import result;
import std.array;

auto stripStringCR()(auto const ref string str) {
    return replace(str, "\r\n", "\n");
}

/** 
 * Gets a subset of tokens from the input stream contained in a pair of matching outer tokens
 * Params:
 *   stream = Stream to search
 *   open = Token delimiting an opening scope
 *   close = Token delimiting a closing scope
 * Returns: Sliced token stream
 */
Result!TokenStream getBalancedContent()(auto ref const(TokenStream) stream, Tok open, Tok close)
    @trusted
{
    import std.conv : to;

    ulong depth = 1;
    for (ulong i = 0; i < stream.length(); i++) {
        const Token tok = stream[i];
        if (tok.type == open)
            ++depth;
        else if (tok.type == close) {
            if (--depth == 0) {
                return Result!(TokenStream).make(TokenStream(stream[0..i]));
            }
        }
    }
    return Result!(TokenStream).fail(
        "Expected matching closing token '" ~ to!string(close) ~ "' but got end of stream!"
    );
}

/** 
 * Checks if the given token is whitespace
 * Params:
 *   tok = Input token
 * Returns: Is whitespace
 */
bool isWhitespace(ref inout(Token) tok) @safe @nogc nothrow {
    return tok.type == Tok.Space || tok.type == Tok.CharReturn ||
        tok.type == Tok.NewLine || tok.type == Tok.Tab;
}

/** 
 * Convert the given token stream to a string
 * Params:
 *   stream = Input stream
 * Returns: Output string
 */
string streamToString()(auto ref const(TokenStream) stream) @trusted {
    string output;
    foreach (ref const(Token) tok; stream) {
        if (tok.type == Tok.StringValue)
            output ~= "\"" ~ tok.value ~ "\"";
        else
            output ~= tok.value;
    }

    return output;
}

/** 
 * Gets the current line indentation from the given position in the stream
 *
 * Assumes the token right before the given position is whitespace, runs until it sees a line break
 * Params:
 *   stream = Input stream
 *   position = Current position in the stream
 * Returns: Ok result with just the indentation whitespace or err if no valid indentation is seen
 */
Result!TokenStream getIndentation()(auto ref const (TokenStream) stream, ulong position) @trusted @nogc nothrow {
    TokenStream output;

    for (ulong i = position; i >= 0; i--) {
        const Token tok = stream[i];
        if (!isWhitespace(tok))
            break;

        if (tok.type == Tok.CharReturn || tok.type == Tok.NewLine)
            break;

        output.insertBack(tok);
    }

    if (stream.length() == 0)
        return Result!(TokenStream).fail("No valid indentation");

    return Result!(TokenStream).make(output);
}

/** 
 * Checks if the stream of tokens up to the first line break is nothing but whitespace
 * Params:
 *   stream = Input stream
 * Returns: Position in the stream following the next line break, assuming the line is empty
 */
Result!ulong isEmptyLine()(auto ref const(TokenStream) stream) @trusted @nogc nothrow {
    bool expectNewline = false;
    for (ulong i = 0; i < stream.length(); i++) {
        const Token tok = stream[i];
        
        if (tok.type == Tok.NewLine) {
            return Result!(ulong).make(i+1);
        } else if (tok.type == Tok.CharReturn) {
            expectNewline = true;
        } else {
            expectNewline = false;
            if (tok.type == Tok.Space || tok.type == Tok.Tab)
                continue;
            else
                return Result!(ulong).fail("Line is not empty");
        }
    }

    return Result!(ulong).fail("Line is not empty or no return present");
}

/** 
 * Like `isEmptyLine`, but looks behind for a dead line
 * Params:
 *   stream = Input stream
 * Returns: Position in the stream before the last line break, assuming the line is empty
 */
Result!ulong endsOnNewLine()(auto ref const(TokenStream) stream) @trusted {
    bool expectReturn = false;
    for (ulong i = stream.length()-1; i >= 0; i--) {
        const Token tok = stream[i];

        if (tok.type == Tok.NewLine) {
            expectReturn = true;
        } else if (tok.type == Tok.CharReturn && expectReturn) {
            return Result!(ulong).make(i);
        } else {
            expectReturn = false;
            if (tok.type == Tok.Space || tok.type == Tok.Tab)
                continue;
            else
                return Result!(ulong).fail("Line is not empty");
        }
    }

    return Result!(ulong).fail("Line is not empty or no return present");
}

/** 
 * Remove leading and trailing whitespace from the input stream
 * Params:
 *   stream = Input stream
 * TemplateParams:
 *   onlyExtraLines = Only trim leading and trailing lines
 * Returns: TokenStream
 */
TokenStream trim(bool onlyExtraLines = false)(auto ref const(TokenStream) stream) @trusted {
    if (stream.length == 0) return TokenStream();

    static if (onlyExtraLines) {
        TokenStream output = TokenStream(stream[0..$]);

        ulong lastN = 0;
        bool n = false;
        for (ulong i = 0; i < output.length(); i++) {
            const Token tok = output[i];
            if (isWhitespace(tok)) {
                if (tok.type == Tok.NewLine) {
                    lastN = i;
                    n = true;
                }
            } else {
                if (n) {
                    output = TokenStream(output[lastN+1..$]);
                    break;
                }
            }
        }

        n = false;
        for (ulong i = output.length()-1; i >= 0; i--) {
            const Token tok = output[i];
            if (isWhitespace(tok)) {
                if (tok.type == Tok.NewLine) {
                    lastN = i;
                    n = true;
                }
            } else {
                if (n) {
                    output = TokenStream(output[0..lastN]);
                    break;
                }
            }
        }

        return output;

    } else {
        TokenStream output = TokenStream(stream[0..$]);

        for (ulong i = 0; i < output.length(); i++) {
            const Token tok = output[i];
            if (!isWhitespace(tok)) {
                output = TokenStream(output[i..$]);
                break;
            }
        }

        for (ulong i = output.length()-1; i >= 0; i--) {
            const Token tok = output[i];
            if (!isWhitespace(tok)) {
                output = TokenStream(output[0..i+1]);
                break;
            }
        }

        return output;
    }
}

/** 
 * Removes excess indentation levels, keeping only the minimum required
 * Params:
 *   stream = Input stream
 *   tabWidth = Number of spaces to consider as 1 indentation layer
 * Returns: TokenStream
 */
TokenStream trimIndentation()(auto ref const(TokenStream) stream, ulong tabWidth = 4) @trusted {
    import std.algorithm : min;
    const auto lines = toLines(stream).unwrap();
    ulong leastIndentLevel = ulong.max;
    foreach (ref const(TokenStream) lineStream; lines) {
        ulong levels = 0;
        ulong spaces = 0;
        
        for (ulong i = 0; i < lineStream.length(); i++) {
            const Token tok = lineStream[i];
            if (tok.type != Tok.Space && tok.type != Tok.Tab) {
                leastIndentLevel = min(leastIndentLevel, levels);
                break;

            } else if (tok.type == Tok.Space) {
                if (++spaces == tabWidth) {
                    spaces = 0;
                    levels++;
                }
            } else if (tok.type == Tok.Tab) {
                levels++;
            }
        }
    }

    // Already de-tabbed
    if (leastIndentLevel == 0) return TokenStream(stream[0..$]);

    // Subtract leastIndentLevel worth of tabs from each line and recombine
    TokenStream output;
    ulong lineIdx = 0;
    foreach (ref const(TokenStream) lineStream; lines) {
        lineIdx++;
        ulong levels = 0;
        ulong spaces = 0;

        for (ulong i = 0; i < lineStream.length(); i++) {
            const Token tok = lineStream[i];
            if (tok.type == Tok.Space) {
                if (++spaces == tabWidth) {
                    spaces = 0;
                    levels++;
                }
            } else if (tok.type == Tok.Tab) {
                levels++;
            } else {
                assert(0);
            }

            if (levels == leastIndentLevel) {
                output ~= lineStream[i+1..$];
                if (lineIdx < lines.length) {
                    output ~= Token(Tok.CharReturn, "\r");
                    output ~= Token(Tok.NewLine, "\n");
                }
                break;
            }
        }
    }

    return output;
}

/** 
 * Split the token stream on the given `sep` token - return value omits the sep in the stream
 *
 * This is meant for the specific case of extracting arguments from a token separated list and as such,
 * enforces some rules on the input stream:
 *
 * A separator token may not appear twice in a row.
 * The stream may not end on a separator.
 * The stream may not start on a separator.
 *
 * TemplateParams:
 *   enforceTokenCount = Raise an error if maxTok+1 tokens are found without a separator between them.
 *   maxTok = Max number of tokens that may be seen before a separator is required if enforcing token count.
 *   ignoreWhitespace = Ignore whitespace in the stream.
 *
 * Params:
 *   stream = Stream to split
 *   sep = Token to split on
 * Returns: Split token stream
 */
Result!TokenStream explode(bool enforceTokenCount = true, ulong maxTok = 1, bool ignoreWhitespace = true)
    (auto ref const(TokenStream) stream, Tok sep) @trusted
{
    TokenStream output;
    bool sawSep = false;
    bool first = true;

    static if (enforceTokenCount)
        ulong tokCount = 0;

    for (ulong i = 0; i < stream.length(); i++) {
        const Token tok = stream[i];

        static if (ignoreWhitespace)
            if (isWhitespace(tok)) continue;

        if (tok.type == sep) {
            if (first)
                return Result!(TokenStream).fail(
                    "Token stream may not start on a separator!"
                );

            if (sawSep)
                return Result!(TokenStream).fail(
                    "Duplicate '" ~ tok.value ~ "' seen in stream!"
                );
            
            sawSep = true;
            static if (enforceTokenCount) tokCount = 0;
            continue;
        }

        static if (enforceTokenCount)
            if (tokCount >= maxTok)
                return Result!(TokenStream).fail(
                    "Multiple tokens in stream without separators!"
                );
        
        first = false;
        sawSep = false;
        output ~= tok;
        static if (enforceTokenCount) tokCount++;
    }

    if (sawSep)
        return Result!(TokenStream).fail(
            "Token stream may not end on a separator!"
        );
    
    return Result!(TokenStream).make(output);
}

/** 
 * Convert the input stream into an array of streams, with each stream being 1 line
 *
 * Will split based on both '\n' and '\r\n' line breaks
 * Params:
 *   stream = Stream to split into lines
 * Returns: Array of token streams
 */
Result!(TokenStream[]) toLines()(auto ref const(TokenStream) stream) {
    TokenStream[] output;
    bool sawReturn = false;
    ulong startIndex = 0;
    ulong lineEndIndex = 0;
    
    for (auto i = 0; i < stream.length(); i++) {
        const Token tok = stream[i];
        if (tok.type == Tok.CharReturn) {
            if (!sawReturn)
                sawReturn = true;
            // We COULD see several \r's in a row - just ignore it
            lineEndIndex = i; // Our index will slide along excess \r's, including any extra in the output

        } else if (tok.type == Tok.NewLine) {
            if (!sawReturn) lineEndIndex = i;

            output ~= TokenStream(stream[startIndex..lineEndIndex]);
            startIndex = i+1;
            sawReturn = false;
        } else {
            sawReturn = false;
        }
    }

    output ~= TokenStream(stream[startIndex..$]);

    return Result!(TokenStream[]).make(output);
}

/** 
 * Indent every line in the input stream with the contents of the indentation stream
 *
 * This can also be used to prepend something to every line in the input stream, not just add whitespace
 * Params:
 *   stream = Input stream to apply indentation to
 *   indentation = Indentation stream to copy to the start of every line in `stream`
 * Returns: Indented token stream
 */
Result!TokenStream indent()(auto ref const(TokenStream) stream, auto ref const(TokenStream) indentation) {
    auto lines = toLines(stream);
    if (!lines.isOk()) return Result!(TokenStream).failFrom(lines);

    auto lineArray = lines.unwrap();
    TokenStream output;
    ulong lineIdx = 0;
    foreach (ref TokenStream lineStream; lineArray) {
        lineIdx++;
        output ~= indentation[0..$];
        output ~= lineStream[0..$];

        if (lineIdx < lineArray.length) {
            output ~= Token(Tok.CharReturn, "\r");
            output ~= Token(Tok.NewLine, "\n");
        }
    }

    return Result!(TokenStream).make(output);
}