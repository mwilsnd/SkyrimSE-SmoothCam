module constructs.struct_alias;

import constructs.iconstruct;

import keywords;
import tokenizer : Token, TokenStream;
import tokenizer.tokens;
import tokenizer.utils;
import tokenizer;
import lex_machine;
import constructs.const_struct;

import std.string;
import std.container.array;

private enum MacroName = "alias";

/**
    #alias MyNewType = SomeOtherConstexprStructType
*/
final class StructAlias : IConstruct {
    public:
        @disable this(ref return scope StructAlias rhs);
        
        /// ctor
        this() {}

        void setConstStructTool(ref ConstStructParser parser) {
            this.parser = parser;
        }

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
                Token aliasName;
            }
            enum State {
                ExpectHash,
                ExpectMacroName,
                ExpectAliasName,
                ExpectAssignment,
                ExpectStructType,
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
                        state.gotoState(State.ExpectAliasName);
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

            state.onState(State.ExpectAliasName, Tok.OtherValue,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                   state.mem.aliasName = tok;
                   state.gotoState(State.ExpectAssignment);
                   return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectAssignment, Tok.Assignment,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                   state.gotoState(State.ExpectStructType);
                   return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectStructType, Tok.OtherValue,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    auto ty = parser.getDeclType(tok.value);
                    if (ty is null)
                        return Result!(Res).fail("Undefined type given to alias. '" ~ tok.value ~ "'");

                    const auto res = parser.createStructAlias(ty, state.mem.aliasName.value);
                    if (!res.isOk()) return Result!(Res).failFrom(res);

                    auto pre = TokenStream(stream[0..state.mem.startIndex]);
                    auto post = TokenStream(stream[position+1..$]);

                    stream = pre ~ post;

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

    private:
        ConstStructParser parser;
}