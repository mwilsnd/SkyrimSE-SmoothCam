module constructs.all_of_struct;
import constructs.iconstruct;

import constructs.const_struct;

import keywords;
import tokenizer : Token, TokenStream;
import tokenizer.tokens;
import tokenizer.utils;
import lex_machine;

import std.string;
import std.container.array;

private enum MacroName = "ImplsOf";

/** 
 * DeclType # ImplsOf
 *
 * Becomes: implNameA, implNameB, implNameC, ...
 */
final class AllOfStruct : IConstruct {
    public:
        @disable this(ref return scope AllOfStruct rhs);
        
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
                string declType;
                ulong startIndex = 0;
            }
            enum State {
                ExpectDeclType,
                ExpectHash,
                ExpectMacro,
            }
            enum Res {
                Continue,
                BreakInner,
                BreakOuter,
            }

            LexMachine!(
                State, State.ExpectDeclType,
                Res, Res.Continue,
                Memory
            ) state;

            state.onState(State.ExpectDeclType, Tok.OtherValue,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.mem.declType = tok.value;
                    state.mem.startIndex = position;
                    state.gotoState(State.ExpectHash);
                    return Result!(Res).make(Res.Continue);
                },
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectHash, Tok.Hash,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.gotoState(State.ExpectMacro);
                    return Result!(Res).make(Res.Continue);
                },
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.resetMemory();
                    state.gotoState(State.ExpectDeclType);
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectMacro, Tok.OtherValue,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    if (tok.value != MacroName) {
                        state.resetMemory();
                        state.gotoState(State.ExpectDeclType);
                        return Result!(Res).make(Res.Continue);
                    }
                    
                    // Done, now look for the decl type
                    const auto ty = parser.getDeclType(state.mem.declType);
                    if (ty is null)
                        return Result!(Res).fail("Unknown struct '" ~
                            state.mem.declType ~ "'!"
                        );
                    
                    // Now get all impls of that type
                    const auto list = parser.getImplsOfDeclType(ty);
                    if (list.length() == 0)
                        return Result!(Res).fail("No implementations of struct type '" ~
                            state.mem.declType ~ "' were found!"
                        );
                    
                    // And slice in our comma separated list of impl names
                    auto pre = stream[0..state.mem.startIndex];
                    auto post = stream[position+1..$];

                    TokenStream mutated;
                    mutated ~= pre;

                    for (ulong j = 0; j < list.length(); j++) {
                        const ConstStructImpl impl = list[j];
                        mutated ~= Token(Tok.OtherValue, impl.implName);

                        if (j != list.length()-1)
                            mutated ~= Token(Tok.Comma, ",");
                    }
                    
                    mutated ~= post;
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
                state.gotoState(State.ExpectDeclType);
            }

            return Result!(bool).make(true);
        }

    private:
        ConstStructParser parser = null;
}