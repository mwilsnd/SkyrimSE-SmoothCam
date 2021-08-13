module constructs.impl_offset_group;
import constructs.iconstruct;
import constructs.decl_offset_group;

import keywords;
import tokenizer : Token, TokenStream;
import tokenizer.tokens;
import tokenizer.utils;
import lex_machine;

import std.container.array;

private enum MacroName = "ImplOffsetGroupPage";

/** 
 * #ImplOffsetGroupPage(GroupName)
 * Unrolls to ->!implControl on each control generated from #CreateOffsetGroup
 */
final class ImplOffsetGroup : IConstruct {
    public:
        @disable this(ref return scope DeclareOffsetGroup rhs);
        
        /// ctor
        this() {}

        /** 
         * Sets the generator used to create offset groups
         * Params:
         *   mgr = DeclareOffsetGroup
         */
        void setOFSMgr(ref DeclareOffsetGroup mgr) {
            ofsGroups = mgr;
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
                Token value;
                bool noSliderHeader = false;
                bool noInterpToggles = false;
            }
            enum State {
                ExpectHash,
                ExpectMacro,
                ExpectOpenParen,
                ExpectName,
                ExpectCommaOrCloseParen,
                ExpectOptFlag,
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
                    state.gotoState(State.ExpectMacro);
                    state.mem.startIndex = position;
                    return Result!(Res).make(Res.Continue);
                },
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectMacro, Tok.OtherValue,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    if (tok.value == MacroName)
                        state.gotoState(State.ExpectOpenParen);
                    else
                        state.gotoState(State.ExpectHash);
                    return Result!(Res).make(Res.Continue);
                },
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.gotoState(State.ExpectHash);
                    state.resetMemory();
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectOpenParen, Tok.OpenParen,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.gotoState(State.ExpectName);
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectName, Tok.OtherValue,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    auto vars = tok.value in ofsGroups.getVarIDs();
                    if (vars is null)
                        Result!(Res).fail("Unknown offset group '" ~ tok.value ~ "'!");

                    state.mem.value = tok;
                    state.gotoState(State.ExpectCommaOrCloseParen);
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectCommaOrCloseParen, [Tok.Comma, Tok.CloseParen],
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    if (tok.type == Tok.Comma)
                        state.gotoState(State.ExpectOptFlag);
                    else {
                        auto pre = stream[0..state.mem.startIndex];
                        auto post = stream[position+1..$];
                        auto vars = state.mem.value.value in ofsGroups.getVarIDs();

                        TokenStream generated;
                        foreach (ref const(string) varName; (*vars).left) {
                            generated ~= Token(Tok.OtherValue, varName);
                            generated ~= Token(Tok.Arrow, "->");
                            generated ~= Token(Tok.Exclamation, "!");
                            generated ~= Token(Tok.OtherValue, "implControl");
                            generated ~= Token(Tok.CharReturn, "\r");
                            generated ~= Token(Tok.NewLine, "\n");
                        }

                        if (!state.mem.noInterpToggles) {
                            generated ~= Token(Tok.OtherValue, "SetCursorPosition");
                            generated ~= Token(Tok.OpenParen, "(");
                            generated ~= Token(Tok.NumericValue, "1");
                            generated ~= Token(Tok.CloseParen, ")");
                            generated ~= Token(Tok.CharReturn, "\r");
                            generated ~= Token(Tok.NewLine, "\n");

                            foreach (ref const(string) varName; (*vars).right) {
                                generated ~= Token(Tok.OtherValue, varName);
                                generated ~= Token(Tok.Arrow, "->");
                                generated ~= Token(Tok.Exclamation, "!");
                                generated ~= Token(Tok.OtherValue, "implControl");
                                generated ~= Token(Tok.CharReturn, "\r");
                                generated ~= Token(Tok.NewLine, "\n");
                            }
                        }

                        TokenStream mutated;
                        
                        auto ind = getIndentation(stream, state.mem.startIndex-1);
                        if (ind.isOk()) {
                            mutated ~= pre[0..state.mem.startIndex-ind.unwrap().length()];
                            mutated ~= indent(generated, ind.unwrap()).unwrap();
                        } else {
                            mutated ~= pre;
                        }

                        mutated ~= post;
                        stream = mutated;

                        return Result!(Res).make(Res.BreakInner);
                    }

                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectOptFlag, Tok.OtherValue,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    if (tok.value == "NoSliderHeader")
                        state.mem.noSliderHeader = true;
                    else if (tok.value == "NoInterpToggles")
                        state.mem.noInterpToggles = true;
                    else
                        return Result!(Res).fail("Unknown flag '" ~ tok.value ~ "'!");

                    state.gotoState(State.ExpectCommaOrCloseParen);
                    return Result!(Res).make(Res.Continue);
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
        DeclareOffsetGroup ofsGroups = null;
}