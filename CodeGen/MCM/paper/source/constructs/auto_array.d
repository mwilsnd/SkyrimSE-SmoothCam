module constructs.auto_array;
import constructs.iconstruct;

import keywords;
import tokenizer : Token, TokenStream;
import tokenizer.tokens;
import tokenizer.utils;
import lex_machine;

import std.string;
import std.container.array;

/** 
 * var = new type[] -> [a, b, c, ...]
 *
 * Becomes:
 * var = new type[3]
 * var[0] = a
 * var[1] = b
 * var[2] = c
 */
final class AutoArray : IConstruct {
    public:
        @disable this(ref return scope AutoArray rhs);
        
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
            import std.conv : to;

            struct Memory {
                Tok type = Tok.tNone;
                string varName;
                ulong startIndex = 0;
            }
            enum State {
                ExpectVarName,
                ExpectAssignment,
                ExpectNew,
                ExpectType,
                ExpectOpenBrace,
                ExpectCloseBrace,
                ExpectArrow,
                ExpectInitList,
            }
            enum Res {
                Continue,
                BreakInner,
                BreakOuter,
            }

            LexMachine!(
                State, State.ExpectVarName,
                Res, Res.Continue,
                Memory
            ) state;

            state.onState(State.ExpectVarName, Tok.OtherValue,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.mem.varName = tok.value;
                    state.mem.startIndex = position;
                    state.gotoState(State.ExpectAssignment);
                    return Result!(Res).make(Res.Continue);
                },
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectAssignment, Tok.Assignment,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.gotoState(State.ExpectNew);
                    return Result!(Res).make(Res.Continue);
                },
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.resetMemory();
                    state.gotoState(State.ExpectVarName);
                    return Result!(Res).make(Res.Continue);
                }
            );
            
            state.onState(State.ExpectNew, Tok.kNew,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.gotoState(State.ExpectType);
                    return Result!(Res).make(Res.Continue);
                },
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.resetMemory();
                    state.gotoState(State.ExpectVarName);
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectType, [Tok.tBool, Tok.tFloat, Tok.tInt, Tok.tString],
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.mem.type = tok.type;
                    state.gotoState(State.ExpectOpenBrace);
                    return Result!(Res).make(Res.Continue);
                },
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.resetMemory();
                    state.gotoState(State.ExpectVarName);
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectOpenBrace, Tok.OpenBrace,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.gotoState(State.ExpectCloseBrace);
                    return Result!(Res).make(Res.Continue);
                },
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.resetMemory();
                    state.gotoState(State.ExpectVarName);
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectCloseBrace, Tok.CloseBrace,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.gotoState(State.ExpectArrow);
                    return Result!(Res).make(Res.Continue);
                },
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.resetMemory();
                    state.gotoState(State.ExpectVarName);
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectArrow, Tok.Arrow,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.gotoState(State.ExpectInitList);
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectInitList, Tok.OpenBrace,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    // Now, read the contents of the list [...]
                    auto subStream = TokenStream(stream[position+1..$]);
                    auto values = getBalancedContent(subStream, Tok.OpenBrace, Tok.CloseBrace);
                    if (!values.isOk())
                        return Result!(Res).failFrom(values);

                    // We should have a comma separated list of value literals
                    auto list = explode(values.unwrap(), Tok.Comma);
                    if (!list.isOk())
                        return Result!(Res).failFrom(list);

                    const auto entries = list.unwrap();
                    const auto num = entries.length();

                    // Now emit the array
                    TokenStream output;
                    auto pre = stream[0..state.mem.startIndex];
                    output ~= pre;

                    auto ws = getIndentation(stream, state.mem.startIndex-1);

                    output ~= Token(Tok.OtherValue, state.mem.varName);
                    output ~= Token(Tok.Space, " ");
                    output ~= Token(Tok.Assignment, "=");
                    output ~= Token(Tok.Space, " ");
                    output ~= Token(Tok.kNew, "new");
                    output ~= Token(Tok.Space, " ");
                    output ~= Token(Tok.OtherValue, typeToString(state.mem.type));
                    output ~= Token(Tok.OpenBrace, "[");
                    output ~= Token(Tok.NumericValue, to!string(num));
                    output ~= Token(Tok.CloseBrace, "]");
                    output ~= Token(Tok.CharReturn, "\r");
                    output ~= Token(Tok.NewLine, "\n");

                    ulong idx = 0;
                    foreach (ref Token e; entries) {
                        // Add indentation to make it look a bit nicer
                        if (ws.isOk())
                            output ~= ws.unwrap()[0..$];

                        output ~= Token(Tok.OtherValue, state.mem.varName);
                        output ~= Token(Tok.OpenBrace, "[");
                        output ~= Token(Tok.NumericValue, to!string(idx));
                        output ~= Token(Tok.CloseBrace, "]");
                        output ~= Token(Tok.Space, " ");
                        output ~= Token(Tok.Assignment, "=");
                        output ~= Token(Tok.Space, " ");
                        output ~= e;
                        output ~= Token(Tok.CharReturn, "\r");
                        output ~= Token(Tok.NewLine, "\n");
                        idx++;
                    }

                    auto post = stream[position+values.unwrap().length()+2..$];
                    // Slice off any trailing empty line left over
                    auto postE = isEmptyLine(TokenStream(post));
                    if (postE.isOk())
                        post = post[postE.unwrap()..$];

                    output ~= post;
                    stream = output;
                    return Result!(Res).make(Res.BreakInner);
                }
            );

            state.onEOS((ref TokenStream stream) => Result!(Res).make(Res.BreakOuter));

            while (true) {
                const auto res = state.exec(stream);
                if (!res.isOk()) Result!(bool).failFrom(res);
                if (res.unwrap() == Res.BreakOuter) break;
                state.resetMemory();
                state.gotoState(State.ExpectVarName);
            }

            return Result!(bool).make(true);
        }
}