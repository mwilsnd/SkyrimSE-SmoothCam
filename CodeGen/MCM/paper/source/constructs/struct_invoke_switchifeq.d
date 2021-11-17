module constructs.struct_invoke_switchifeq;
import constructs.iconstruct;

import constructs.const_struct;

import keywords;
import tokenizer : Token, TokenStream;
import tokenizer.tokens;
import tokenizer.utils;
import lex_machine;

import std.string;
import std.container.array;

private enum MacroName = "StructInvokeSwitchIfEquals";
private {
    struct SwitchBranch {
        /// Impls inside this branch
        Array!ConstStructImpl impls;

        /** 
         * Generate the inner if case branches
         * Params:
         *   lhs = Left hand side of the if case
         *   rhs = Right hand side of the if case (must be a member variable of each struct impl)
         *   macroName = Name of the macro to invoke
         * Returns: Token stream containing complete if branch block
         */
        Result!TokenStream generate()(auto ref const(string) lhs, auto ref const(string) rhs,
            auto ref const(string) macroName)
        {
            if (impls.length() == 0)
                return Result!(TokenStream).make(TokenStream());
            
            TokenStream stream;
            bool first = true;
            for (ulong i = 0; i < impls.length(); i++) {
                const ConstStructImpl impl = impls[i];

                const MemberVar* var = rhs in impl.memberVars;
                if (var is null)
                    return Result!(TokenStream).fail("Undefined variable '" ~ rhs ~ "'!");
                
                // if/elseIf
                if (first) {
                    stream ~= Token(Tok.Tab, "\t");
                    stream ~= Token(Tok.kIf, "if");
                    first = false;
                } else {
                    stream ~= Token(Tok.CharReturn, "\r");
                    stream ~= Token(Tok.NewLine, "\n");
                    stream ~= Token(Tok.Tab, "\t");
                    stream ~= Token(Tok.kElseIf, "elseIf");
                }

                // (lhs == resolve(rhs))
                // >
                stream ~= Token(Tok.Space, " ");
                stream ~= Token(Tok.OpenParen, "(");
                stream ~= Token(Tok.OtherValue, lhs);
                stream ~= Token(Tok.Space, " ");
                stream ~= Token(Tok.Equals, "==");
                stream ~= Token(Tok.Space, " ");

                auto toks = impl.memberToActual(rhs, true);
                if (!toks.isOk()) return Result!(TokenStream).failFrom(toks);
                stream ~= toks.unwrap();

                stream ~= Token(Tok.CloseParen, ")");
                stream ~= Token(Tok.CharReturn, "\r");
                stream ~= Token(Tok.NewLine, "\n");
                stream ~= Token(Tok.Tab, "\t");
                stream ~= Token(Tok.Tab, "\t");

                // \tinvoke->!macroName
                stream ~= Token(Tok.OtherValue, impl.implName);
                stream ~= Token(Tok.Arrow, "->");
                stream ~= Token(Tok.Exclamation, "!");
                stream ~= Token(Tok.OtherValue, macroName);

                // return
                stream ~= Token(Tok.CharReturn, "\r");
                stream ~= Token(Tok.NewLine, "\n");
                stream ~= Token(Tok.Tab, "\t");
                stream ~= Token(Tok.Tab, "\t");
                stream ~= Token(Tok.kReturn, "return");
            }

            stream ~= Token(Tok.CharReturn, "\r");
            stream ~= Token(Tok.NewLine, "\n");
            stream ~= Token(Tok.Tab, "\t");
            stream ~= Token(Tok.kEndIf, "endIf");
            stream ~= Token(Tok.CharReturn, "\r");
            stream ~= Token(Tok.NewLine, "\n");
            return Result!(TokenStream).make(stream);
        }
    }

    struct Switch {
        /// Member var name being switched on, must not be `real`, value must be known at 'compile' time
        string memberVarBucket;
        /// Set by the type of the first struct member seen, all types that follow must match
        Tok resolvedType = Tok.tNone;
        /// Bucket map, grouped by value literal of memberVarBucket
        SwitchBranch[string] branches;

        /** 
         * Register a new impl with the switch thing
         * Params:
         *   impl = Struct impl to add
         */
        Result!bool put(ConstStructImpl* impl) {
            import std.conv : to;

            const(MemberVar)* var = memberVarBucket in impl.memberVars;
            if (var is null)
                return Result!(bool).fail("Member var '" ~ memberVarBucket ~ "' not found on struct '" ~
                    impl.implName ~ "'!"
                );
            
            if (var.isReal)
                return Result!(bool).fail("Cannot #StructInvokeSwitchIfEquals on a member tagged 'real' (struct '" ~
                    impl.implName ~ "')!"
                );
            
            string key = var.valueToString();

            // First var, resolve type and insert branch
            if (resolvedType == Tok.tNone) {
                resolvedType = var.type;

                SwitchBranch branch;
                branch.impls ~= *impl;
                branches[key] = branch;
                return Result!(bool).make(true);
            }

            // Assert type match
            if (resolvedType != var.type)
                return Result!(bool).fail("Type mismatch in #StructInvokeSwitchIfEquals - expected type '" ~
                    to!string(resolvedType) ~ "', not '" ~ to!string(var.type) ~ "'!"
                );
            
            // Check if we already have a branch for this value
            SwitchBranch* branch = key in branches;
            if (branch is null) {
                SwitchBranch newBranch;
                newBranch.impls ~= *impl;
                branches[key] = newBranch;
                return Result!(bool).make(true);
            }

            // Already have a branch, add to it
            branch.impls ~= *impl;
            return Result!(bool).make(true);
        }
    }
}

/** 
 * Invoke a macro among a list of struct impls if 2 nested if conditions are true
 * Outer condition is generated from a unique list of inline values from all input structs
 * (Meaning the outer switch can only be performed on values which are not 'real', must be known at 'compile' time)
 *  ```
 *  #StructInvokeSwitchIfEquals(
 *      a_option, ref,              ; Inner condition, ref can be 'real'
 *      activePage, page,           ; Outer 'switch' condition, page cannot 'real'
 *      implSelectHandler,          ; Macro to invoke
 *      [KeyBindSetting # ImplsOf]  ; List of structs
 *  )
 *  ```
 *  Assuming unique `page` members on all structs condenses to [page1, page2],
 *  Becomes:
 *  ```
 *  if (activePage == "page1")
 *      structs with page == "page1"
 *          if (inImpl1.ref == a_option)
 *              imImpl1->!implSelectHandler
 *          elseIf (inImpl2.ref == a_option)
 *              inImpl2->!implSelectHandler
 *          endIf
 *  elseIf (activePage == "page2")
 *      structs with page == "page2"
 *          if (inImpl3.ref == a_option)
 *              inImpl3->!implSelectHandler
 *          elseIf
 *              ...
 *  endIf
 *  ```
 *  Effectively, this is a switch on the outer condition, then if case on the inner condition
 */
final class StructInvokeSwitchIfEq : IConstruct {
    public:
        @disable this(ref return scope StructInvokeSwitchIfEq rhs);
        
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
            enum State {
                ExpectHash,
                ExpectMacro,
                ExpectOpenParen,
                ExpectIfLHS,
                ExpectComma,
                ExpectIfRHS,
                ExpectSwitchCond,
                ExpectSwitchValue,
                ExpectInvocation,
                ExpectOpenBrace,
                ExpectCloseParen,
            }
            struct Memory {
                ulong startIndex = 0;
                Token ifLhs;
                Token ifRhs;
                Token switchCond;
                Token switchValue;
                Token invocation;
                State jmp;
                Switch branchGenerator;
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
                    state.gotoState(State.ExpectIfLHS);
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectIfLHS, Tok.OtherValue,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.gotoState(State.ExpectComma);
                    state.mem.jmp = State.ExpectIfRHS;
                    state.mem.ifLhs = tok;
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectComma, Tok.Comma,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.gotoState(state.mem.jmp);
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectIfRHS, Tok.OtherValue,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.gotoState(State.ExpectComma);
                    state.mem.jmp = State.ExpectSwitchCond;
                    state.mem.ifRhs = tok;
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectSwitchCond, Tok.OtherValue,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.gotoState(State.ExpectComma);
                    state.mem.jmp = State.ExpectSwitchValue;
                    state.mem.switchCond = tok;
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectSwitchValue, Tok.OtherValue,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.gotoState(State.ExpectComma);
                    state.mem.jmp = State.ExpectInvocation;
                    state.mem.switchValue = tok;
                    state.mem.branchGenerator.memberVarBucket = tok.value;
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectInvocation, Tok.OtherValue,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    state.gotoState(State.ExpectComma);
                    state.mem.jmp = State.ExpectOpenBrace;
                    state.mem.invocation = tok;
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectOpenBrace, Tok.OpenBrace,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    // Now we can read the list of impls
                    auto subStream = TokenStream(stream[position+1..$]);
                    auto values = getBalancedContent(subStream, Tok.OpenBrace, Tok.CloseBrace);
                    if (!values.isOk())
                        return Result!(Res).failFrom(values);

                    // We should have a comma separated list of value literals
                    auto list = explode(values.unwrap(), Tok.Comma);
                    if (!list.isOk())
                        return Result!(Res).failFrom(list);
                    
                    // Build a unique list of values for each, sort into buckets
                    auto impls = list.unwrap();
                    foreach (ref Token t; impls) {
                        if (t.type != Tok.OtherValue)
                            return Result!(Res).fail("Unexpected token '" ~
                                t.value ~ "' when parsing StructInvokeSwitchIfEquals!"
                            );

                        ConstStructImpl* impl = parser.getImplType(t.value);
                        if (impl is null)
                            return Result!(Res).fail("Unknown variable '" ~
                                t.value ~ "' when parsing StructInvokeSwitchIfEquals!"
                            );
                        
                        auto err = state.mem.branchGenerator.put(impl);
                        if (!err.isOk())
                            return Result!(Res).failFrom(err);
                    }

                    state.gotoState(State.ExpectCloseParen);
                    state.jump(values.unwrap().length()+2);
                    return Result!(Res).make(Res.Continue);
                }
            );

            state.onState(State.ExpectCloseParen, Tok.CloseParen,
                (ref const(Token) tok, ref TokenStream stream, ulong position) {
                    auto pre = stream[0..state.mem.startIndex];
                    auto post = stream[position+1..$];

                    TokenStream mutated;
                    mutated ~= pre;

                    auto level = getIndentation(
                        TokenStream(pre),
                        pre.length > 0 ? cast(long)pre.length -1 : cast(long)pre.length
                    );

                    // Generate branches, slice in stream
                    bool first = true;
                    foreach (ref const(string) key, ref SwitchBranch branch; state.mem.branchGenerator.branches) {
                        auto branchStream = branch.generate(
                            state.mem.ifLhs.value, state.mem.ifRhs.value, state.mem.invocation.value
                        );

                        if (!branchStream.isOk())
                            return Result!(Res).failFrom(branchStream);

                        if (first)
                            mutated ~= Token(Tok.kIf, "if");
                        else
                            mutated ~= Token(Tok.kElseIf, "elseIf");

                        first = false;
                        mutated ~= Token(Tok.Space, " ");
                        mutated ~= Token(Tok.OpenParen, "(");
                        mutated ~= state.mem.switchCond;
                        mutated ~= Token(Tok.Space, " ");
                        mutated ~= Token(Tok.Equals, "==");
                        mutated ~= Token(Tok.Space, " ");
                        
                        switch (state.mem.branchGenerator.resolvedType) {
                            case Tok.tBool:
                                mutated ~= Token(Tok.BoolValue, key);
                                break;

                            case Tok.tInt: goto case;
                            case Tok.tFloat:
                                mutated ~= Token(Tok.NumericValue, key);
                                break;

                            case Tok.tLiteral:
                                mutated ~= Token(Tok.OtherValue, key);
                                break;

                            case Tok.tString:
                                mutated ~= Token(Tok.StringValue, key);
                                break;

                            default: assert(0);
                        }

                        mutated ~= Token(Tok.CloseParen, ")");
                        mutated ~= Token(Tok.CharReturn, "\r");
                        mutated ~= Token(Tok.NewLine, "\n");

                        if (level.isOk())
                            mutated ~= indent(branchStream.unwrap(), level.unwrap()).unwrap()[0..$];
                        else
                            mutated ~= branchStream.unwrap()[0..$];
                    }

                    mutated ~= Token(Tok.kEndIf, "endIf");
                    mutated ~= Token(Tok.CharReturn, "\r");
                    mutated ~= Token(Tok.NewLine, "\n");
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
}