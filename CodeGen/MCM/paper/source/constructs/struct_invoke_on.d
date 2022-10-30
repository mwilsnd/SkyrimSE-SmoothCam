module constructs.struct_invoke_on;
import constructs.iconstruct;

import constructs.const_struct;

import keywords;
import tokenizer : Token, TokenStream;
import tokenizer.tokens;
import tokenizer.utils;
import lex_machine;

import std.string;
import std.container.array;

private enum MacroName = "StructInvokeOn";

/** 
 * #StructInvokeOn(macroName, [implA, implB, ...])
 *
 * Becomes:
 *  implA->!macroName
 *  implB->!macroName
 *  ...
 */
final class StructInvokeOn : IConstruct {
    public:
        @disable this(ref return scope StructInvokeOn rhs);
        
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
                string invocation;
                TokenStream generated;
            }
            enum State {
                ExpectHash,
                ExpectMacroName,
                ExpectOpenParen,
                ExpectInvocation,
                ExpectComma,
                ExpectOpenBrace,
                ExpectCloseParen,
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
                        state.gotoState(State.ExpectOpenParen);
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

            state.onState(State.ExpectOpenParen, Tok.OpenParen,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.gotoState(State.ExpectInvocation);
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectInvocation, Tok.OtherValue,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.mem.invocation = tok.value;
                    state.gotoState(State.ExpectComma);
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectComma, Tok.Comma,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.gotoState(State.ExpectOpenBrace);
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectOpenBrace, Tok.OpenBrace,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    auto subStream = TokenStream(stream[position+1..$]);
                    auto impls = getBalancedContent(subStream, Tok.OpenBrace, Tok.CloseBrace);
                    if (!impls.isOk())
                        return Result!(Res).failFrom(impls);

                    auto ws = getIndentation(stream, state.mem.startIndex-1);

                    auto implList = impls.unwrap();
                    auto generated = createInvocations(state.mem.invocation, implList, ws);
                    if (!generated.isOk())
                        return Result!(Res).failFrom(generated);

                    generated.unwrapEmplace(state.mem.generated);
                    state.gotoState(State.ExpectCloseParen);
                    state.jump(implList.length() + 2);
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectCloseParen, Tok.CloseParen,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    auto pre = stream[0..state.mem.startIndex];
                    auto post = stream[position+1..$];
                    // Slice off any trailing empty line left over
                    auto postE = isEmptyLine(TokenStream(post));
                    if (postE.isOk())
                        post = post[postE.unwrap()..$];
                    
                    TokenStream mutated;
                    mutated ~= pre;
                    mutated ~= state.mem.generated;
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
                state.gotoState(State.ExpectHash);
            }

            return Result!(bool).make(true);
        }

    private:
        ConstStructParser parser = null;

        /** 
         * Generate a list of macro invocations from the input stream
         * Params:
         *   name = Name of the macro to invoke
         *   stream = Comma separated list of impls to invoke name on
         *   indent = getIndentation() result for correct line indent matching
         * Returns: Output stream of macro invocations
         */
        Result!TokenStream createInvocations(ref const(string) name, ref const(TokenStream) stream,
            ref Result!TokenStream indent)
        {
            TokenStream output;

            bool firstLine = true;
            bool expectName = true;
            bool expectComma = false;
            for (ulong i = 0; i < stream.length(); i++) {
                const Token tok = stream[i];
                if (isWhitespace(tok)) continue;

                if (expectName) {
                    if (tok.type != Tok.OtherValue)
                        return Result!(TokenStream).fail("Unexpected token '" ~ tok.value ~ "', wanted a name!");
                    
                    if (!firstLine && indent.isOk())
                        output ~= indent.unwrap()[0..$];

                    output ~= tok;
                    output ~= Token(Tok.Arrow, "->");
                    output ~= Token(Tok.Exclamation, "!");
                    output ~= Token(Tok.OtherValue, name);
                    output ~= Token(Tok.CharReturn, "\r");
                    output ~= Token(Tok.NewLine, "\n");
                    expectName = false;
                    expectComma = true;
                    firstLine = false;

                } else if (expectComma) {
                    if (tok.type != Tok.Comma)
                        return Result!(TokenStream).fail("Unexpected token '" ~ tok.value ~ "', wanted ','!");
                    expectComma = false;
                    expectName = true;
                }
            }

            return Result!(TokenStream).make(output);
        }
}