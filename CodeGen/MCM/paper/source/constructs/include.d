module constructs.include;

import constructs.iconstruct;

import keywords;
import tokenizer : Token, TokenStream;
import tokenizer.tokens;
import tokenizer.utils;
import tokenizer;
import lex_machine;

import std.string;
import std.container.array;
import fs = std.file;

private enum MacroName = "include";

/**
    #include<path_relative_from_working_dir>
*/
final class Include : IConstruct {
    public:
        @disable this(ref return scope Include rhs);
        
        /// ctor
        this() {}

        /**
         * Process the token stream and build constructs
         * Params:
         *  stream: Input tokens to parse
         * Returns: Parse result
         */
        Result!bool parse(ref const(TokenStream) stream) @trusted {
            // do nothing
            return Result!(bool).make(true);
        }

        /**
         * Apply generated constructs to the token stream
         * Params:
         *  stream: Mutable token stream to apply to
         * Returns: Apply result
         */
        Result!bool apply(ref TokenStream stream) @trusted {
            struct Memory {
                ulong startIndex = 0;
            }
            enum State {
                ExpectHash,
                ExpectMacroName,
                ExpectPath
            }
            enum Res {
                Continue,
                BreakInner,
                BreakOuter,
            }

            LexMachine!(
                State, State.ExpectHash,
                Res, Res.Continue,
                Memory
            ) state;

            state.onState(State.ExpectHash, Tok.Hash,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.mem.startIndex = position;
                    state.gotoState(State.ExpectMacroName);
                    return Result!(Res).make(Res.Continue);
                },
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectMacroName, Tok.OtherValue,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    if (tok.value == MacroName)
                        state.gotoState(State.ExpectPath);
                    else {
                        state.resetMemory();
                        state.gotoState(State.ExpectHash);
                    }

                    return Result!(Res).make(Res.Continue);
                },
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.resetMemory();
                    state.gotoState(State.ExpectHash);
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectPath, Tok.StringValue,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    string fileContents;
                    try {
                        fileContents = cast(string)fs.read("./" ~ tok.value);
                    } catch(fs.FileException e) {
                        return Result!(Res).fail(e.toString());
                    }

                    TokenStream includeStream;
                    auto err = tokenize(fileContents, includeStream);
                    if (!err.isOk())
                        return Result!(Res).failFrom(err);

                    TokenStream mutated;
                    mutated ~= TokenStream(stream[0..state.mem.startIndex]);
                    mutated ~= includeStream;
                    mutated ~= TokenStream(stream[position+1..$]);
                    stream = mutated;
                    return Result!(Res).make(Res.BreakInner);
                }
            );

            state.onEOS((ref TokenStream stream) => Result!(Res).make(Res.BreakOuter));

            while (true) {
                const auto res = state.exec(stream);
                if (!res.isOk()) return Result!(bool).failFrom(res);
                if (res.unwrap() == Res.BreakOuter) break;
                state.resetMemory();
                state.gotoState(State.ExpectHash);
            }

            return Result!(bool).make(true);
        }
}